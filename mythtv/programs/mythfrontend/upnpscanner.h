#ifndef UPNPSCANNER_H
#define UPNPSCANNER_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDomDocument>
#include <QMutex>
#include <QTimer>

#include "upnpsubscription.h"
#include "mthread.h"
#include "upnpexp.h"

#include "videometadatalistmanager.h"

class MediaServer;
class UPNPSubscription;
class meta_dir_node;

class MediaServerItem
{
  public:
    MediaServerItem() : m_scanned(false) { }
    MediaServerItem(const QString& id, const QString& parent,
                    const QString& name, const QString& url)
      : m_id(id), m_parentid(parent), m_name(name), m_url(url) { }
    QString NextUnbrowsed(void);
    MediaServerItem* Find(QString &id);
    bool Add(MediaServerItem &item);
    void Reset(void);

    QString m_id;
    QString m_parentid;
    QString m_name;
    QString m_url;
    bool    m_scanned {false};
    QMap<QString, MediaServerItem> m_children;
};

class UPNPScanner : public QObject
{
   Q_OBJECT

  public:
   ~UPNPScanner();

    static void         Enable(bool enable, UPNPSubscription *sub = nullptr);
    static UPNPScanner* Instance(UPNPSubscription *sub = nullptr);

    void StartFullScan(void);
    void GetInitialMetadata(VideoMetadataListManager::metadata_list* list,
                            meta_dir_node *node);
    void GetMetadata(VideoMetadataListManager::metadata_list* list,
                     meta_dir_node *node);
    bool GetMetadata(QVariant &data);
    QMap<QString,QString> ServerList(void);

  protected:
    void customEvent(QEvent *event) override; // QObject
    void timerEvent(QTimerEvent * event) override; // QObject

  private slots:
    void Start();
    void Stop(void);
    void Update(void);
    void CheckStatus(void);
    void replyFinished(QNetworkReply *reply);

  private:
    explicit UPNPScanner(UPNPSubscription *sub)
        : QObject(), m_subscription(sub) {}
    void ScheduleUpdate(void);
    void CheckFailure(const QUrl &url);
    void Debug(void);
    void BrowseNextContainer(void);
    void SendBrowseRequest(const QUrl &url, const QString &objectid);
    void AddServer(const QString &usn, const QString &url);
    void RemoveServer(const QString &usn);
    void ScheduleRenewal(const QString &usn, int timeout);

    // xml parsing of browse requests
    void ParseBrowse(const QUrl &url, QNetworkReply *reply);
    void FindItems(const QDomNode &n, MediaServerItem &content,
                   bool &resetparent);
    QDomDocument* FindResult(const QDomNode &n, uint &num,
                             uint &total, uint &updateid);

    // xml parsing of device description
    bool ParseDescription(const QUrl &url, QNetworkReply *reply);
    void ParseDevice(QDomElement &element, QString &controlURL,
                     QString &eventURL, QString &friendlyName);
    void ParseServiceList(QDomElement &element, QString &controlURL,
                          QString &eventURL);
    void ParseService(QDomElement &element, QString &controlURL,
                      QString &eventURL);

    // convert MediaServerItems to video metadata
    void GetServerContent(QString &usn, MediaServerItem *content,
                          VideoMetadataListManager::metadata_list* list,
                          meta_dir_node *node);

  private:
    static  UPNPScanner* gUPNPScanner;
    static  bool         gUPNPScannerEnabled;
    static  MThread*     gUPNPScannerThread;
    static  QMutex*      gUPNPScannerLock;

    UPNPSubscription *m_subscription {nullptr};
    QMutex  m_lock {QMutex::Recursive};
    QHash<QString,MediaServer*> m_servers;
    QNetworkAccessManager *m_network {nullptr};
    // TODO Move to QMultiHash when we move to Qt >=4.7
    // QHash(QUrl) unsupported on < 4.7
    QMultiMap<QUrl, QNetworkReply*> m_descriptionRequests;
    QMultiMap<QUrl, QNetworkReply*> m_browseRequests;

    QTimer *m_updateTimer {nullptr};
    QTimer *m_watchdogTimer {nullptr};

    QString m_masterHost;
    int     m_masterPort {0};

    bool    m_scanComplete {false};
    bool    m_fullscan {false};
};

#endif // UPNPSCANNER_H
