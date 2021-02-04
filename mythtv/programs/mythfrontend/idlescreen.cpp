
#include "idlescreen.h"

#include <chrono>

#include <QTimer>

#include <mythcontext.h>
#include <mythsystemlegacy.h>

#include <mythuibuttonlist.h>
#include <mythuistatetype.h>
#include <mythuitext.h>
#include <mythmainwindow.h>

#include <programinfo.h>

#include <tvremoteutil.h>

static constexpr std::chrono::milliseconds UPDATE_INTERVAL { 15s };

IdleScreen::IdleScreen(MythScreenStack *parent)
              :MythScreenType(parent, "standbymode"),
              m_updateScreenTimer(new QTimer(this))
{
    gCoreContext->addListener(this);
    GetMythMainWindow()->EnterStandby();

    connect(m_updateScreenTimer, &QTimer::timeout, this, &IdleScreen::UpdateScreen);
    m_updateScreenTimer->start(1000);

    // if this is a frontend only machine and the user has set a time to wait before shutting down
    // and a halt command has been set then start the shutdown count down
    if (gCoreContext->IsFrontendOnly() && gCoreContext->GetNumSetting("FrontendShutdownTimeout", 0) > 0
        && !gCoreContext->GetSetting("HaltCommand", "").isEmpty())
    {
        m_FEShutdownEnabled = true;
        m_secondsToShutdown = std::chrono::seconds(gCoreContext->GetNumSetting("FrontendShutdownTimeout", 0));
    }
}

IdleScreen::~IdleScreen()
{
    GetMythMainWindow()->ExitStandby();
    gCoreContext->removeListener(this);

    if (m_updateScreenTimer)
        m_updateScreenTimer->disconnect();
}

bool IdleScreen::Create(void)
{
    // Load the theme for this screen
    bool foundtheme = LoadWindowFromXML("status-ui.xml", "standbymode", this);
    if (!foundtheme)
        return false;

    bool err = false;
    UIUtilE::Assign(this, m_statusState, "backendstatus", &err);

    /* currentrecording, nextrecording, conflicts and conflictwarning are optional */
    UIUtilW::Assign(this, m_currentRecordings, "currentrecording");
    UIUtilW::Assign(this, m_nextRecordings, "nextrecording");
    UIUtilW::Assign(this, m_conflictingRecordings, "conflicts");
    UIUtilW::Assign(this, m_conflictWarning, "conflictwarning");

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'standbymode'");
        return false;
    }

    UpdateScheduledList();

    return true;
}

void IdleScreen::Load(void)
{
    MythScreenType::Load();
}

void IdleScreen::Init(void)
{
    UpdateScreen();
}

bool IdleScreen::CheckConnectionToServer(void)
{
    m_updateScreenTimer->stop();

    bool bRes = false;

    if (gCoreContext->IsConnectedToMaster())
        bRes = true;
    else
    {
        if (gCoreContext->SafeConnectToMasterServer(false))
            bRes = true;
    }

    if (bRes)
        m_updateScreenTimer->start(UPDATE_INTERVAL);
    else
        m_updateScreenTimer->start(5s);

    return bRes;
}

void IdleScreen::IsShutdownLocked()
{
    bool wasLocked = m_shutdownLocked;

    if (gCoreContext->IsFrontendOnly())
    {
        // don't rely on the value from the cache it may be out of date
        gCoreContext->ClearSettingsCache(gCoreContext->GetHostName() + " MythFEShutdownLock");

        // check to see if the "MythFEShutdownLock has changed
        m_shutdownLocked = (gCoreContext->GetNumSettingOnHost("MythFEShutdownLock", gCoreContext->GetHostName(), 0) > 0);
    }
    else
    {
        // backend machine
        gCoreContext->ClearSettingsCache(gCoreContext->GetHostName() + " MythShutdownLock");

        // check to see if the "MythFEShutdownLock has changed
        m_shutdownLocked = (gCoreContext->GetNumSettingOnHost("MythShutdownLock", gCoreContext->GetHostName(), 0) > 0);
    }

    if (m_shutdownLocked)
    {
        m_shuttingDown = false;
        m_secondsToShutdown = -1s;
    }
    else
    {
        if (wasLocked && gCoreContext->IsFrontendOnly())
            m_secondsToShutdown = std::chrono::seconds(gCoreContext->GetNumSetting("FrontendShutdownTimeout", 0));
    }
}

void IdleScreen::UpdateShutdownCountdown()
{
    IsShutdownLocked();

    if (m_shutdownLocked)
    {
        // locked so abort shutdown
        UpdateStatus();
        return;
    }

    if (m_shuttingDown)
    {
        // we wait a maximum of 10 seconds before assuming the shutdown failed
        m_secondsToShutdown--;
        if (m_secondsToShutdown == 0s)
        {
            LOG(VB_GENERAL, LOG_ERR, "Failed to shutdown! Check the 'Halt Command' "
                                     "setting on the 'Shutdown/Reboot Settings' page.");
            m_secondsToShutdown = -1s;
            m_shutdownFailed = true;
        }
    }
    else
    {
        m_secondsToShutdown--;
        if (m_secondsToShutdown == 0s)
            DoShutdown();
    }
}

void IdleScreen::DoShutdown()
{
    QString poweroff_cmd = gCoreContext->GetSetting("HaltCommand", "");
    if (!poweroff_cmd.isEmpty())
    {
        m_shuttingDown = true;
        m_shutdownFailed = false;
        m_secondsToShutdown = 10s;
        myth_system(poweroff_cmd);
    }
}

void IdleScreen::UpdateStatus(void)
{
    QString state = "idle";

    if (m_shutdownFailed)
    {
        state = "error";
    }
    else if (m_secondsToShutdown >= 0s)
    {
        state = "shuttingdown";
    }
    else if ((m_FEShutdownEnabled || !gCoreContext->IsFrontendOnly()) && m_shutdownLocked)
        state = "locked";
    else if (CheckConnectionToServer())
    {
        if (RemoteGetRecordingStatus())
            state = "recording";
    }
    else
    {
        state = "offline";
    }

    m_statusState->DisplayState(state);

    MythUIType* shuttingdown = m_statusState->GetState("shuttingdown");

    if (shuttingdown)
    {
        MythUIText *statusText = dynamic_cast<MythUIText *>(shuttingdown->GetChild("status"));

        if (statusText)
        {
            QString status;

            if (m_shutdownLocked)
            {
                if (gCoreContext->IsFrontendOnly())
                {
                    if (m_FEShutdownEnabled)
                        status = tr("Frontend shutdown is locked");
                }
                else
                    status = tr("Backend shutdown is locked");
            }
            else if (gCoreContext->IsFrontendOnly() && m_shuttingDown)
                status = tr("Frontend is shutting down");
            else if (gCoreContext->IsFrontendOnly() &&  m_secondsToShutdown > 0s)
                status = tr("Frontend will shutdown in %n second(s)", "", m_secondsToShutdown.count());
            else if (m_secondsToShutdown >= 0s)
                status = tr("Backend will shutdown in %n second(s)", "", m_secondsToShutdown.count());

            if (!status.isEmpty())
                statusText->SetText(status);
            else
                statusText->Reset();
        }
    }
}

void IdleScreen::UpdateScreen(void)
{
    if (m_FEShutdownEnabled)
        UpdateShutdownCountdown();

    UpdateStatus();

    m_timeSinceLastUpdate++;

    if (m_timeSinceLastUpdate < IDLESCREEN_UPDATE_INTERVAL)
        return;

    if (m_currentRecordings)
    {
        m_currentRecordings->Reset();
        m_currentRecordings->SetCanTakeFocus(false);
    }

    if (m_nextRecordings)
    {
        m_nextRecordings->Reset();
        m_nextRecordings->SetCanTakeFocus(false);
    }

    if (m_conflictingRecordings)
    {
        m_conflictingRecordings->Reset();
        m_conflictingRecordings->SetCanTakeFocus(false);
    }

    if (m_conflictWarning)
        m_conflictWarning->SetVisible(m_hasConflicts);

    // update scheduled
    if (!m_scheduledList.empty())
    {
        auto pit = m_scheduledList.begin();

        while (pit != m_scheduledList.end())
        {
            ProgramInfo *progInfo = *pit;
            if (progInfo)
            {
                MythUIButtonList *list = nullptr;
                const RecStatus::Type recstatus = progInfo->GetRecordingStatus();

                switch(recstatus)
                {
                    case RecStatus::Recording:
                    case RecStatus::Tuning:
                    case RecStatus::Failing:
                        list = m_currentRecordings;
                        break;

                    case RecStatus::WillRecord:
                    case RecStatus::Pending:
                        list = m_nextRecordings;
                        break;

                    case RecStatus::Conflict:
                        list = m_conflictingRecordings;
                        break;

                    default:
                        list = nullptr;
                        break;
                }

                if (list != nullptr)
                {
                    auto *item = new MythUIButtonListItem(list,"",
                                                 QVariant::fromValue(progInfo));

                    InfoMap infoMap;
                    progInfo->ToMap(infoMap);
                    item->SetTextFromMap(infoMap, "");
                }
            }
            ++pit;
        }
    }

    m_timeSinceLastUpdate = 0;
}

bool IdleScreen::UpdateScheduledList()
{
    {
        // clear pending flag early in case something happens while
        // we're updating
        QMutexLocker lock(&m_schedUpdateMutex);
        SetPendingSchedUpdate(false);
    }

    m_scheduledList.clear();

    if (!gCoreContext->IsConnectedToMaster())
    {
        return false;
    }

    if (!LoadFromScheduler(m_scheduledList, m_hasConflicts))
        return false;

    UpdateScreen();

    return true;
}

bool IdleScreen::keyPressEvent(QKeyEvent* event)
{
    QStringList actions;
    bool handled = GetMythMainWindow()->TranslateKeyPress("Global", event, actions, true);

    for (int i = 0; i < actions.size() && !handled; i++)
    {
        QString action = actions[i];
        handled = true;

        if (action == "ESCAPE")
        {
            if (m_secondsToShutdown >= 0s)
            {
                m_secondsToShutdown = -1s;
                UpdateStatus();
            }
            else
                handled = false;
        }
        else
            handled = false;
    }

    if (!handled)
        handled = MythScreenType::keyPressEvent(event);

    return handled;
}

void IdleScreen::customEvent(QEvent* event)
{
    if (event->type() == MythEvent::MythEventMessage)
    {
        auto *me = dynamic_cast<MythEvent *>(event);
        if (me == nullptr)
            return;

        if (me->Message().startsWith("RECONNECT_"))
        {
            m_secondsToShutdown = -1s;
            UpdateStatus();
        }
        else if (me->Message().startsWith("SHUTDOWN_COUNTDOWN"))
        {
            QString secs = me->Message().mid(19);
            m_secondsToShutdown = std::chrono::seconds(secs.toInt());
            UpdateStatus();
        }
        else if (me->Message().startsWith("SHUTDOWN_NOW"))
        {
            if (gCoreContext->IsFrontendOnly())
            {
                // does the user want to shutdown this frontend only machine
                // when the BE shuts down?
                if (gCoreContext->GetNumSetting("ShutdownWithMasterBE", 0) == 1)
                {
                     LOG(VB_GENERAL, LOG_NOTICE,
                         "Backend has gone offline, Shutting down frontend");
                     DoShutdown();
                }
            }
        }
        else if (me->Message().startsWith("SCHEDULE_CHANGE") ||
                 me->Message().startsWith("RECORDING_LIST_CHANGE") ||
                 me->Message() == "UPDATE_PROG_INFO")
        {
            QMutexLocker lock(&m_schedUpdateMutex);

            if (!PendingSchedUpdate())
            {
                QTimer::singleShot(50ms, this, &IdleScreen::UpdateScheduledList);
                SetPendingSchedUpdate(true);
            }
        }
    }

    MythUIType::customEvent(event);
}
