#ifndef IDLESCREEN_H
#define IDLESCREEN_H

#include <mythscreentype.h>
// libmyth
#include "programinfo.h"

class MythUIStateType;
class MythUIButtonList;
class QTimer;

#define IDLESCREEN_UPDATE_INTERVAL   15000

class IdleScreen : public MythScreenType
{
    Q_OBJECT

  public:
    explicit IdleScreen(MythScreenStack *parent);
    ~IdleScreen() override;

    bool Create(void) override; // MythScreenType
    bool keyPressEvent(QKeyEvent *event) override; // MythScreenType
    void customEvent(QEvent *e) override; // MythUIType


  public slots:
    void UpdateShutdownCountdown();
    void UpdateStatus(void);
    void UpdateScreen(void);
    bool UpdateScheduledList();

  protected:
    void Load(void) override; // MythScreenType
    void Init(void) override; // MythScreenType

  private:
    bool CheckConnectionToServer(void);
    bool PendingSchedUpdate() const             { return m_pendingSchedUpdate; }
    void SetPendingSchedUpdate(bool newState)   { m_pendingSchedUpdate = newState; }
    void IsShutdownLocked(void);
    void DoShutdown(void);

    QTimer           *m_updateScreenTimer     {nullptr};
    int               m_timeSinceLastUpdate   {IDLESCREEN_UPDATE_INTERVAL};

    MythUIStateType  *m_statusState           {nullptr};
    MythUIButtonList *m_currentRecordings     {nullptr};
    MythUIButtonList *m_nextRecordings        {nullptr};
    MythUIButtonList *m_conflictingRecordings {nullptr};
    MythUIText       *m_conflictWarning       {nullptr};

    std::chrono::seconds  m_secondsToShutdown {-1s};
    int             m_FEShutdownEnabled       {false};
    bool            m_shutdownLocked          {false};
    bool            m_shuttingDown            {false};
    bool            m_shutdownFailed          {false};

    QMutex          m_schedUpdateMutex;
    bool            m_pendingSchedUpdate      {false};
    ProgramList     m_scheduledList;
    bool            m_hasConflicts            {false};
};

#endif // IDLESCREEN_H
