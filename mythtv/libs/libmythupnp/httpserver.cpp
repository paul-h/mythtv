//////////////////////////////////////////////////////////////////////////////
// Program Name: httpserver.cpp
// Created     : Oct. 1, 2005
//
// Purpose     : HTTP 1.1 Mini Server Implmenetation
//               Used for UPnp/AV implementation & status information
//                                                                            
// Copyright (c) 2005 David Blain <dblain@mythtv.org>
//                                          
// Licensed under the GPL v2 or later, see COPYING for details                    
//
//////////////////////////////////////////////////////////////////////////////

// ANSI C headers
#include <cmath>

// POSIX headers
#include <compat.h>
#ifndef _WIN32
#include <sys/utsname.h> 
#endif

// Qt headers
#include <QScriptEngine>

// MythTV headers
#include "httpserver.h"
#include "upnputil.h"
#include "upnp.h" // only needed for Config... remove once config is moved.
#include "compat.h"
#include "mythdirs.h"
#include "mythlogging.h"
#include "htmlserver.h"
#include "mythversion.h"
#include <mythcorecontext.h>

#include "serviceHosts/rttiServiceHost.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// HttpServer Class Implementation
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QMutex   HttpServer::s_platformLock;
QString  HttpServer::s_platform;

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

HttpServer::HttpServer(const QString &sApplicationPrefix) :
    ServerPool(), m_sSharePath(GetShareDir()),
    m_pHtmlServer(new HtmlServerExtension(m_sSharePath, sApplicationPrefix)),
    m_threadPool("HttpServerPool"), m_running(true)
{
    // Number of connections processed concurrently
    int maxHttpWorkers = max(QThread::idealThreadCount() * 2, 2); // idealThreadCount can return -1
    // Don't allow more connections than we can process, it causes browsers
    // to open lots of new connections instead of reusing existing ones
    setMaxPendingConnections(maxHttpWorkers);
    m_threadPool.setMaxThreadCount(maxHttpWorkers);

    LOG(VB_UPNP, LOG_NOTICE, QString("HttpServer(): Max Thread Count %1")
                                .arg(m_threadPool.maxThreadCount()));

    // ----------------------------------------------------------------------
    // Build Platform String
    // ----------------------------------------------------------------------
    {
        QMutexLocker locker(&s_platformLock);
#ifdef _WIN32
        s_platform = QString("Windows/%1.%2")
            .arg(LOBYTE(LOWORD(GetVersion())))
            .arg(HIBYTE(LOWORD(GetVersion())));
#else
        struct utsname uname_info;
        uname( &uname_info );
        s_platform = QString("%1/%2")
            .arg(uname_info.sysname).arg(uname_info.release);
#endif
    }

    LOG(VB_UPNP, LOG_INFO, QString("HttpServer() - SharePath = %1")
            .arg(m_sSharePath));

    // -=>TODO: Load Config XML
    // -=>TODO: Load & initialize - HttpServerExtensions

    // ----------------------------------------------------------------------
    // Enable Rtti Service for all instances of HttpServer
    // and register with QtScript Engine.
    // Rtti service is an alternative to using the xsd uri
    // it returns xml/json/etc... definitions of types/enums 
    // ----------------------------------------------------------------------

    RegisterExtension( new RttiServiceHost( m_sSharePath ));

    QScriptEngine *pEngine = ScriptEngine();

    pEngine->globalObject().setProperty("Rtti",
         pEngine->scriptValueFromQMetaObject< ScriptableRtti >() );

#ifndef QT_NO_OPENSSL
    m_sslConfig = QSslConfiguration::defaultConfiguration();

    QString hostKeyPath = gCoreContext->GetSetting("hostSSLKey", "");
    QFile hostKeyFile(hostKeyPath);
    if (!hostKeyFile.exists() || !hostKeyFile.open(QIODevice::ReadOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("HttpServer: Host key file (%1) does not exist").arg(hostKeyPath));
    } else {
        QByteArray rawHostKey = hostKeyFile.readAll();
        m_sslHostKey = QSslKey(rawHostKey, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
        if (m_sslHostKey.isNull())
        {
            LOG(VB_GENERAL, LOG_ERR, QString("HttpServer: Unable to load host key file (%1)").arg(hostKeyPath));
        } else {
            QString hostCertPath = gCoreContext->GetSetting("hostSSLCertificate", "");
            QList<QSslCertificate> certList = QSslCertificate::fromPath(hostCertPath);
            if (!certList.isEmpty())
                m_sslHostCert = certList.first();

            if (!m_sslHostCert.isValid())
                LOG(VB_GENERAL, LOG_ERR, QString("HttpServer: Unable to load host cert file (%1)").arg(hostCertPath));
            
            QString caCertPath = gCoreContext->GetSetting("caSSLCertificate", "");
            m_sslCACertList = QSslCertificate::fromPath(caCertPath);

            if (m_sslCACertList.isEmpty())
                LOG(VB_GENERAL, LOG_ERR, QString("HttpServer: Unable to load CA cert file (%1)").arg(hostCertPath));
        }
    }
#endif
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

HttpServer::~HttpServer()
{
    m_rwlock.lockForWrite();
    m_running = false;
    m_rwlock.unlock();

    m_threadPool.Stop();

    while (!m_extensions.empty())
    {
        delete m_extensions.takeFirst();
    }

    if (m_pHtmlServer != NULL)
        delete m_pHtmlServer;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

QString HttpServer::GetPlatform(void)
{
    QMutexLocker locker(&s_platformLock);
    return s_platform;
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

QString HttpServer::GetServerVersion(void)
{
    QString mythVersion = MYTH_SOURCE_VERSION;
    mythVersion = mythVersion.right(mythVersion.length() - 1); // Trim off the leading 'v'
    return QString("MythTV/%2 %1, UPnP/1.0").arg(HttpServer::GetPlatform())
                                             .arg(mythVersion);
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

QScriptEngine* HttpServer::ScriptEngine()
{
    return ((HtmlServerExtension *)m_pHtmlServer)->ScriptEngine();
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void HttpServer::newTcpConnection(qt_socket_fd_t socket)
{
    PoolServerType type = kTCPServer;
    PrivTcpServer *server = dynamic_cast<PrivTcpServer *>(QObject::sender());
    if (server)
        type = server->GetServerType();

    m_threadPool.startReserved(
        new HttpWorker(*this, socket, type,
                       m_sslConfig, m_sslHostKey, m_sslHostCert, m_sslCACertList),
        QString("HttpServer%1").arg(socket));
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void HttpServer::RegisterExtension( HttpServerExtension *pExtension )
{
    if (pExtension != NULL )
    {
        m_rwlock.lockForWrite();
        m_extensions.append( pExtension );

        // Add to multimap for quick lookup.

        QStringList list = pExtension->GetBasePaths();

        for( int nIdx = 0; nIdx < list.size(); nIdx++)
            m_basePaths.insert( list[ nIdx ], pExtension );

        m_rwlock.unlock();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void HttpServer::UnregisterExtension( HttpServerExtension *pExtension )
{
    if (pExtension != NULL )
    {
        m_rwlock.lockForWrite();

        QStringList list = pExtension->GetBasePaths();

        for( int nIdx = 0; nIdx < list.size(); nIdx++)
            m_basePaths.remove( list[ nIdx ], pExtension );

        m_extensions.removeAll(pExtension);

        delete pExtension;

        m_rwlock.unlock();
    }
}

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void HttpServer::DelegateRequest(HTTPRequest *pRequest)
{
    bool bProcessed = false;

    LOG(VB_UPNP, LOG_DEBUG, QString("m_sBaseUrl: %1").arg( pRequest->m_sBaseUrl ));
    m_rwlock.lockForRead();

    QList< HttpServerExtension* > list = m_basePaths.values( pRequest->m_sBaseUrl );

    for (int nIdx=0; nIdx < list.size() && !bProcessed; nIdx++ )
    {
        try
        {
            bProcessed = list[ nIdx ]->ProcessRequest(pRequest);
        }
        catch(...)
        {
            LOG(VB_GENERAL, LOG_ERR, QString("HttpServer::DelegateRequest - "
                                             "Unexpected Exception - "
                                             "pExtension->ProcessRequest()."));
        }
    }

#if 0
    HttpServerExtensionList::iterator it = m_extensions.begin();

    for (; (it != m_extensions.end()) && !bProcessed; ++it)
    {
        try
        {
            bProcessed = (*it)->ProcessRequest(pRequest);
        }
        catch(...)
        {
            LOG(VB_GENERAL, LOG_ERR, QString("HttpServer::DelegateRequest - "
                                             "Unexpected Exception - "
                                             "pExtension->ProcessRequest()."));
        }
    }
#endif
    m_rwlock.unlock();

    if (!bProcessed)
        bProcessed = m_pHtmlServer->ProcessRequest(pRequest);

    if (!bProcessed)
    {
        pRequest->m_eResponseType   = ResponseTypeHTML;
        pRequest->m_nResponseStatus = 404; 
    }
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//
// HttpWorkerThread Class Implementation
//
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

HttpWorker::HttpWorker(HttpServer &httpServer, qt_socket_fd_t sock,
                       PoolServerType type, QSslConfiguration sslConfig,
                       QSslKey hostKey, QSslCertificate hostCert,
                       QList<QSslCertificate> caCerts) :
    m_httpServer(httpServer), m_socket(sock), m_socketTimeout(10000),
    m_connectionType(type), m_sslConfig(sslConfig), m_sslHostKey(hostKey),
    m_sslHostCert(hostCert), m_sslCACerts(caCerts)
{
    LOG(VB_UPNP, LOG_DEBUG, QString("HttpWorker(%1): New connection")
                                        .arg(m_socket));
    m_socketTimeout = 1000 *
        UPnp::GetConfiguration()->GetValue("HTTP/KeepAliveTimeoutSecs", 10);
}                  

/////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////

void HttpWorker::run(void)
{
#if 0
    LOG(VB_UPNP, LOG_DEBUG,
        QString("HttpWorker::run() socket=%1 -- begin").arg(m_socket));
#endif

    bool                    bTimeout   = false;
    bool                    bKeepAlive = true;
    HTTPRequest            *pRequest   = NULL;
    QTcpSocket             *pSocket;

    if (m_connectionType == kSSLServer)
    {

#ifndef QT_NO_OPENSSL
        QSslSocket *pSslSocket = new QSslSocket();
        if (pSslSocket->setSocketDescriptor(m_socket))
        {
            pSslSocket->setSslConfiguration(m_sslConfig);
            pSslSocket->setPrivateKey(m_sslHostKey);
            pSslSocket->setLocalCertificate(m_sslHostCert);
            pSslSocket->addCaCertificates(m_sslCACerts);
            pSslSocket->startServerEncryption();
            if (pSslSocket->waitForEncrypted(5000))
            {
                LOG(VB_UPNP, LOG_DEBUG, "SSL Handshake occurred, connection encrypted");
            }
            else
            {
                LOG(VB_UPNP, LOG_DEBUG, "SSL Handshake FAILED, connection terminated");
                delete pSslSocket;
                pSslSocket = NULL;
            }
        }
        else
        {
            delete pSslSocket;
            pSslSocket = NULL;
        }

        if (pSslSocket)
            pSocket = dynamic_cast<QTcpSocket *>(pSslSocket);
        else
            return;
#else
        return;
#endif
    }
    else // Plain old unencrypted socket
    {
        pSocket = new QTcpSocket();
        pSocket->setSocketDescriptor(m_socket);
    }

    int nRequestsHandled = 0; // Allow debugging of keep-alive and connection re-use

    try
    {
        while (m_httpServer.IsRunning() && bKeepAlive && pSocket &&
               pSocket->isValid() &&
               pSocket->state() != QAbstractSocket::ClosingState)
        {
            // We set a timeout on keep-alive connections to avoid blocking
            // new clients from connecting - Default at time of writing was
            // 10 seconds
            bTimeout = !(pSocket->waitForReadyRead(m_socketTimeout));

            if (bTimeout) // Either client closed the socket or we timed out waiting for new data
                break;

            int64_t nBytes = pSocket->bytesAvailable();
            if (!m_httpServer.IsRunning())
                break;

            if ( nBytes > 0)
            {
                // ----------------------------------------------------------
                // See if this is a valid request
                // ----------------------------------------------------------

                pRequest = new BufferedSocketDeviceRequest( pSocket );
                if (pRequest != NULL)
                {
                    if ( pRequest->ParseRequest() )
                    {
                        bKeepAlive = pRequest->GetKeepAlive();

                        // ------------------------------------------------------
                        // Request Parsed... Pass on to Main HttpServer class to 
                        // delegate processing to HttpServerExtensions.
                        // ------------------------------------------------------
                        if ((pRequest->m_nResponseStatus != 400) &&
                            (pRequest->m_nResponseStatus != 401))
                            m_httpServer.DelegateRequest(pRequest);

                        nRequestsHandled++;
                    }
                    else
                    {
                        LOG(VB_UPNP, LOG_ERR, "ParseRequest Failed.");

                        pRequest->m_nResponseStatus = 501;
                        bKeepAlive = false;
                    }

                    // -------------------------------------------------------
                    // Always MUST send a response.
                    // -------------------------------------------------------
                    if (pRequest->SendResponse() < 0)
                    {
                        bKeepAlive = false;
                        LOG(VB_UPNP, LOG_ERR,
                            QString("socket(%1) - Error returned from "
                                    "SendResponse... Closing connection")
                                .arg(pSocket->socketDescriptor()));
                    }

                    // -------------------------------------------------------
                    // Check to see if a PostProcess was registered
                    // -------------------------------------------------------
                    if ( pRequest->m_pPostProcess != NULL )
                        pRequest->m_pPostProcess->ExecutePostProcess();

                    delete pRequest;
                    pRequest = NULL;
                }
                else
                {
                    LOG(VB_GENERAL, LOG_ERR,
                        "Error Creating BufferedSocketDeviceRequest");
                    bKeepAlive = false;
                }
            }
            else
            {
                bKeepAlive = false;
            }
        }
    }
    catch(...)
    {
        LOG(VB_GENERAL, LOG_ERR, 
            "HttpWorkerThread::ProcessWork - Unexpected Exception.");
    }

    delete pRequest;

    LOG(VB_UPNP, LOG_DEBUG, QString("HttpWorker(%1): Connection %1 closed, requests handled %2")
                                        .arg(pSocket->socketDescriptor())
                                        .arg(nRequestsHandled));
    pSocket->close();
    delete pSocket;
    pSocket = NULL;

#if 0
    LOG(VB_UPNP, LOG_DEBUG, "HttpWorkerThread::run() -- end");
#endif
}


