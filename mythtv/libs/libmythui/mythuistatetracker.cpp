#include <chrono> // for milliseconds
#include <thread> // for sleep_for

#include <QCoreApplication>
#include "mythmainwindow.h"
#include "mythevent.h"
#include "mythuistatetracker.h"
#include "mythuiactions.h"

MythUIStateTracker::MythUIStateTracker()
  : m_lastUpdated(QTime::currentTime().addSecs(-1))
{
}

MythUIStateTracker* MythUIStateTracker::GetMythUIStateTracker()
{
    gUIStateLock.lock();
    if (!gUIState)
        gUIState = new MythUIStateTracker();
    gUIStateLock.unlock();
    return gUIState;
}

void MythUIStateTracker::SetState(const QVariantMap& NewState)
{
    auto * state = MythUIStateTracker::GetMythUIStateTracker();
    gUIStateLock.lock();
    state->m_state = NewState;
    state->m_lastUpdated = QTime::currentTime();
    gUIStateLock.unlock();
}

void MythUIStateTracker::GetState(QVariantMap &State)
{
    auto * state = MythUIStateTracker::GetMythUIStateTracker();
    gUIStateLock.lock();
    State = state->m_state;
    gUIStateLock.unlock();
}

void MythUIStateTracker::GetFreshState(QVariantMap &State)
{
    if (MythUIStateTracker::TimeSinceLastUpdate() < 500)
    {
        MythUIStateTracker::GetState(State);
        return;
    }

    auto * event = new MythEvent(ACTION_GETSTATUS);
    qApp->postEvent(GetMythMainWindow(), event);

    int tries = 0;
    while ((tries++ < 100) && (MythUIStateTracker::TimeSinceLastUpdate() >= 500))
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

    MythUIStateTracker::GetState(State);
}

int MythUIStateTracker::TimeSinceLastUpdate()
{
    auto * state = MythUIStateTracker::GetMythUIStateTracker();
    gUIStateLock.lock();
    int age = state->m_lastUpdated.msecsTo(QTime::currentTime());
    gUIStateLock.unlock();
    return age < 0 ? 1000000 : age;
}
