#pragma once
#include <QQmlAbstractUrlInterceptor>
#include <QQmlApplicationEngine>

#include "settings.h"

class MythQmlAbstractUrlInterceptor : public QQmlAbstractUrlInterceptor
{
  public:
    MythQmlAbstractUrlInterceptor(QQmlApplicationEngine *engine, Settings *settings) { m_qmlEngine = engine; m_settings = settings;}

    QUrl intercept(const QUrl &url, DataType type);

    void setTheme(const QString &theme);

  private:
    QQmlApplicationEngine *m_qmlEngine;
    Settings *m_settings;
    QString m_theme;
    QString m_activeThemePath;
    QString m_defaultThemePath;

    QMap<QString, QString> m_fileMap;
};