// c/c++
#include <iostream>

// qt
#include <QDebug>
#include <QFile>

// mythfrontend_qml
#include "urlinterceptor.h"

QUrl MythQmlAbstractUrlInterceptor::intercept(const QUrl &url, DataType type)
{
    QString sUrl = url.toString();

    // we are only interested in our theme urls
    if (!sUrl.startsWith(m_settings->sharePath()) || sUrl.endsWith("qmldir"))
        return url;

    qDebug() << "looking for URL: " << sUrl;

    // look in the map first
    if (m_fileMap.contains(sUrl))
    {
        qDebug() << "+ found in map: " << m_fileMap.value(sUrl);
        return QUrl("file://" + m_fileMap.value(sUrl));
    }

    QString fileName = sUrl;

    if (fileName.startsWith(m_defaultThemePath))
        fileName.remove(m_defaultThemePath);

    if (fileName.startsWith(m_activeThemePath))
        fileName.remove(m_activeThemePath);

    // look up in the active theme
    if (!m_defaultThemePath.isEmpty())
    {
        QString searchURL = m_activeThemePath + fileName;

        if (QFile::exists(searchURL.remove("file://")))
        {
            qDebug() << "= found in active theme: " << searchURL;
            m_fileMap.insert(sUrl, searchURL);
            return QUrl("file://" + searchURL);
        }
        else
            qDebug() << "- NOT found in active theme: " << searchURL;
    }

    // look up in the default theme
    if (!m_defaultThemePath.isEmpty())
    {
        QString searchURL = m_defaultThemePath + fileName;

        if (QFile::exists(searchURL.remove("file://")))
        {
            qDebug() << "= found in default theme: " << searchURL;
            m_fileMap.insert(sUrl, searchURL);
            return QUrl("file://" + searchURL);
        }
        else
            qDebug() << "- NOT found in default theme: " << searchURL;
    }

    // fall back to the original url
    m_fileMap.insert(sUrl, sUrl);
    return url;
}

void MythQmlAbstractUrlInterceptor::setTheme(const QString& theme)
{
   m_theme = theme;

   m_defaultThemePath = m_settings->sharePath() + "qml/Themes/default-wide";
   m_activeThemePath = m_settings->sharePath() + "qml/Themes/" + m_theme;

   m_fileMap.clear();
}
