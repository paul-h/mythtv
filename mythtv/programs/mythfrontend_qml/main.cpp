// qt
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <QHostInfo>
#include <QDomDocument>
#include <QFile>
#include <QDir>

// from QmlVlc
#include <QmlVlc.h>
#include <QmlVlc/QmlVlcConfig.h>

// mythfrontend_qml
#include "sqlquerymodel.h"
#include <databaseutils.h>

#define SHAREPATH "file:///usr/share/mythtv/"

static QString dbHost;
static QString dbPort;
static QString dbUser;
static QString dbPassword;
static QString dbName;

static bool loadDBSettings(void)
{
    QDomDocument doc("mydocument");
    QFile file(QDir::homePath() + "/.mythtv/config.xml");
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open config file";
        return false;
    }

    if (!doc.setContent(&file))
    {
        file.close();
        qDebug() << "Failed to read from config file";
        return false;
    }
    file.close();

    QString docType = doc.doctype().name();
    QDomNodeList dbNodeList;
    QDomNode node;
    QDomElement elem;

    dbNodeList = doc.elementsByTagName("Database");

    if (dbNodeList.count() != 1)
    {
        qDebug() << "Expected 1 'Database' node but got " << dbNodeList.count();
        return false;
    }

    node = dbNodeList.at(0);
    dbHost = node.namedItem(QString("Host")).toElement().text();
    dbUser = node.namedItem(QString("UserName")).toElement().text();
    dbPassword = node.namedItem(QString("Password")).toElement().text();
    dbName = node.namedItem(QString("DatabaseName")).toElement().text();
    dbPort = node.namedItem(QString("Port")).toElement().text();

    return true;
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setApplicationName("MythFrontendQML");
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);

    RegisterQmlVlc();
    QmlVlcConfig& config = QmlVlcConfig::instance();
    config.enableAdjustFilter( true );
    config.enableMarqueeFilter(false);
    config.enableLogoFilter(false);
    config.enableDebug(false);
    config.enableLoopPlayback(false);
    config.setTrustedEnvironment(true);

    QQmlApplicationEngine engine;

    // get the database login details from ~/.mythtv/config.xml
    if (!loadDBSettings())
    {
        qDebug() << "Failed to load database config";
        return 1;
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(dbHost);
    db.setDatabaseName(dbName);
    db.setUserName(dbUser);
    db.setPassword(dbPassword);
    bool ok = db.open();

    if (!ok)
    {
        qDebug() << "Failed to open the database";
        return 1;
    }

    engine.rootContext()->setContextProperty("hostName", QHostInfo::localHostName());
    engine.rootContext()->setContextProperty("sharePath", QString(SHAREPATH));
    engine.rootContext()->setContextProperty("themePath", QString(SHAREPATH) + "themes/MythCenter-wide/");
    engine.rootContext()->setContextProperty("qmlPath",   QString(SHAREPATH) + "qml/Themes/MythCenter-wide/");

    // create the radio streams model
    SqlQueryModel *radioStreamsModel = new SqlQueryModel(&engine);
    radioStreamsModel->setQuery("SELECT * FROM music_radios", db);
    engine.rootContext()->setContextProperty("radioStreamsModel", radioStreamsModel);

    // create the tv channels model
    SqlQueryModel *dbChannelsModel = new SqlQueryModel(&engine);
    dbChannelsModel->setQuery("SELECT chanid, channum, callsign, name, icon, xmltvid FROM channel ORDER BY cast(channum as unsigned);", db);
    engine.rootContext()->setContextProperty("dbChannelsModel", dbChannelsModel);

    // create the database utils
    DatabaseUtils databaseUtils(db);
    engine.rootContext()->setContextProperty("dbUtils", &databaseUtils);

    engine.addImportPath(QString(SHAREPATH) + "qml");

    engine.load(QUrl(QString(SHAREPATH) + "qml/main.qml"));

    return app.exec();
}

