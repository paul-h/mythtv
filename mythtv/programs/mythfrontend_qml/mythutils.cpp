#include <iostream>

#include <QFile>
#include <QString>

#include "mythutils.h"

QString MythUtils::findThemeFile(const QString &fileName)
{
    // do we have a full path
    if (QFile::exists(fileName))
        return fileName;

    // look in the active theme
    if (QFile::exists(m_settings->qmlPath().remove("file://") + fileName))
        return m_settings->qmlPath() + fileName;

    // look in the default theme
    if (QFile::exists(m_settings->sharePath().remove("file://") + "qml/Themes/default-wide/" + fileName))
        return m_settings->sharePath() + "qml/Themes/default-wide/" + fileName;

    // not found
    return QString();
}

