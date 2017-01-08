#pragma once

// qt
#include <QObject>
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

  private:
    Settings *m_settings;
    QQmlApplicationEngine *m_engine;
};