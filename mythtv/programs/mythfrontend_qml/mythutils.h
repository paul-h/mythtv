#pragma once

// qt
#include <QObject>

// mythfrontend_qml
#include "settings.h"

class MythUtils : public QObject
{
    Q_OBJECT
  public:
    MythUtils(Settings *settings) {m_settings = settings;}

    Q_INVOKABLE QString findThemeFile(const QString &fileName);

  private:
    Settings *m_settings;
};