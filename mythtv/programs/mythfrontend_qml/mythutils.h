#pragma once

// qt
#include <QObject>
#include <QDateTime>
#include <QQmlApplicationEngine>

// mythfrontend_qml
#include "settings.h"

class MythUtils : public QObject
{
    Q_OBJECT
  public:
      MythUtils(Settings *settings, QQmlApplicationEngine* engine) {m_settings = settings; m_engine = engine;}

    Q_INVOKABLE QString findThemeFile(const QString &fileName);
    Q_INVOKABLE bool grabScreen(const QString &fileName);
    Q_INVOKABLE bool fileExists(const QString &fileName);
    Q_INVOKABLE QDateTime addMinutes(const QDateTime &dateTime, int minutes);

  private:
    Settings *m_settings;
    QQmlApplicationEngine *m_engine;
};
