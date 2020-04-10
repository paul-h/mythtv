// Qt
#include <QDir>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QPainter>

// MythTV
#include "config.h"
#include "mythcdrom.h"
#include "mythmainwindow.h"
#include "mythevent.h"
#include "iso639.h"
#include "bdiowrapper.h"
#include "bdringbuffer.h"
#include "mythlogging.h"
#include "mythcorecontext.h"
#include "mythlocale.h"
#include "mythdirs.h"
#include "libbluray/bluray.h"
#include "mythiowrapper.h"
#include "mythuiactions.h"
#include "tv_actions.h"

// Std
#include <fcntl.h>

// BluRay
#if CONFIG_LIBBLURAY_EXTERNAL
#include "libbluray/log_control.h"
#include "libbluray/meta_data.h"
#include "libbluray/overlay.h"
#else
#include "util/log_control.h"
#include "libbluray/bdnav/meta_data.h"
#include "libbluray/decoders/overlay.h"
#endif
#include "libbluray/keys.h"

#define LOC QString("BDRingBuf: ")

BDOverlay::BDOverlay(const bd_overlay_s* const Overlay)
  : m_image(Overlay->w, Overlay->h, QImage::Format_Indexed8),
    m_x(Overlay->x),
    m_y(Overlay->y)
{
    Wipe();
}

BDOverlay::BDOverlay(const bd_argb_overlay_s* const Overlay)
  : m_image(Overlay->w, Overlay->h, QImage::Format_ARGB32),
    m_x(Overlay->x),
    m_y(Overlay->y)
{
}

void BDOverlay::SetPalette(const BD_PG_PALETTE_ENTRY *Palette)
{
    if (!Palette)
        return;

    QVector<QRgb> rgbpalette;
    for (int i = 0; i < 256; i++)
    {
        int y  = Palette[i].Y;
        int cr = Palette[i].Cr;
        int cb = Palette[i].Cb;
        int a  = Palette[i].T;
        int r  = qBound(0, int(y + 1.4022 * (cr - 128)), 0xff);
        int b  = qBound(0, int(y + 1.7710 * (cb - 128)), 0xff);
        int g  = qBound(0, int(1.7047 * y - (0.1952 * b) - (0.5647 * r)), 0xff);
        rgbpalette.push_back(static_cast<uint>((a << 24) | (r << 16) | (g << 8) | b));
    }
    m_image.setColorTable(rgbpalette);
}

void BDOverlay::Wipe(void)
{
    Wipe(0, 0, m_image.width(), m_image.height());
}

void BDOverlay::Wipe(int Left, int Top, int Width, int Height)
{
    if (m_image.format() == QImage::Format_Indexed8)
    {
        uint8_t *data = m_image.bits();
        int32_t offset = (Top * m_image.bytesPerLine()) + Left;
        for (int i = 0; i < Height; i++ )
        {
            memset(&data[offset], 0xff, static_cast<size_t>(Width));
            offset += m_image.bytesPerLine();
        }
    }
    else
    {
        QColor   transparent(0, 0, 0, 255);
        QPainter painter(&m_image);
        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.fillRect(Left, Top, Width, Height, transparent);
    }
}

static void HandleOverlayCallback(void *Data, const bd_overlay_s *const Overlay)
{
    auto *bdrb = static_cast<BDRingBuffer*>(Data);
    if (bdrb)
        bdrb->SubmitOverlay(Overlay);
}

static void HandleARGBOverlayCallback(void *Data, const bd_argb_overlay_s *const Overlay)
{
    auto *bdrb = static_cast<BDRingBuffer*>(Data);
    if (bdrb)
        bdrb->SubmitARGBOverlay(Overlay);
}

static void FileOpenedCallback(void* Data)
{
    auto *obj = static_cast<BDRingBuffer*>(Data);
    if (obj)
        obj->ProgressUpdate();
}

static void BDLogger(const char* Message)
{
    LOG(VB_PLAYBACK, LOG_DEBUG, LOC + QString(Message).trimmed());
}

static int BDRead(void *Handle, void *Buf, int LBA, int NumBlocks)
{
    if (mythfile_seek(*(static_cast<int*>(Handle)), LBA * 2048LL, SEEK_SET) != -1)
        return static_cast<int>(mythfile_read(*(static_cast<int*>(Handle)), Buf,
                                              static_cast<size_t>(NumBlocks * 2048)) / 2048);
    return -1;
}

BDInfo::BDInfo(const QString &Filename)
{
    LOG(VB_PLAYBACK, LOG_INFO, QString("BDInfo: Trying %1").arg(Filename));
    QString name = Filename;

    if (name.startsWith("bd:"))
    {
        name.remove(0,3);
        while (name.startsWith("//"))
            name.remove(0,1);
    }

    // clean path filename
    name = QDir(QDir::cleanPath(name)).canonicalPath();
    if (name.isEmpty())
    {
        LOG(VB_GENERAL, LOG_ERR, QString("BDInfo:%1 nonexistent").arg(name));
        name = Filename;
    }

    LOG(VB_GENERAL, LOG_INFO, QString("BDInfo: Opened BDRingBuffer device at %1").arg(name));

    // Make sure log messages from the Bluray library appear in our logs
    bd_set_debug_handler(BDLogger);
    bd_set_debug_mask(DBG_CRIT | DBG_NAV | DBG_BLURAY);

    // Use our own wrappers for file and directory access
    redirectBDIO();

    int bdhandle = -1;
    BLURAY* bdnav = nullptr;
    if (Filename.startsWith("myth:") && MythCDROM::inspectImage(Filename) != MythCDROM::kUnknown)
    {
        // Use streaming for remote images.
        // Streaming encrypted images causes a SIGSEGV in aacs code when
        // using the makemkv libraries due to the missing "device" name.
        // Since a local device (which is likely to be encrypted) can be
        // opened directly, only use streaming for remote images, which
        // presumably won't be encrypted.
        bdhandle = mythfile_open(Filename.toLocal8Bit().data(), O_RDONLY);
        if (bdhandle >= 0)
        {
            bdnav = bd_init();
            if (bdnav)
                bd_open_stream(bdnav, &bdhandle, BDRead);
        }
    }
    else
    {
        QByteArray keyfile = QString("%1/KEYDB.cfg").arg(GetConfDir()).toLatin1();
        bdnav = bd_open(name.toLocal8Bit().data(), keyfile.constData());
    }

    if (!bdnav)
    {
        m_lastError = tr("Could not open Blu-ray device: %1").arg(name);
        LOG(VB_GENERAL, LOG_ERR, QString("BDInfo: ") + m_lastError);
        m_isValid = false;
    }
    else
    {
        GetNameAndSerialNum(bdnav, m_name, m_serialnumber, name, QString("BDInfo: "));
        bd_close(bdnav);
    }

    if (bdhandle >= 0)
        mythfile_close(bdhandle);

    LOG(VB_PLAYBACK, LOG_INFO, QString("BDInfo: Done"));
}

void BDInfo::GetNameAndSerialNum(BLURAY* BluRay, QString &Name,
                                 QString &SerialNum, const QString &Filename,
                                 const QString &LogPrefix)
{
    const meta_dl *metaDiscLibrary = bd_get_meta(BluRay);

    if (metaDiscLibrary)
    {
        Name = QString(metaDiscLibrary->di_name);
    }
    else
    {
        // Use the directory name for the Bluray name
        QDir dir(Filename);
        Name = dir.dirName();
        LOG(VB_PLAYBACK, LOG_DEBUG, LogPrefix + QString("Generated bd name '%1'")
            .arg(Name));
    }

    SerialNum.clear();

    // Try to find the first clip info file and
    // use its SHA1 hash as a serial number.
    for (uint32_t idx = 0; idx < 200; idx++)
    {
        QString clip = QString("BDMV/CLIPINF/%1.clpi").arg(idx, 5, 10, QChar('0'));
        void*   buffer     = nullptr;
        int64_t buffersize = 0;
        if (bd_read_file(BluRay, clip.toLocal8Bit().data(), &buffer, &buffersize) != 0)
        {
            QCryptographicHash crypto(QCryptographicHash::Sha1);
            // Add the clip number to the hash
            crypto.addData(reinterpret_cast<const char*>(&idx), sizeof(idx));
            // then the length of the file
            crypto.addData(reinterpret_cast<const char*>(&buffersize), sizeof(buffersize));
            // and then the contents
            crypto.addData(reinterpret_cast<const char*>(buffer), static_cast<int>(buffersize));
            SerialNum = QString("%1__gen").arg(QString(crypto.result().toBase64()));
            free(buffer);
            LOG(VB_PLAYBACK, LOG_DEBUG, LogPrefix + QString("Generated serial number '%1'")
                .arg(SerialNum));
            break;
        }
    }

    if (SerialNum.isEmpty())
        LOG(VB_GENERAL, LOG_ERR, LogPrefix + "Unable to generate serial number");
}

bool BDInfo::IsValid(void) const
{
    return m_isValid;
}

bool BDInfo::GetNameAndSerialNum(QString &Name, QString &SerialNum)
{
    Name      = m_name;
    SerialNum = m_serialnumber;
    return !(Name.isEmpty() && SerialNum.isEmpty());
}

QString BDInfo::GetLastError(void) const
{
    return m_lastError;
}

BDRingBuffer::BDRingBuffer(const QString &Filename)
  : RingBuffer(kRingBuffer_BD),
    m_overlayPlanes(2, nullptr)
{
    m_tryHDMVNavigation = nullptr != getenv("MYTHTV_HDMV");
    m_mainThread = QThread::currentThread();
    OpenFile(Filename);
}

BDRingBuffer::~BDRingBuffer()
{
    KillReadAheadThread();
    Close();
}

void BDRingBuffer::Close(void)
{
    if (m_bdnav)
    {
        m_infoLock.lock();
        for (auto it = m_cachedTitleInfo.begin(); it !=m_cachedTitleInfo.end(); ++it)
            bd_free_title_info(it.value());
        m_cachedTitleInfo.clear();
        for (auto it = m_cachedPlaylistInfo.begin(); it !=m_cachedPlaylistInfo.end(); ++it)
            bd_free_title_info(it.value());
        m_cachedPlaylistInfo.clear();
        m_infoLock.unlock();
        bd_close(m_bdnav);
        m_bdnav = nullptr;
    }

    if (m_imgHandle > 0)
    {
        mythfile_close(m_imgHandle);
        m_imgHandle = -1;
    }

    ClearOverlays();
}

long long BDRingBuffer::SeekInternal(long long Position, int Whence)
{
    long long ret = -1;

    m_posLock.lockForWrite();

    // Optimize no-op seeks
    if (m_readAheadRunning &&
        ((Whence == SEEK_SET && Position == m_readPos) || (Whence == SEEK_CUR && Position == 0)))
    {
        ret = m_readPos;
        m_posLock.unlock();
        return ret;
    }

    // only valid for SEEK_SET & SEEK_CUR
    long long newposition = (SEEK_SET == Whence) ? Position : m_readPos + Position;

    // Here we perform a normal seek. When successful we
    // need to call ResetReadAhead(). A reset means we will
    // need to refill the buffer, which takes some time.
    if ((SEEK_END == Whence) || ((SEEK_CUR == Whence) && newposition != 0))
    {
        errno = EINVAL;
        ret = -1;
    }
    else
    {
        SeekInternal(static_cast<uint64_t>(newposition));
        m_currentTime = bd_tell_time(m_bdnav);
        ret = newposition;
    }

    if (ret >= 0)
    {
        m_readPos = ret;
        m_ignoreReadPos = -1;
        if (m_readAheadRunning)
            ResetReadAhead(m_readPos);
        m_readAdjust = 0;
    }
    else
    {
        QString cmd = QString("Seek(%1, %2)").arg(Position)
            .arg((Whence == SEEK_SET) ? "SEEK_SET" :
                 ((Whence == SEEK_CUR) ?"SEEK_CUR" : "SEEK_END"));
        LOG(VB_GENERAL, LOG_ERR, LOC + cmd + " Failed" + ENO);
    }

    m_posLock.unlock();
    m_generalWait.wakeAll();
    return ret;
}

uint64_t BDRingBuffer::SeekInternal(uint64_t Position)
{
    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Seeking to '%1'").arg(Position));
    m_processState = PROCESS_NORMAL;
    if (m_bdnav)
        return static_cast<uint64_t>(bd_seek_time(m_bdnav, Position));
    return 0;
}

void BDRingBuffer::GetDescForPos(QString &Desc)
{
    if (!m_infoLock.tryLock())
        return;
    Desc = tr("Title %1 chapter %2").arg(m_currentTitle).arg(m_currentTitleInfo->chapters->idx);
    m_infoLock.unlock();
}

bool BDRingBuffer::HandleAction(const QStringList &Actions, int64_t Pts)
{
    if (!m_isHDMVNavigation)
        return false;

    if (Actions.contains(ACTION_MENUTEXT))
    {
        PressButton(BD_VK_POPUP, Pts);
        return true;
    }

    if (!IsInMenu())
        return false;

    bool handled = true;
    if (Actions.contains(ACTION_UP) || Actions.contains(ACTION_CHANNELUP))
        PressButton(BD_VK_UP, Pts);
    else if (Actions.contains(ACTION_DOWN) || Actions.contains(ACTION_CHANNELDOWN))
        PressButton(BD_VK_DOWN, Pts);
    else if (Actions.contains(ACTION_LEFT) || Actions.contains(ACTION_SEEKRWND))
        PressButton(BD_VK_LEFT, Pts);
    else if (Actions.contains(ACTION_RIGHT) || Actions.contains(ACTION_SEEKFFWD))
        PressButton(BD_VK_RIGHT, Pts);
    else if (Actions.contains(ACTION_0))
        PressButton(BD_VK_0, Pts);
    else if (Actions.contains(ACTION_1))
        PressButton(BD_VK_1, Pts);
    else if (Actions.contains(ACTION_2))
        PressButton(BD_VK_2, Pts);
    else if (Actions.contains(ACTION_3))
        PressButton(BD_VK_3, Pts);
    else if (Actions.contains(ACTION_4))
        PressButton(BD_VK_4, Pts);
    else if (Actions.contains(ACTION_5))
        PressButton(BD_VK_5, Pts);
    else if (Actions.contains(ACTION_6))
        PressButton(BD_VK_6, Pts);
    else if (Actions.contains(ACTION_7))
        PressButton(BD_VK_7, Pts);
    else if (Actions.contains(ACTION_8))
        PressButton(BD_VK_8, Pts);
    else if (Actions.contains(ACTION_9))
        PressButton(BD_VK_9, Pts);
    else if (Actions.contains(ACTION_SELECT))
        PressButton(BD_VK_ENTER, Pts);
    else
        handled = false;

    return handled;
}

void BDRingBuffer::ProgressUpdate(void)
{
    // This thread check is probably unnecessary as processEvents should
    // only handle events in the calling thread - and not all threads
    if (!is_current_thread(m_mainThread))
        return;

    qApp->postEvent(GetMythMainWindow(),
                    new MythEvent(MythEvent::kUpdateTvProgressEventType));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool BDRingBuffer::BDWaitingForPlayer(void)
{
    return m_playerWait;
}

void BDRingBuffer::SkipBDWaitingForPlayer(void)
{
    m_playerWait = false;
}

/** \fn BDRingBuffer::OpenFile(const QString &, uint)
 *  \brief Opens a bluray device for reading.
 *
 *  \param lfilename   Path of the bluray device to read.
 *  \param retry_ms    Ignored. This value is part of the API
 *                     inherited from the parent class.
 *  \return Returns true if the bluray was opened.
 */
bool BDRingBuffer::OpenFile(const QString &Filename, uint /*Retry*/)
{
    m_safeFilename = Filename;
    m_filename = Filename;

    // clean path filename
    QString filename = QDir(QDir::cleanPath(Filename)).canonicalPath();
    if (filename.isEmpty())
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + QString("%1 nonexistent").arg(Filename));
        filename = Filename;
    }
    m_safeFilename = filename;

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Opened BDRingBuffer device at %1")
            .arg(filename));

    // Make sure log messages from the Bluray library appear in our logs
    bd_set_debug_handler(BDLogger);
    bd_set_debug_mask(DBG_CRIT | DBG_NAV | DBG_BLURAY);

    // Use our own wrappers for file and directory access
    redirectBDIO();

    // Ask mythiowrapper to update this object on file open progress. Opening
    // a bluray disc can involve opening several hundred files which can take
    // several minutes when the disc structure is remote. The callback allows
    // us to 'kick' the main UI - as the 'please wait' widget is still visible
    // at this stage
    mythfile_open_register_callback(filename.toLocal8Bit().data(), this, FileOpenedCallback);

    QMutexLocker locker(&m_infoLock);
    m_rwLock.lockForWrite();

    if (m_bdnav)
        Close();

    QString keyfile = QString("%1/KEYDB.cfg").arg(GetConfDir());
    const char *keyfilepath = keyfile.toLatin1().data();

    if (filename.startsWith("myth:") && MythCDROM::inspectImage(filename) != MythCDROM::kUnknown)
    {
        // Use streaming for remote images.
        // Streaming encrypted images causes a SIGSEGV in aacs code when
        // using the makemkv libraries due to the missing "device" name.
        // Since a local device (which is likely to be encrypted) can be
        // opened directly, only use streaming for remote images, which
        // presumably won't be encrypted.
        m_imgHandle = mythfile_open(filename.toLocal8Bit().data(), O_RDONLY);
        if (m_imgHandle >= 0)
        {
            m_bdnav = bd_init();
            if (m_bdnav)
                bd_open_stream(m_bdnav, &m_imgHandle, BDRead);
        }
    }
    else
    {
        m_bdnav = bd_open(filename.toLocal8Bit().data(), keyfilepath);
    }

    if (!m_bdnav)
    {
        m_lastError = tr("Could not open Blu-ray device: %1").arg(filename);
        m_rwLock.unlock();
        mythfile_open_register_callback(filename.toLocal8Bit().data(), this, nullptr);
        return false;
    }

    const meta_dl *metaDiscLibrary = bd_get_meta(m_bdnav);

    if (metaDiscLibrary)
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Disc Title: %1 (%2)")
            .arg(metaDiscLibrary->di_name).arg(metaDiscLibrary->language_code));
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Alternative Title: %1")
            .arg(metaDiscLibrary->di_alternative));
        LOG(VB_GENERAL, LOG_INFO, LOC + QString("Disc Number: %1 of %2")
            .arg(metaDiscLibrary->di_set_number).arg(metaDiscLibrary->di_num_sets));
    }

    BDInfo::GetNameAndSerialNum(m_bdnav, m_name, m_serialNumber, m_safeFilename, LOC);

    // Check disc to see encryption status, menu and navigation types.
    m_topMenuSupported   = false;
    m_firstPlaySupported = false;
    const BLURAY_DISC_INFO *discinfo = bd_get_disc_info(m_bdnav);
    if (!discinfo || (discinfo->aacs_detected && !discinfo->aacs_handled) ||
        (discinfo->bdplus_detected && !discinfo->bdplus_handled))
    {
        // couldn't decrypt bluray
        bd_close(m_bdnav);
        m_bdnav = nullptr;
        m_lastError = tr("Could not open Blu-ray device %1, failed to decrypt").arg(filename);
        m_rwLock.unlock();
        mythfile_open_register_callback(filename.toLocal8Bit().data(), this, nullptr);
        return false;
    }

    // The following settings affect HDMV navigation
    // (default audio track selection,
    // parental controls, menu language, etc.  They are not yet used.

    // Set parental level "age" to 99 for now.  TODO: Add support for FE level
    bd_set_player_setting(m_bdnav, BLURAY_PLAYER_SETTING_PARENTAL, 99);

    // Set preferred language to FE guide language
    const char *langpref = gCoreContext->GetSetting("ISO639Language0", "eng").toLatin1().data();
    QString QScountry    = gCoreContext->GetLocale()->GetCountryCode().toLower();
    const char *country  = QScountry.toLatin1().data();
    bd_set_player_setting_str(m_bdnav, BLURAY_PLAYER_SETTING_AUDIO_LANG, langpref);
    // Set preferred presentation graphics language to the FE guide language
    bd_set_player_setting_str(m_bdnav, BLURAY_PLAYER_SETTING_PG_LANG, langpref);
    // Set preferred menu language to the FE guide language
    bd_set_player_setting_str(m_bdnav, BLURAY_PLAYER_SETTING_MENU_LANG, langpref);
    // Set player country code via MythLocale. (not a region setting)
    bd_set_player_setting_str(m_bdnav, BLURAY_PLAYER_SETTING_COUNTRY_CODE, country);

    uint32_t regioncode = static_cast<uint32_t>(gCoreContext->GetNumSetting("BlurayRegionCode"));
    if (regioncode > 0)
        bd_set_player_setting(m_bdnav, BLURAY_PLAYER_SETTING_REGION_CODE, regioncode);

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Using %1 as keyfile...").arg(QString(keyfilepath)));

    // Return an index of relevant titles (excludes dupe clips + titles)
    LOG(VB_GENERAL, LOG_INFO, LOC + "Retrieving title list (please wait).");
    m_numTitles = bd_get_titles(m_bdnav, TITLES_RELEVANT, 30);
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Found %1 titles.").arg(m_numTitles));
    if (!m_numTitles)
    {
        // no title, no point trying any longer
        bd_close(m_bdnav);
        m_bdnav = nullptr;
        m_lastError = tr("Unable to find any Blu-ray compatible titles");
        m_rwLock.unlock();
        mythfile_open_register_callback(filename.toLocal8Bit().data(), this, nullptr);
        return false;
    }

    if (discinfo)
    {
        m_topMenuSupported   = (discinfo->top_menu_supported != 0U);
        m_firstPlaySupported = (discinfo->first_play_supported != 0U);

        LOG(VB_PLAYBACK, LOG_INFO, LOC + "*** Blu-ray Disc Information ***");
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("First Play Supported: %1")
            .arg(discinfo->first_play_supported ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Top Menu Supported: %1")
            .arg(discinfo->top_menu_supported ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Number of HDMV Titles: %1")
            .arg(discinfo->num_hdmv_titles));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Number of BD-J Titles: %1")
            .arg(discinfo->num_bdj_titles));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Number of Unsupported Titles: %1")
            .arg(discinfo->num_unsupported_titles));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("AACS present on disc: %1")
            .arg(discinfo->aacs_detected ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("libaacs used: %1")
            .arg(discinfo->libaacs_detected ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("AACS handled: %1")
            .arg(discinfo->aacs_handled ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("BD+ present on disc: %1")
            .arg(discinfo->bdplus_detected ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("libbdplus used: %1")
            .arg(discinfo->libbdplus_detected ? "yes" : "no"));
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("BD+ handled: %1")
            .arg(discinfo->bdplus_handled ? "yes" : "no"));
    }

    m_mainTitle = 0;
    m_currentTitleLength = 0;
    m_titlesize = 0;
    m_currentTime = 0;
    m_currentTitleInfo = nullptr;
    m_currentTitleAngleCount = 0;
    m_processState = PROCESS_NORMAL;
    m_lastEvent.event = BD_EVENT_NONE;
    m_lastEvent.param = 0;

    // Mostly event-driven values below
    m_currentAngle = 0;
    m_currentTitle = -1;
    m_currentPlaylist = 0;
    m_currentPlayitem = 0;
    m_currentChapter = 0;
    m_currentAudioStream = 0;
    m_currentIGStream = 0;
    m_currentPGTextSTStream = 0;
    m_currentSecondaryAudioStream = 0;
    m_currentSecondaryVideoStream = 0;
    m_pgTextSTEnabled = false;
    m_secondaryAudioEnabled = false;
    m_secondaryVideoEnabled = false;
    m_secondaryVideoIsFullscreen = false;
    m_stillMode = BLURAY_STILL_NONE;
    m_stillTime = 0;
    m_timeDiff = 0;
    m_inMenu = false;

    // First, attempt to initialize the disc in HDMV navigation mode.
    // If this fails, fall back to the traditional built-in title switching
    // mode.
    if (m_tryHDMVNavigation && m_firstPlaySupported && bd_play(m_bdnav))
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + "Using HDMV navigation mode.");
        m_isHDMVNavigation = true;

        // Register the Menu Overlay Callback
        bd_register_overlay_proc(m_bdnav, this, HandleOverlayCallback);
        bd_register_argb_overlay_proc(m_bdnav, this, HandleARGBOverlayCallback, nullptr);
    }
    else
    {
        LOG(VB_GENERAL, LOG_INFO, LOC + "Using title navigation mode.");

        // Loop through the relevant titles and find the longest
        uint64_t titleLength = 0;
        BLURAY_TITLE_INFO *titleInfo = nullptr;
        bool found = false;
        for( unsigned i = 0; i < m_numTitles; ++i)
        {
            titleInfo = GetTitleInfo(i);
            if (!titleInfo)
                continue;
            if (titleLength == 0 || (titleInfo->duration > titleLength))
            {
                m_mainTitle = titleInfo->idx;
                titleLength = titleInfo->duration;
                found = true;
            }
        }

        if (!found)
        {
            // no title, no point trying any longer
            bd_close(m_bdnav);
            m_bdnav = nullptr;
            m_lastError = tr("Unable to find any usable Blu-ray titles");
            m_rwLock.unlock();
            mythfile_open_register_callback(filename.toLocal8Bit().data(), this, nullptr);
            return false;
        }
        SwitchTitle(m_mainTitle);
    }

    m_readBlockSize   = BD_BLOCK_SIZE * 62;
    m_setSwitchToNext = false;
    m_ateof           = false;
    m_commsError      = false;
    m_numFailures     = 0;
    m_rawBitrate      = 8000;
    CalcReadAheadThresh();
    m_rwLock.unlock();
    mythfile_open_register_callback(filename.toLocal8Bit().data(), this, nullptr);
    return true;
}

long long BDRingBuffer::GetReadPosition(void) const
{
    if (m_bdnav)
        return static_cast<long long>(bd_tell(m_bdnav));
    return 0;
}

bool BDRingBuffer::IsOpen(void) const
{
    return m_bdnav;
}

bool BDRingBuffer::IsInMenu(void) const
{
    return m_inMenu;
}

uint32_t BDRingBuffer::GetNumChapters(void)
{
    QMutexLocker locker(&m_infoLock);
    if (m_currentTitleInfo)
        return m_currentTitleInfo->chapter_count - 1;
    return 0;
}

uint32_t BDRingBuffer::GetCurrentChapter(void)
{
    if (m_bdnav)
        return bd_get_current_chapter(m_bdnav);
    return 0;
}

uint64_t BDRingBuffer::GetChapterStartTime(uint32_t Chapter)
{
    if (Chapter >= GetNumChapters())
        return 0;
    QMutexLocker locker(&m_infoLock);
    return static_cast<uint64_t>(static_cast<double>(m_currentTitleInfo->chapters[Chapter].start) / 90000.0);
}

uint64_t BDRingBuffer::GetChapterStartFrame(uint32_t Chapter)
{
    if (Chapter >= GetNumChapters())
        return 0;
    QMutexLocker locker(&m_infoLock);
    return static_cast<uint64_t>((m_currentTitleInfo->chapters[Chapter].start * GetFrameRate()) / 90000.0);
}

uint32_t BDRingBuffer::GetNumTitles(void) const
{
    return m_numTitles;
}

int BDRingBuffer::GetCurrentTitle(void)
{
    QMutexLocker locker(&m_infoLock);
    return m_currentTitle;
}

uint64_t BDRingBuffer::GetCurrentAngle(void) const
{
    return static_cast<uint64_t>(m_currentAngle);
}

int BDRingBuffer::GetTitleDuration(int Title)
{
    QMutexLocker locker(&m_infoLock);
    auto numTitles = GetNumTitles();
    if (!(numTitles > 0 && Title >= 0 && Title < static_cast<int>(numTitles)))
        return 0;

    BLURAY_TITLE_INFO *info = GetTitleInfo(static_cast<uint32_t>(Title));
    if (!info)
        return 0;

    return static_cast<int>(static_cast<double>(info->duration) / 90000.0);
}

uint64_t BDRingBuffer::GetTitleSize(void) const
{
    return m_titlesize;
}

uint64_t BDRingBuffer::GetTotalTimeOfTitle(void) const
{
    return m_currentTitleLength / 90000;
}

uint64_t BDRingBuffer::GetCurrentTime(void)
{
    return m_currentTime / 90000;
}

bool BDRingBuffer::SwitchTitle(uint32_t Index)
{
    if (!m_bdnav)
        return false;

    m_infoLock.lock();
    m_currentTitleInfo = GetTitleInfo(Index);
    m_infoLock.unlock();
    bd_select_title(m_bdnav, Index);

    return UpdateTitleInfo();
}

bool BDRingBuffer::SwitchPlaylist(uint32_t Index)
{
    if (!m_bdnav)
        return false;

    LOG(VB_PLAYBACK, LOG_INFO, LOC + "SwitchPlaylist - start");

    m_infoLock.lock();
    m_currentTitleInfo = GetPlaylistInfo(Index);
    m_currentTitle = static_cast<int>(bd_get_current_title(m_bdnav));
    m_infoLock.unlock();
    bool result = UpdateTitleInfo();

    LOG(VB_PLAYBACK, LOG_INFO, LOC + "SwitchPlaylist - end");
    return result;
}

BLURAY_TITLE_INFO* BDRingBuffer::GetTitleInfo(uint32_t Index)
{
    if (!m_bdnav)
        return nullptr;

    QMutexLocker locker(&m_infoLock);
    if (m_cachedTitleInfo.contains(Index))
        return m_cachedTitleInfo.value(Index);

    if (Index > m_numTitles)
        return nullptr;

    BLURAY_TITLE_INFO* result = bd_get_title_info(m_bdnav, Index, 0);
    if (result)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Found title %1 info").arg(Index));
        m_cachedTitleInfo.insert(Index,result);
        return result;
    }
    return nullptr;
}

BLURAY_TITLE_INFO* BDRingBuffer::GetPlaylistInfo(uint32_t Index)
{
    if (!m_bdnav)
        return nullptr;

    QMutexLocker locker(&m_infoLock);
    if (m_cachedPlaylistInfo.contains(Index))
        return m_cachedPlaylistInfo.value(Index);

    BLURAY_TITLE_INFO* result = bd_get_playlist_info(m_bdnav, Index, 0);
    if (result)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Found playlist %1 info").arg(Index));
        m_cachedPlaylistInfo.insert(Index,result);
        return result;
    }
    return nullptr;
}

bool BDRingBuffer::UpdateTitleInfo(void)
{
    QMutexLocker locker(&m_infoLock);
    if (!m_currentTitleInfo)
        return false;

    m_titleChanged = true;
    m_currentTitleLength = m_currentTitleInfo->duration;
    m_currentTitleAngleCount = m_currentTitleInfo->angle_count;
    m_currentAngle = 0;
    m_currentPlayitem = 0;
    m_timeDiff = 0;
    m_titlesize = bd_get_title_size(m_bdnav);
    uint32_t chapter_count = GetNumChapters();
    uint64_t total_secs = m_currentTitleLength / 90000;
    int hours = static_cast<int>(total_secs / 60 / 60);
    int minutes = static_cast<int>((total_secs / 60) - (static_cast<uint64_t>(hours * 60)));
    double secs = static_cast<double>(total_secs) - static_cast<double>(hours * 60 * 60 + minutes * 60);
    QString duration = QString("%1:%2:%3")
            .arg(hours,   2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs,    2, 'f', 1, QChar('0'));
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("New title info: Index %1 Playlist: %2 Duration: %3 ""Chapters: %5")
            .arg(m_currentTitle).arg(m_currentTitleInfo->playlist).arg(duration).arg(chapter_count));
    LOG(VB_GENERAL, LOG_INFO, LOC + QString("New title info: Clips: %1 Angles: %2 Title Size: %3 Frame Rate %4")
            .arg(m_currentTitleInfo->clip_count).arg(m_currentTitleAngleCount).arg(m_titlesize)
            .arg(GetFrameRate()));

    for (uint i = 0; i < chapter_count; i++)
    {
        uint64_t framenum   = GetChapterStartFrame(i);
        total_secs = GetChapterStartTime(i);
        hours   = static_cast<int>(total_secs / 60 / 60);
        minutes = static_cast<int>((total_secs / 60) - (static_cast<uint64_t>(hours * 60)));
        secs    = static_cast<double>(total_secs) - static_cast<double>(hours * 60 * 60 + minutes * 60);
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Chapter %1 found @ [%2:%3:%4]->%5")
            .arg(i + 1,   2, 10, QChar('0')).arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0')).arg(secs,  6, 'f', 3, QChar('0'))
            .arg(framenum));
    }

    int still = BLURAY_STILL_NONE;
    int time  = 0;
    if (m_currentTitleInfo->clip_count)
    {
        for (uint i = 0; i < m_currentTitleInfo->clip_count; i++)
        {
            LOG(VB_PLAYBACK, LOG_INFO, LOC +
                QString("Clip %1 stillmode %2 stilltime %3 videostreams %4 audiostreams %5 igstreams %6")
                    .arg(i).arg(m_currentTitleInfo->clips[i].still_mode)
                    .arg(m_currentTitleInfo->clips[i].still_time)
                    .arg(m_currentTitleInfo->clips[i].video_stream_count)
                    .arg(m_currentTitleInfo->clips[i].audio_stream_count)
                    .arg(m_currentTitleInfo->clips[i].ig_stream_count));
            still |= m_currentTitleInfo->clips[i].still_mode;
            time = m_currentTitleInfo->clips[i].still_time;
        }
    }

    if (m_currentTitleInfo->clip_count > 1 && still != BLURAY_STILL_NONE)
    {
        LOG(VB_GENERAL, LOG_WARNING, LOC + "Warning: more than 1 clip, following still "
                                           "frame analysis may be wrong");
    }

    if (still == BLURAY_STILL_TIME)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Entering still frame (%1 seconds) UNSUPPORTED").arg(time));
        bd_read_skip_still(m_bdnav);
    }
    else if (still == BLURAY_STILL_INFINITE)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Entering infinite still frame.");
    }

    m_stillMode = still;
    m_stillTime = time;
    return true;
}

bool BDRingBuffer::TitleChanged(void)
{
    bool ret = m_titleChanged;
    m_titleChanged = false;
    return ret;
}

bool BDRingBuffer::SwitchAngle(uint Angle)
{
    if (!m_bdnav)
        return false;

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Switching to Angle '%1'").arg(Angle));
    bd_seamless_angle_change(m_bdnav, Angle);
    m_currentAngle = static_cast<int>(Angle);
    return true;
}

uint64_t BDRingBuffer::GetNumAngles(void)
{
    return m_currentTitleAngleCount;
}

uint64_t BDRingBuffer::GetTotalReadPosition(void)
{
    if (m_bdnav)
        return bd_get_title_size(m_bdnav);
    return 0;
}

int64_t BDRingBuffer::AdjustTimestamp(int64_t Timestamp)
{
    int64_t newTimestamp = Timestamp;
    if ((newTimestamp != AV_NOPTS_VALUE) && (newTimestamp >= m_timeDiff))
        newTimestamp -= m_timeDiff;
    return newTimestamp;
}

int BDRingBuffer::SafeRead(void *Buffer, uint Size)
{
    int result = 0;
    if (m_isHDMVNavigation)
    {
        result = HandleBDEvents() ? 0 : -1;
        while (result == 0)
        {
            BD_EVENT event;
            result = bd_read_ext(m_bdnav, static_cast<unsigned char*>(Buffer),
                                 static_cast<int>(Size), &event);
            if (result == 0)
            {
                HandleBDEvent(event);
                result = HandleBDEvents() ? 0 : -1;
            }
        }
    }
    else
    {
        if (m_processState != PROCESS_WAIT)
        {
            processState_t lastState = m_processState;

            if (m_processState == PROCESS_NORMAL)
                result = bd_read(m_bdnav, static_cast<unsigned char*>(Buffer), static_cast<int>(Size));

            HandleBDEvents();

            if (m_processState == PROCESS_WAIT && lastState == PROCESS_NORMAL)
            {
                // We're waiting for the decoder to drain its buffers
                // so don't give it any more data just yet.
                m_pendingData = QByteArray(static_cast<const char*>(Buffer), result);
                result = 0;
            }
            else
            if (m_processState == PROCESS_NORMAL && lastState == PROCESS_REPROCESS)
            {
                // The decoder has finished draining its buffers so give
                // it that last block of data we read
                result = m_pendingData.size();
                memcpy(Buffer, m_pendingData.constData(), static_cast<size_t>(result));
                m_pendingData.clear();
            }
        }
    }

    if (result < 0)
        StopReads();

    m_currentTime = bd_tell_time(m_bdnav);
    return result;
}

double BDRingBuffer::GetFrameRate(void)
{
    QMutexLocker locker(&m_infoLock);
    if (m_bdnav && m_currentTitleInfo)
    {
        switch (m_currentTitleInfo->clips->video_streams->rate)
        {
            case BLURAY_VIDEO_RATE_24000_1001: return 23.97;
            case BLURAY_VIDEO_RATE_24:         return 24.00;
            case BLURAY_VIDEO_RATE_25:         return 25.00;
            case BLURAY_VIDEO_RATE_30000_1001: return 29.97;
            case BLURAY_VIDEO_RATE_50:         return 50.00;
            case BLURAY_VIDEO_RATE_60000_1001: return 59.94;
            default: break;
        }
    }
    return 0;
}

int BDRingBuffer::GetAudioLanguage(uint StreamID)
{
    QMutexLocker locker(&m_infoLock);

    int code = iso639_str3_to_key("und");

    if (m_currentTitleInfo && m_currentTitleInfo->clip_count > 0)
    {
        bd_clip& clip = m_currentTitleInfo->clips[0];
        const BLURAY_STREAM_INFO* stream = FindStream(StreamID, clip.audio_streams, clip.audio_stream_count);
        if (stream)
        {
            const uint8_t* lang = stream->lang;
            code = iso639_key_to_canonical_key((lang[0] << 16) | (lang[1] << 8) | lang[2]);
        }
    }

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Audio Lang: 0x%1 Code: %2")
        .arg(code, 3, 16).arg(iso639_key_to_str3(code)));
    return code;
}

int BDRingBuffer::GetSubtitleLanguage(uint StreamID)
{
    QMutexLocker locker(&m_infoLock);
    int code = iso639_str3_to_key("und");
    if (m_currentTitleInfo && m_currentTitleInfo->clip_count > 0)
    {
        bd_clip& clip = m_currentTitleInfo->clips[0];
        const BLURAY_STREAM_INFO* stream = FindStream(StreamID, clip.pg_streams, clip.pg_stream_count);
        if (stream)
        {
            const uint8_t* lang = stream->lang;
            code = iso639_key_to_canonical_key((lang[0]<<16)|(lang[1]<<8)|lang[2]);
        }
    }

    LOG(VB_GENERAL, LOG_INFO, LOC + QString("Subtitle Lang: 0x%1 Code: %2")
                                  .arg(code, 3, 16).arg(iso639_key_to_str3(code)));
    return code;
}

void BDRingBuffer::PressButton(int32_t Key, int64_t Pts)
{
    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("Key %1 (pts %2)").arg(Key).arg(Pts));
    // HACK for still frame menu navigation
    Pts = 1;
    if (!m_bdnav || Pts <= 0 || Key < 0)
        return;
    bd_user_input(m_bdnav, Pts, static_cast<uint32_t>(Key));
}

void BDRingBuffer::ClickButton(int64_t Pts, uint16_t X, uint16_t Y)
{
    if (!m_bdnav)
        return;
    if (Pts <= 0 || X == 0 || Y == 0)
        return;
    bd_mouse_select(m_bdnav, Pts, X, Y);
}

/** \brief jump to a Blu-ray root or popup menu
 */
bool BDRingBuffer::GoToMenu(const QString &Menu, int64_t Pts)
{
    if (!m_isHDMVNavigation || Pts < 0)
        return false;

    if (!m_topMenuSupported)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Top Menu not supported");
        return false;
    }

    LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("GoToMenu %1").arg(Menu));

    if (Menu.compare("root") == 0)
    {
        if (bd_menu_call(m_bdnav, Pts))
        {
            LOG(VB_PLAYBACK, LOG_INFO, LOC +QString("Invoked Top Menu (pts %1)").arg(Pts));
            return true;
        }
    }
    else if (Menu.compare("popup") == 0)
    {
        PressButton(BD_VK_POPUP, Pts);
        return true;
    }

    return false;
}

bool BDRingBuffer::HandleBDEvents(void)
{
    if (m_processState != PROCESS_WAIT)
    {
        if (m_processState == PROCESS_REPROCESS)
        {
            HandleBDEvent(m_lastEvent);
            // HandleBDEvent will change the process state
            // if it needs to so don't do it here.
        }

        while (m_processState == PROCESS_NORMAL && bd_get_event(m_bdnav, &m_lastEvent))
        {
            HandleBDEvent(m_lastEvent);
            if (m_lastEvent.event == BD_EVENT_NONE || m_lastEvent.event == BD_EVENT_ERROR)
                return false;
        }
    }
    return true;
}

void BDRingBuffer::HandleBDEvent(BD_EVENT &Event)
{
    switch (Event.event) {
        case BD_EVENT_NONE:
            break;
        case BD_EVENT_ERROR:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_ERROR %1").arg(Event.param));
            break;
        case BD_EVENT_ENCRYPTED:
            LOG(VB_GENERAL, LOG_ERR, LOC + "EVENT_ENCRYPTED, playback will fail.");
            break;

        /* current playback position */

        case BD_EVENT_ANGLE:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_ANGLE %1").arg(Event.param));
            m_currentAngle = static_cast<int>(Event.param);
            break;
        case BD_EVENT_TITLE:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_TITLE %1 (old %2)")
                .arg(Event.param).arg(m_currentTitle));
            m_currentTitle = static_cast<int>(Event.param);
            break;
        case BD_EVENT_END_OF_TITLE:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_END_OF_TITLE %1").arg(m_currentTitle));
            WaitForPlayer();
            break;
        case BD_EVENT_PLAYLIST:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_PLAYLIST %1 (old %2)")
                .arg(Event.param).arg(m_currentPlaylist));
            m_currentPlaylist = static_cast<int>(Event.param);
            m_timeDiff = 0;
            m_currentPlayitem = 0;
            SwitchPlaylist(static_cast<uint32_t>(m_currentPlaylist));
            break;
        case BD_EVENT_PLAYITEM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_PLAYITEM %1").arg(Event.param));
            {
                if (m_currentPlayitem != static_cast<int>(Event.param))
                {
                    int64_t out = static_cast<int64_t>(m_currentTitleInfo->clips[m_currentPlayitem].out_time);
                    int64_t in  = static_cast<int64_t>(m_currentTitleInfo->clips[Event.param].in_time);
                    int64_t diff = in - out;
                    if (diff != 0 && m_processState == PROCESS_NORMAL)
                    {
                        LOG(VB_PLAYBACK, LOG_DEBUG, LOC + QString("PTS discontinuity - waiting for decoder: this %1, last %2, diff %3")
                            .arg(in).arg(out).arg(diff));
                        m_processState = PROCESS_WAIT;
                        break;
                    }

                    m_timeDiff += diff;
                    m_processState = PROCESS_NORMAL;
                    m_currentPlayitem = static_cast<int>(Event.param);
                }
            }
            break;
        case BD_EVENT_CHAPTER:
            // N.B. event chapter numbering 1...N, chapter seeks etc 0...
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_CHAPTER %1").arg(Event.param));
            m_currentChapter = static_cast<int>(Event.param);
            break;
        case BD_EVENT_PLAYMARK:
            /* playmark reached */
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_PLAYMARK"));
            break;

        /* playback control */
        case BD_EVENT_PLAYLIST_STOP:
            /* HDMV VM or JVM stopped playlist playback. Flush all buffers. */
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("ToDo EVENT_PLAYLIST_STOP %1")
                .arg(Event.param));
            break;

        case BD_EVENT_STILL:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_STILL %1").arg(Event.param));
            break;
        case BD_EVENT_STILL_TIME:
            // we use the clip information to determine the still frame status
            // sleep a little
            usleep(10000);
            break;
        case BD_EVENT_SEEK:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SEEK"));
            break;

        /* stream selection */

        case BD_EVENT_AUDIO_STREAM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_AUDIO_STREAM %1").arg(Event.param));
            m_currentAudioStream = static_cast<int>(Event.param);
            break;
        case BD_EVENT_IG_STREAM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_IG_STREAM %1").arg(Event.param));
            m_currentIGStream = static_cast<int>(Event.param);
            break;
        case BD_EVENT_PG_TEXTST_STREAM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_PG_TEXTST_STREAM %1").arg(Event.param));
            m_currentPGTextSTStream = static_cast<int>(Event.param);
            break;
        case BD_EVENT_SECONDARY_AUDIO_STREAM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SECONDARY_AUDIO_STREAM %1").arg(Event.param));
            m_currentSecondaryAudioStream = static_cast<int>(Event.param);
            break;
        case BD_EVENT_SECONDARY_VIDEO_STREAM:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SECONDARY_VIDEO_STREAM %1").arg(Event.param));
            m_currentSecondaryVideoStream = static_cast<int>(Event.param);
            break;

        case BD_EVENT_PG_TEXTST:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_PG_TEXTST %1")
                .arg(Event.param ? "enable" : "disable"));
            m_pgTextSTEnabled = (Event.param != 0U);
            break;
        case BD_EVENT_SECONDARY_AUDIO:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SECONDARY_AUDIO %1")
                .arg(Event.param ? "enable" : "disable"));
            m_secondaryAudioEnabled = (Event.param != 0U);
            break;
        case BD_EVENT_SECONDARY_VIDEO:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SECONDARY_VIDEO %1")
                .arg(Event.param ? "enable" : "disable"));
            m_secondaryVideoEnabled = (Event.param != 0U);
            break;
        case BD_EVENT_SECONDARY_VIDEO_SIZE:
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_SECONDARY_VIDEO_SIZE %1")
                .arg(Event.param==0 ? "PIP" : "fullscreen"));
            m_secondaryVideoIsFullscreen = (Event.param != 0U);
            break;

        /* status */
        case BD_EVENT_IDLE:
            /* Nothing to do. Playlist is not playing, but title applet is running.
             * Application should not call bd_read*() immediately again to avoid busy loop. */
            usleep(40000);
            break;

        case BD_EVENT_MENU:
            /* Interactive menu visible */
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("EVENT_MENU %1")
                .arg(Event.param==0 ? "no" : "yes"));
            m_inMenu = (Event.param == 1);
            break;

        case BD_EVENT_KEY_INTEREST_TABLE:
            /* BD-J key interest table changed */
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("ToDo EVENT_KEY_INTEREST_TABLE %1")
                .arg(Event.param));
            break;

        case BD_EVENT_UO_MASK_CHANGED:
            /* User operations mask was changed */
            LOG(VB_PLAYBACK, LOG_INFO, LOC + QString("ToDo EVENT_UO_MASK_CHANGED %1")
                .arg(Event.param));
            break;

        default:
            LOG(VB_PLAYBACK, LOG_ERR, LOC + QString("Unknown Event! %1 %2")
                .arg(Event.event).arg(Event.param));
            break;
      }
}

bool BDRingBuffer::IsInStillFrame(void) const
{
    return m_stillTime > 0 && m_stillMode != BLURAY_STILL_NONE;
}

/**
 * \brief Find the stream with the given ID from an array of streams.
 * \param streamid      The stream ID (pid) to look for
 * \param streams       Pointer to an array of streams
 * \param streamCount   Number of streams in the array
 * \return Pointer to the matching stream if found, otherwise nullptr.
 */
const BLURAY_STREAM_INFO* BDRingBuffer::FindStream(uint StreamID,
                                                   BLURAY_STREAM_INFO* Streams,
                                                   int StreamCount)
{
    const BLURAY_STREAM_INFO* stream = nullptr;
    for (int i = 0; i < StreamCount && !stream; i++)
        if (Streams[i].pid == StreamID)
            stream = &Streams[i];
    return stream;
}

bool BDRingBuffer::IsValidStream(uint StreamId)
{
    if (m_currentTitleInfo && m_currentTitleInfo->clip_count > 0)
    {
        bd_clip& clip = m_currentTitleInfo->clips[0];
        if (FindStream(StreamId, clip.audio_streams,     clip.audio_stream_count) ||
            FindStream(StreamId, clip.video_streams,     clip.video_stream_count) ||
            FindStream(StreamId, clip.ig_streams,        clip.ig_stream_count) ||
            FindStream(StreamId, clip.pg_streams,        clip.pg_stream_count) ||
            FindStream(StreamId, clip.sec_audio_streams, clip.sec_audio_stream_count) ||
            FindStream(StreamId, clip.sec_video_streams, clip.sec_video_stream_count))
        {
            return true;
        }
    }

    return false;
}

void BDRingBuffer::UnblockReading(void)
{
    m_processState = PROCESS_REPROCESS;
}

bool BDRingBuffer::IsReadingBlocked(void)
{
    return m_processState == PROCESS_WAIT;
}

bool BDRingBuffer::IsHDMVNavigation(void) const
{
    return m_isHDMVNavigation;
}

void BDRingBuffer::WaitForPlayer(void)
{
    if (m_ignorePlayerWait)
        return;

    LOG(VB_PLAYBACK, LOG_INFO, LOC + "Waiting for player's buffers to drain");
    m_playerWait = true;
    int count = 0;
    while (m_playerWait && count++ < 200)
        usleep(10000);
    if (m_playerWait)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Player wait state was not cleared");
        m_playerWait = false;
    }
}

void BDRingBuffer::IgnoreWaitStates(bool Ignore)
{
    m_ignorePlayerWait = Ignore;
}

bool BDRingBuffer::StartFromBeginning(void)
{
    if (m_bdnav && m_isHDMVNavigation)
    {
        LOG(VB_PLAYBACK, LOG_INFO, LOC + "Starting from beginning...");
        return true; //bd_play(m_bdnav);
    }
    return true;
}

bool BDRingBuffer::GetNameAndSerialNum(QString &Name, QString &SerialNum)
{
    if (!m_bdnav)
        return false;
    Name      = m_name;
    SerialNum = m_serialNumber;
    return !SerialNum.isEmpty();
}

/** \brief Get a snapshot of the current BD state
 */
bool BDRingBuffer::GetBDStateSnapshot(QString& State)
{
    int      title = GetCurrentTitle();
    uint64_t time  = m_currentTime;
    uint64_t angle = GetCurrentAngle();
    if (title >= 0)
        State = QString("title:%1,time:%2,angle:%3").arg(title).arg(time).arg(angle);
    else
        State.clear();
    return !State.isEmpty();
}

/** \brief Restore a BD snapshot
 */
bool BDRingBuffer::RestoreBDStateSnapshot(const QString& State)
{
    QStringList states = State.split(",", QString::SkipEmptyParts);
    QHash<QString, uint64_t> settings;

    for (const QString& state : states)
    {
        QStringList keyvalue = state.split(":", QString::SkipEmptyParts);
        if (keyvalue.length() != 2)
        {
            LOG(VB_PLAYBACK, LOG_ERR, LOC + QString("Invalid BD state: %1 (%2)")
                .arg(state).arg(State));
        }
        else
        {
            settings[keyvalue[0]] = keyvalue[1].toULongLong();
            //LOG(VB_PLAYBACK, LOG_DEBUG, LOC + QString( "%1 = %2" ).arg(keyvalue[0]).arg(keyvalue[1]));
        }
    }

    if (settings.contains("title") && settings.contains("time"))
    {
        uint32_t title = static_cast<uint32_t>(settings["title"]);
        uint64_t time  = settings["time"];
        uint64_t angle = 0;

        if (settings.contains("angle"))
            angle = settings["angle"];

        if (title != static_cast<uint32_t>(m_currentTitle))
            SwitchTitle(title);

        SeekInternal(static_cast<long long>(time), SEEK_SET);
        SwitchAngle(static_cast<uint>(angle));
        return true;
    }

    return false;
}


void BDRingBuffer::ClearOverlays(void)
{
    QMutexLocker lock(&m_overlayLock);

    while (!m_overlayImages.isEmpty())
    {
        BDOverlay *overlay = m_overlayImages.takeFirst();
        delete overlay;
        overlay = nullptr;
    }

    // NOLINTNEXTLINE(modernize-loop-convert)
    for (int i = 0; i < m_overlayPlanes.size(); i++)
    {
        BDOverlay*& osd = m_overlayPlanes[i];

        if (osd)
        {
            delete osd;
            osd = nullptr;
        }
    }
}

BDOverlay* BDRingBuffer::GetOverlay(void)
{
    QMutexLocker lock(&m_overlayLock);
    if (!m_overlayImages.isEmpty())
        return m_overlayImages.takeFirst();
    return nullptr;
}

void BDRingBuffer::SubmitOverlay(const bd_overlay_s* const Overlay)
{
    if (!Overlay || (Overlay && (Overlay->plane > m_overlayPlanes.size())))
        return;

    LOG(VB_PLAYBACK, LOG_DEBUG, QString("--------------------"));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay->cmd    = %1, %2")
        .arg(Overlay->cmd).arg(Overlay->plane));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay rect    = (%1,%2,%3,%4)")
        .arg(Overlay->x).arg(Overlay->y).arg(Overlay->w).arg(Overlay->h));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay->pts    = %1")
        .arg(Overlay->pts));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("update palette  = %1")
        .arg(Overlay->palette_update_flag ? "yes":"no"));

    BDOverlay*& osd = m_overlayPlanes[Overlay->plane];

    switch(Overlay->cmd)
    {
        case BD_OVERLAY_INIT:
            // init overlay plane. Size and position of plane in x,y,w,h
            // init overlay plane. Size of plane in w,h
            delete osd;
            osd = new BDOverlay(Overlay);
            break;
        case BD_OVERLAY_CLOSE:
            // close overlay
            {
                if (osd)
                {
                    delete osd;
                    osd = nullptr;
                }
                QMutexLocker lock(&m_overlayLock);
                m_overlayImages.append(new BDOverlay());
            }
            break;
        /* following events can be processed immediately, but changes
         * should not be flushed to display before next FLUSH event
         */
        case BD_OVERLAY_HIDE:    /* overlay is empty and can be hidden */
        case BD_OVERLAY_CLEAR:   /* clear plane */
            if (osd)
                osd->Wipe();
            break;
        case BD_OVERLAY_WIPE:    /* clear area (x,y,w,h) */
            if (osd)
                osd->Wipe(Overlay->x, Overlay->y, Overlay->w, Overlay->h);
            break;
        case BD_OVERLAY_DRAW:    /* draw bitmap (x,y,w,h,img,palette,crop) */
            if (osd)
            {
                const BD_PG_RLE_ELEM *rlep = Overlay->img;
                int actual = Overlay->w * Overlay->h;
                uint8_t *data = osd->m_image.bits();
                data = &data[(Overlay->y * osd->m_image.bytesPerLine()) + Overlay->x];
                for (int i = 0; i < actual; i += rlep->len, rlep++)
                {
                    int dst_y = (i / Overlay->w) * osd->m_image.bytesPerLine();
                    int dst_x = (i % Overlay->w);
                    memset(data + dst_y + dst_x, rlep->color, rlep->len);
                }
                osd->SetPalette(Overlay->palette);
            }
            break;

        case BD_OVERLAY_FLUSH:   /* all changes have been done, flush overlay to display at given pts */
            if (osd)
            {
                auto* newOverlay = new BDOverlay(*osd);
                newOverlay->m_image = osd->m_image.convertToFormat(QImage::Format_ARGB32);
                newOverlay->m_pts = Overlay->pts;
                QMutexLocker lock(&m_overlayLock);
                m_overlayImages.append(newOverlay);
            }
            break;
        default: break;
    }
}

void BDRingBuffer::SubmitARGBOverlay(const bd_argb_overlay_s * const Overlay)
{
    if (!Overlay || (Overlay && (Overlay->plane > m_overlayPlanes.size())))
        return;

    LOG(VB_PLAYBACK, LOG_DEBUG, QString("--------------------"));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay->cmd,plane = %1, %2")
        .arg(Overlay->cmd).arg(Overlay->plane));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay->(x,y,w,h) = %1,%2,%3x%4 - %5")
        .arg(Overlay->x).arg(Overlay->y).arg(Overlay->w).arg(Overlay->h).arg(Overlay->stride));
    LOG(VB_PLAYBACK, LOG_DEBUG, QString("overlay->pts       = %1").arg(Overlay->pts));

    BDOverlay*& osd = m_overlayPlanes[Overlay->plane];
    switch(Overlay->cmd)
    {
        case BD_ARGB_OVERLAY_INIT:
            /* init overlay plane. Size of plane in w,h */
            delete osd;
            osd = new BDOverlay(Overlay);
            break;
        case BD_ARGB_OVERLAY_CLOSE:
            /* close overlay */
            {
                if (osd)
                {
                    delete osd;
                    osd = nullptr;
                }
                QMutexLocker lock(&m_overlayLock);
                m_overlayImages.append(new BDOverlay());
            }
            break;
        /* following events can be processed immediately, but changes
         * should not be flushed to display before next FLUSH event
         */
        case BD_ARGB_OVERLAY_DRAW:
            if (osd)
            {
                /* draw image */
                uint8_t* data = osd->m_image.bits();
                int32_t srcOffset = 0;
                int32_t dstOffset = (Overlay->y * osd->m_image.bytesPerLine()) + (Overlay->x * 4);
                for (uint16_t y = 0; y < Overlay->h; y++)
                {
                    memcpy(&data[dstOffset], &Overlay->argb[srcOffset], Overlay->w * 4);
                    dstOffset += osd->m_image.bytesPerLine();
                    srcOffset += Overlay->stride;
                }
            }
            break;
        case BD_ARGB_OVERLAY_FLUSH:
            /* all changes have been done, flush overlay to display at given pts */
            if (osd)
            {
                QMutexLocker lock(&m_overlayLock);
                auto* newOverlay = new BDOverlay(*osd);
                newOverlay->m_pts = Overlay->pts;
                m_overlayImages.append(newOverlay);
            }
            break;
        default:
            LOG(VB_PLAYBACK, LOG_ERR, QString("Unknown ARGB overlay - %1").arg(Overlay->cmd));
            break;
    }
}
