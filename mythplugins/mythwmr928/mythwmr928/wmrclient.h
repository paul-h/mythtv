#ifndef WMRCLIENT_H_
#define WMRCLIENT_H_

using namespace std;

#include <iostream>

// qt
#include <QObject>
#include <QStringList>
#include <QString>
#include <QTimer>
#include <QDateTime>
#include <QMutex>
#include <QSize>

// myth
#include <mythsocket.h>
#include <mythtv/mythexp.h>

class MythImage;

typedef struct 
{
    double  outsideTemp;
    int     outsideHum;
    double  outsideDew;
    double  insideTemp;
    int     insideHum;
    double  insideDew;
    int     pressure;
    int     sealevel;
    int     forecast;
    double  windSpeed;
    double  windGust;
    int     windDir;
    QString windDir2;
    double  windChill;
    double  rainRate;
    double  rainYest;
    double  rainTotal;
}CurrentConditions;

class MPUBLIC WMRClient : public QObject
{
    Q_OBJECT

  protected:
    WMRClient();

    static bool m_server_unavailable;
    static class WMRClient *m_wmrclient;

  public:
    ~WMRClient();

    static class WMRClient *get(void);
    static bool setupWMRClient (void);

    // Used to actually connect to an WMR server
    bool connectToHost(const QString &hostname, unsigned int port);
    bool connected(void) { return m_bConnected; }

    bool checkProtoVersion(void);

    // If you want to be pleasant, call shutdown() before deleting your WMRClient 
    // device
    void shutdown();

    void getCurrentCond(CurrentConditions *cond);
    void getChart(const QString &type, const QDateTime &startTime,
                  const QDateTime endTime, const QString &res,
                  const QSize &size, MythImage **image);

  private slots:
    void restartConnection(void);  // Try to re-establish the connection to 
                                   // WMRServer every 10 seconds
  private:
    bool readData(unsigned char *data, int dataSize);
    bool sendReceiveStringList(QStringList &strList);

    MythSocket       *m_socket;
    QMutex            m_socketLock;
    QString           m_hostname;
    uint              m_port;
    bool              m_bConnected;
    QTimer           *m_retryTimer;
    bool              m_wmrclientReady;
};

#endif
