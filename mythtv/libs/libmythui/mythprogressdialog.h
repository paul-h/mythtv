#ifndef MYTHPROGRESSBOX_H_
#define MYTHPROGRESSBOX_H_

#include <QEvent>
#include <QMutex>

#include "mythscreentype.h"

class MythUIText;
class MythUIProgressBar;

class MUI_PUBLIC ProgressUpdateEvent : public QEvent
{
  public:
    ProgressUpdateEvent(uint count, uint total=0, QString message="") :
        QEvent(kEventType), m_total(total), m_count(count),
        m_message(message) { }

    QString GetMessage() { return m_message; }
    uint GetTotal() { return m_total; }
    uint GetCount() { return m_count; }

    static Type kEventType;

  private:
    uint m_total;
    uint m_count;
    QString m_message;
};

class MUI_PUBLIC MythUIBusyDialog : public MythScreenType
{
    Q_OBJECT
  public:
    MythUIBusyDialog(const QString &message,
                  MythScreenStack *parent, const char *name);

    bool Create(void) override; // MythScreenType
    bool keyPressEvent(QKeyEvent *event) override; // MythScreenType
    void SetMessage(const QString &message);
    void Reset(void) override; // MythUIType
    void Pulse(void) override; // MythUIType

  protected:
    QString m_origMessage;
    QString m_message;
    bool    m_haveNewMessage;
    QString m_newMessage;
    QMutex  m_newMessageLock;

    MythUIText *m_messageText;
};

class MUI_PUBLIC MythUIProgressDialog : public MythScreenType
{
    Q_OBJECT
  public:
    MythUIProgressDialog(const QString &message,
                  MythScreenStack *parent, const char *name);

    bool Create(void) override; // MythScreenType
    bool keyPressEvent(QKeyEvent *event) override; // MythScreenType
    void customEvent(QEvent *event) override; // MythUIType
    void SetTotal(uint total);
    void SetProgress(uint count);
    void SetMessage(const QString &message);

  protected:
    void UpdateProgress(void);

    QString m_message;
    uint m_total;
    uint m_count;

    MythUIText *m_messageText;
    MythUIText *m_progressText;
    MythUIProgressBar *m_progressBar;
};

MUI_PUBLIC MythUIBusyDialog  *ShowBusyPopup(const QString &message);

#endif
