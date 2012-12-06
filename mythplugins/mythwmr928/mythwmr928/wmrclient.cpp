/*
    zmclient.cpp
*/

#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include <netinet/in.h>
#include <assert.h>

// qt
#include <qapplication.h>
#include <qregexp.h>

// myth
#include <mythcontext.h>
#include <mythdialogs.h>
#include <mythtimer.h>
#include <mythimage.h>

// wmr928
#include "wmrclient.h"

// the protocol version we understand
#define WMR_PROTOCOL_VERSION "1"

//#define BUFFER_SIZE  (2048*1536*3)

WMRClient::WMRClient()
    : QObject(NULL),
      m_socket(NULL),
      m_socketLock(QMutex::Recursive),
      m_hostname("localhost"),
      m_port(6548),
      m_bConnected(false),
      m_retryTimer(new QTimer(this)),
      m_wmrclientReady(false)
{
    setObjectName("WMRClient");
    connect(m_retryTimer, SIGNAL(timeout()),   this, SLOT(restartConnection()));
}

bool  WMRClient::m_server_unavailable = false;
class WMRClient *WMRClient::m_wmrclient = NULL;

class WMRClient *WMRClient::get(void)
{
    if (m_wmrclient == NULL && m_server_unavailable == false)
        m_wmrclient = new WMRClient;
    return m_wmrclient;
}

bool WMRClient::setupWMRClient(void) 
{
    QString wmrserver_host;
    int wmrserver_port;

    if (m_wmrclient) 
    {
        delete m_wmrclient;
        m_wmrclient = NULL;
        m_server_unavailable = false;
    }

    wmrserver_host = gCoreContext->GetSetting("WMR928ServerIP", "localhost");
    wmrserver_port = gCoreContext->GetNumSetting("WMR928ServerPort", 6549);

    class WMRClient *wmrclient = WMRClient::get();
    if (wmrclient->connectToHost(wmrserver_host, wmrserver_port) == false) 
    {
        delete m_wmrclient;
        m_wmrclient = NULL;
        m_server_unavailable = false;
        return false;
    }

    return true;
}

bool WMRClient::connectToHost(const QString &lhostname, unsigned int lport)
{
    QMutexLocker locker(&m_socketLock);

    m_hostname = lhostname;
    m_port = lport;

    m_bConnected = false;
    int count = 0;
    do
    {
        ++count;

         LOG(VB_GENERAL, LOG_INFO, 
             QString("Connecting to wmr server: %1:%2 (try %3 of 10)")
                     .arg(m_hostname).arg(m_port).arg(count));

        if (m_socket)
        {
            m_socket->DecrRef();
            m_socket = NULL;
        }

        m_socket = new MythSocket();
        //m_socket->setCallbacks(this);
        if (!m_socket->ConnectToHost(m_hostname, m_port))
        {
            m_socket->DecrRef();
            m_socket = NULL;
        }
        else
        {
            m_wmrclientReady = true;
            m_bConnected = true;
        }

        usleep(500000);

    } while (count < 10 && !m_bConnected);

    if (!m_bConnected)
    {
        MythPopupBox::showOkPopup(GetMythMainWindow(), "Connection failure",
                              tr("Cannot connect to the mythwmrserver - Is it running? " 
                                 "Have you set the correct IP and port in the settings?"));
    }

    // check the server uses the same protocol as us
    if (m_bConnected && !checkProtoVersion())
    {
        m_wmrclientReady = false;
        m_bConnected = false;
    }

    if (m_bConnected == false)
        m_server_unavailable = true;

    return m_bConnected;
}

bool WMRClient::sendReceiveStringList(QStringList &strList)
{
    QStringList origStrList = strList;

    bool ok = false;
    if (m_bConnected)
        ok = m_socket->SendReceiveStringList(strList);

    if (!ok)
    {
        LOG(VB_GENERAL, LOG_ERR, "Connection to mythwmrserver lost");

        if (!connectToHost(m_hostname, m_port))
        {
            LOG(VB_GENERAL, LOG_ERR, "Re connection to mythwmrserver failed");
            return false;
        }

        // try to resend 
        strList = origStrList;
        ok = m_socket->SendReceiveStringList(strList);
        if (!ok)
        {
            m_bConnected = false;
            return false;
        }
    }

    if (strList.empty())
        return false;

    // the server sends "UNKNOWN_COMMAND" if it did not recognise the command
    if (strList[0] == "UNKNOWN_COMMAND")
    {
        LOG(VB_GENERAL, LOG_ERR, "Somethings is getting passed to the server that it doesn't understand");
        return false;
    }

    // the server sends "ERROR" if it failed to process the command
    if (strList[0].startsWith("ERROR"))
    {
         LOG(VB_GENERAL, LOG_ERR, QString("The server failed to process the command. "
                                          "The error was:- \n\t\t\t%1").arg(strList[0]));
        return false;
    }

    // we should get "OK" from the server if everything is OK
    if (strList[0] != "OK")
        return false;

    return true;
}

bool WMRClient::checkProtoVersion(void)
{
    QStringList strList("HELLO");
    if (!sendReceiveStringList(strList))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("Server didn't respond to 'HELLO'!!"));

        MythPopupBox::showOkPopup(GetMythMainWindow(), "Connection failure",
            tr("The mythwmrserver didn't respond to our request to get the protocol version!!"));
        return false;
    }

    if (strList[1] != WMR_PROTOCOL_VERSION)
    {
         LOG(VB_GENERAL, LOG_ERR, 
             QString("Protocol version mismatch (plugin=%1, mythwmrserver=%2)")
                     .arg(WMR_PROTOCOL_VERSION).arg(strList[1]));

        MythPopupBox::showOkPopup(GetMythMainWindow(), "Connection failure",
                         tr("The mythwmrserver uses protocol version %1, "
                            "but this client only understands version %2. "
                            "Make sure you are running compatible versions of "
                            "both the server and plugin.")
                            .arg(strList[1]).arg(WMR_PROTOCOL_VERSION));
        return false;
    }

    LOG(VB_GENERAL, LOG_INFO, QString("Using protocol version %1").arg(WMR_PROTOCOL_VERSION));
    return true;
}

void WMRClient::restartConnection()
{
    // Reset the flag
    m_wmrclientReady = false;
    m_bConnected = false;
    m_server_unavailable = false;

    // Retry to connect. . .  Maybe the user restarted mythwmrserver?
    connectToHost(m_hostname, m_port);
}

void WMRClient::shutdown()
{
    QMutexLocker locker(&m_socketLock);

    if (m_socket)
        m_socket->DisconnectFromHost();

    m_wmrclientReady = false;
    m_bConnected = false;
}

WMRClient::~WMRClient()
{
    m_wmrclient = NULL;

    if (m_socket)
    {
        m_socket->DecrRef();
        m_socket = NULL;
        m_wmrclientReady = false;
    }

    if (m_retryTimer)
        delete m_retryTimer;
}

void WMRClient::getCurrentCond(CurrentConditions *cond)
{
    QStringList strList("GET_CURRENT_CONDITIONS");
    if (!sendReceiveStringList(strList))
        return;

    QStringList::Iterator it = strList.begin();
    it++;
    while (it != strList.end())
    {
        if (!(*it).isEmpty())
        {
            cout << "processing '" << qPrintable(*it) << "'" << endl;
            if ((*it).startsWith("thb0-temp"))
                cond->insideTemp = (*it).mid(10).toDouble();
            else if ((*it).startsWith("thb0-hum"))
                cond->insideHum = (*it).mid(9).toInt();
            else if ((*it).startsWith("thb0-dew"))
                cond->insideDew = (*it).mid(9).toDouble();
            else if ((*it).startsWith("thb0-press"))
                cond->pressure = (*it).mid(11).toInt();
            else if ((*it).startsWith("thb0-fc"))
                cond->forecast = (*it).mid(8).toInt();
            else if ((*it).startsWith("thb0-sealevel"))
                cond->sealevel = (*it).mid(14).toInt();
            else if ((*it).startsWith("th0-temp"))
                cond->outsideTemp = (*it).mid(9).toDouble();
            else if ((*it).startsWith("th0-hum"))
                cond->outsideHum = (*it).mid(8).toInt();
            else if ((*it).startsWith("th0-dew"))
                cond->outsideDew = (*it).mid(8).toDouble();
            else if ((*it).startsWith("wind0-dir "))
                cond->windDir = (*it).mid(10).toInt();
            else if ((*it).startsWith("wind0-dir2"))
                cond->windDir2 = (*it).mid(11);
            else if ((*it).startsWith("wind0-speed"))
                cond->windSpeed = (*it).mid(12).toDouble();
            else if ((*it).startsWith("gust0-speed"))
                cond->windGust = (*it).mid(12).toDouble();
            else if ((*it).startsWith("wind0-chill"))
                cond->windChill = (*it).mid(12).toDouble();
            else if ((*it).startsWith("rain0-rate"))
                cond->rainRate = (*it).mid(11).toDouble();
            else if ((*it).startsWith("rain0-yest"))
                cond->rainYest = (*it).mid(11).toDouble();
            else if ((*it).startsWith("rain0-total"))
                cond->rainTotal = (*it).mid(12).toDouble();
            else
                cout << "unknown condition: " << qPrintable(*it) << endl;

        }

        it++;
    }
}

#if 0
void WMRClient::getEventDates(const QString &monitorName, bool oldestFirst,
                            QStringList &dateList)
{
    dateList.clear();

    QStringList strList = "GET_EVENT_DATES";
    strList << monitorName << (oldestFirst ? "1" : "0") ;

    if (!sendReceiveStringList(strList))
        return;

    bool bOK;
    int dateCount = strList[1].toInt(&bOK);
    if (!bOK)
    {
        VERBOSE(VB_IMPORTANT, "WMRClient received bad int in getEventDates()");
        return;
    }

    // sanity check 
    if ((int)(strList.size() - 3) != dateCount)
    {
        VERBOSE(VB_IMPORTANT, "WMRClient got a mismatch between the number of dates and "
                "the expected number of stringlist items in getEventDates()");
        return;
    }

    QStringList::Iterator it = strList.begin();
    it++; it++;
    for (int x = 0; x < dateCount; x++)
    {
        dateList.append(*it++);
    }
}
#endif

bool WMRClient::readData(unsigned char *data, int dataSize)
{
    qint64 read = 0;
    int errmsgtime = 0;
    MythTimer timer;
    timer.start();
    int elapsed;

    while (dataSize > 0)
    {
        qint64 sret = m_socket->Read((char*) data + read, dataSize, 100);
        if (sret > 0)
        {
            read += sret;
            dataSize -= sret;
            if (dataSize > 0)
            {
                timer.start();
            }
        }
        else if (sret < 0)
        {
            LOG(VB_GENERAL, LOG_ERR, "readData: Error, readBlock");
            m_socket->DisconnectFromHost();
            return false;
        }
        else if (!m_socket->IsConnected())
        {
            LOG(VB_GENERAL, LOG_ERR, "readData: Error, socket went unconnected");
            m_socket->DisconnectFromHost();
            return false;
        }
        else
        {
            elapsed = timer.elapsed();
            if (elapsed  > 10000)
            {
                if ((elapsed - errmsgtime) > 10000)
                {
                    errmsgtime = elapsed;
                    LOG(VB_GENERAL, LOG_ERR, 
                        QString("m_socket->: Waiting for data: %1 %2")
                                .arg(read).arg(dataSize));
                }
            }

            if (elapsed > 100000)
            {
                LOG(VB_GENERAL, LOG_ERR, "Error, readData timeout (readBlock)");
                return false;
            }
        }
    }

    return true;
}
#if 0
int WMRClient::getLiveFrame(int monitorID, QString &status, unsigned char* buffer, int bufferSize)
{
    QStringList strList = "GET_LIVE_FRAME";
    strList << QString::number(monitorID);
    if (!sendReceiveStringList(strList))
    {
        // the server sends a "WARNING" message if there is no new frame available
        // we can safely ignore it
        if (strList[0].startsWith("WARNING"))
            return 0;
        else
        {
            status = strList[0];
            return 0;
        }
    }

    // get status
    status = strList[2];

    // get frame length from data
    int imageSize = strList[3].toInt();

    assert(bufferSize > imageSize);

    // read the frame data
    if (imageSize == 0)
        return 0;

    if (!readData(buffer, imageSize))
    {
        VERBOSE(VB_GENERAL, "WMRClient::getLiveFrame(): Failed to get image data");
        return 0;
    }

    return imageSize;
}
#endif

void WMRClient::getChart(const QString &type, const QDateTime &startTime,
                         const QDateTime endTime, const QString &res,
                         const QSize &size, MythImage **image)
{
    if (*image)
    {
        (*image)->DecrRef();
        *image = NULL;
    }

    QStringList strList("GET_CHART");
    strList << type;
    strList << startTime.toString("yyyyMMddhhmmss");
    strList << endTime.toString("yyyyMMddhhmmss");
    strList << res;
    strList << QString::number(size.width());
    strList << QString::number(size.height());

    if (!sendReceiveStringList(strList))
        return;

    // get image size from data
    int imageSize = strList[1].toInt();

    // grab the image data
    unsigned char *data = new unsigned char[imageSize];
    if (!readData(data, imageSize))
    {
        LOG(VB_GENERAL, LOG_ERR, "WMRClient::getChart(): Failed to get image data");
        return;
    }

    // get a MythImage
    *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
    //(*image)->DecrRef();

    // extract the image data and create a MythImage from it
    if (!(*image)->loadFromData(data, imageSize, "PNG"))
    {
        LOG(VB_GENERAL, LOG_ERR, "WMRClient::getChart(): Failed to load image from data");
        return;
    }
}
