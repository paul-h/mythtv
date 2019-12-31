
#include "mythcorecontext.h"

#include <QCoreApplication>

#include "tvbrowsehelper.h"
#include "playercontext.h"
#include "remoteencoder.h"
#include "recordinginfo.h"
#include "channelutil.h"
#include "mythlogging.h"
#include "cardutil.h"
#include "tv_play.h"

#define LOC      QString("BH: ")

#define GetPlayer(X,Y) GetPlayerHaveLock(X, Y, __FILE__ , __LINE__)
#define GetOSDLock(X) GetOSDL(X, __FILE__, __LINE__)

static void format_time(int seconds, QString &tMin, QString &tHrsMin)
{
    int minutes     = seconds / 60;
    int hours       = minutes / 60;
    int min         = minutes % 60;

    tMin = TV::tr("%n minute(s)", "", minutes);
    tHrsMin = QString("%1:%2").arg(hours).arg(min, 2, 10, QChar('0'));
}

TVBrowseHelper::TVBrowseHelper(
    TV      *tv,
    uint     browse_max_forward,
    bool     browse_all_tuners,
    bool     use_channel_groups,
    const QString&  db_channel_ordering) :
    MThread("TVBrowseHelper"),
    m_tv(tv),
    m_dbBrowseMaxForward(browse_max_forward),
    m_dbBrowseAllTuners(browse_all_tuners),
    m_dbUseChannelGroups(use_channel_groups)
{
    m_dbAllChannels = ChannelUtil::GetChannels(
        0, true, "channum, callsign");
    ChannelUtil::SortChannels(
        m_dbAllChannels, db_channel_ordering, false);

    ChannelInfoList::const_iterator it = m_dbAllChannels.begin();
    for (; it != m_dbAllChannels.end(); ++it)
    {
        m_dbChanidToChannum[(*it).m_chanId] = (*it).m_chanNum;
        m_dbChanidToSourceid[(*it).m_chanId] = (*it).m_sourceId;
        m_dbChannumToChanids.insert((*it).m_chanNum,(*it).m_chanId);
    }

    m_dbAllVisibleChannels = ChannelUtil::GetChannels(
        0, true, "channum, callsign");
    ChannelUtil::SortChannels(
        m_dbAllVisibleChannels, db_channel_ordering, false);

    start();
}


/// \brief Begins channel browsing.
/// \note This may only be called from the UI thread.
bool TVBrowseHelper::BrowseStart(PlayerContext *ctx, bool skip_browse)
{
    if (!gCoreContext->IsUIThread())
        return false;

    QMutexLocker locker(&m_lock);

    if (m_ctx)
        return m_ctx == ctx;

    m_tv->ClearOSD(ctx);

    ctx->LockPlayingInfo(__FILE__, __LINE__);
    if (ctx->m_playingInfo)
    {
        m_ctx       = ctx;
        m_chanNum   = ctx->m_playingInfo->GetChanNum();
        m_chanId    = ctx->m_playingInfo->GetChanID();
        m_startTime = ctx->m_playingInfo->GetScheduledStartTime(MythDate::ISODate);
        ctx->UnlockPlayingInfo(__FILE__, __LINE__);

        if (!skip_browse)
        {
            BrowseInfo bi(BROWSE_SAME, m_chanNum, m_chanId, m_startTime);
            locker.unlock();
            BrowseDispInfo(ctx, bi);
        }
        return true;
    }
    ctx->UnlockPlayingInfo(__FILE__, __LINE__);
    return false;
}

/** \brief Ends channel browsing.
 *         Changing the channel if change_channel is true.
 *  \note This may only be called from the UI thread.
 *  \param ctx PlayerContext to end browsing on
 *  \param change_channel iff true we call ChangeChannel()
 */
void TVBrowseHelper::BrowseEnd(PlayerContext *ctx, bool change_channel)
{
    if (!gCoreContext->IsUIThread())
        return;

    QMutexLocker locker(&m_lock);

    if (ctx && m_ctx != ctx)
        return;

    if (!m_ctx)
        return;

    {
        QMutexLocker locker2(&m_tv->m_timerIdLock);
        if (m_tv->m_browseTimerId)
        {
            m_tv->KillTimer(m_tv->m_browseTimerId);
            m_tv->m_browseTimerId = 0;
        }
    }

    m_list.clear();
    m_wait.wakeAll();

    OSD *osd = m_tv->GetOSDLock(ctx);
    if (osd)
        osd->HideWindow("browse_info");
    m_tv->ReturnOSDLock(ctx, osd);

    if (change_channel)
        m_tv->ChangeChannel(ctx, 0, m_chanNum);

    m_ctx = nullptr;
}

void TVBrowseHelper::BrowseDispInfo(PlayerContext *ctx, BrowseInfo &bi)
{
    if (!gCoreContext->IsUIThread())
        return;

    if (!BrowseStart(ctx, true))
        return;

    {
        QMutexLocker locker(&m_tv->m_timerIdLock);
        if (m_tv->m_browseTimerId)
        {
            m_tv->KillTimer(m_tv->m_browseTimerId);
            m_tv->m_browseTimerId =
                m_tv->StartTimer(TV::kBrowseTimeout, __LINE__);
        }
    }

    QMutexLocker locker(&m_lock);
    if (BROWSE_SAME == bi.m_dir)
        m_list.clear();
    m_list.push_back(bi);
    m_wait.wakeAll();
}

void TVBrowseHelper::BrowseChannel(PlayerContext *ctx, const QString &channum)
{
    if (!gCoreContext->IsUIThread())
        return;

    if (m_dbBrowseAllTuners)
    {
        BrowseInfo bi(channum, 0);
        BrowseDispInfo(ctx, bi);
        return;
    }

    if (!ctx->m_recorder || !ctx->m_lastCardid)
        return;

    uint    inputid   = ctx->m_lastCardid;
    uint    sourceid  = CardUtil::GetSourceID(inputid);
    if (sourceid)
    {
        BrowseInfo bi(channum, sourceid);
        BrowseDispInfo(ctx, bi);
    }
}

BrowseInfo TVBrowseHelper::GetBrowsedInfo(void) const
{
    QMutexLocker locker(&m_lock);
    BrowseInfo bi(BROWSE_SAME);
    if (m_ctx != nullptr)
    {
        bi.m_chanNum   = m_chanNum;
        bi.m_chanId    = m_chanId;
        bi.m_startTime = m_startTime;
    }
    return bi;
}

/**
 *  \note This may only be called from the UI thread.
 */
bool TVBrowseHelper::IsBrowsing(void) const
{
    if (!gCoreContext->IsUIThread())
        return true;

    return m_ctx != nullptr;
}

/** \brief Returns a chanid for the channum, or 0 if none is available.
 *
 *  This will prefer a given sourceid first, and then a given card id,
 *  but if one or the other can not be satisfied but m_dbBrowseAllTuners
 *  is set then it will look to see if the chanid is available on any
 *  tuner.
 */
uint TVBrowseHelper::GetChanId(
    const QString &channum, uint pref_cardid, uint pref_sourceid) const
{
    if (pref_sourceid)
    {
        auto it = m_dbAllChannels.cbegin();
        for (; it != m_dbAllChannels.cend(); ++it)
        {
            if ((*it).m_sourceId == pref_sourceid && (*it).m_chanNum == channum)
                return (*it).m_chanId;
        }
    }

    if (pref_cardid)
    {
        auto it = m_dbAllChannels.cbegin();
        for (; it != m_dbAllChannels.cend(); ++it)
        {
            if ((*it).GetInputIds().contains(pref_cardid) &&
                (*it).m_chanNum == channum)
                return (*it).m_chanId;
        }
    }

    if (m_dbBrowseAllTuners)
    {
        auto it = m_dbAllChannels.cbegin();
        for (; it != m_dbAllChannels.cend(); ++it)
        {
            if ((*it).m_chanNum == channum)
                return (*it).m_chanId;
        }
    }

    return 0;
}

/**
 *  \brief Fetches information on the desired program from the backend.
 *  \param direction BrowseDirection to get information on.
 *  \param infoMap InfoMap to fill in with returned data
 */
void TVBrowseHelper::GetNextProgram(
    BrowseDirection direction, InfoMap &infoMap) const
{
    if (!m_ctx || !m_ctx->m_recorder)
        return;

    QString title;
    QString subtitle;
    QString desc;
    QString category;
    QString endtime;
    QString callsign;
    QString iconpath;
    QDateTime begts;
    QDateTime endts;

    QString starttime = infoMap["dbstarttime"];
    QString chanid    = infoMap["chanid"];
    QString channum   = infoMap["channum"];
    QString seriesid  = infoMap["seriesid"];
    QString programid = infoMap["programid"];

    m_ctx->m_recorder->GetNextProgram(
        direction,
        title,     subtitle,  desc,      category,
        starttime, endtime,   callsign,  iconpath,
        channum,   chanid,    seriesid,  programid);

    if (!starttime.isEmpty())
        begts = MythDate::fromString(starttime);
    else
        begts = MythDate::fromString(infoMap["dbstarttime"]);

    infoMap["starttime"] = MythDate::toString(begts, MythDate::kTime);
    infoMap["startdate"] = MythDate::toString(
        begts, MythDate::kDateFull | MythDate::kSimplify);

    infoMap["endtime"] = infoMap["enddate"] = "";
    if (!endtime.isEmpty())
    {
        endts = MythDate::fromString(endtime);
        infoMap["endtime"] = MythDate::toString(endts, MythDate::kTime);
        infoMap["enddate"] = MythDate::toString(
            endts, MythDate::kDateFull | MythDate::kSimplify);
    }

    infoMap["lenmins"] = TV::tr("%n minute(s)", "", 0);
    infoMap["lentime"] = "0:00";
    if (begts.isValid() && endts.isValid())
    {
        QString lenM;
        QString lenHM;
        format_time(begts.secsTo(endts), lenM, lenHM);
        infoMap["lenmins"] = lenM;
        infoMap["lentime"] = lenHM;
    }

    infoMap["dbstarttime"] = starttime;
    infoMap["dbendtime"]   = endtime;
    infoMap["title"]       = title;
    infoMap["subtitle"]    = subtitle;
    infoMap["description"] = desc;
    infoMap["category"]    = category;
    infoMap["callsign"]    = callsign;
    infoMap["channum"]     = channum;
    infoMap["chanid"]      = chanid;
    infoMap["iconpath"]    = iconpath;
    infoMap["seriesid"]    = seriesid;
    infoMap["programid"]   = programid;
}

void TVBrowseHelper::GetNextProgramDB(
    BrowseDirection direction, InfoMap &infoMap) const
{
    uint chanid = infoMap["chanid"].toUInt();
    if (!chanid)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "GetNextProgramDB() requires a chanid");
        return;
    }

    int chandir = -1;
    switch (direction)
    {
        case BROWSE_UP:       chandir = CHANNEL_DIRECTION_UP;       break;
        case BROWSE_DOWN:     chandir = CHANNEL_DIRECTION_DOWN;     break;
        case BROWSE_FAVORITE: chandir = CHANNEL_DIRECTION_FAVORITE; break;
        case BROWSE_SAME:
        case BROWSE_LEFT:
        case BROWSE_RIGHT:
        case BROWSE_INVALID:
        default: break; // quiet -Wswitch-enum
    }
    if (chandir != -1)
    {
        chanid = ChannelUtil::GetNextChannel(
            m_dbAllVisibleChannels, chanid, 0 /*mplexid_restriction*/,
            0 /* chanid restriction */,
            static_cast<ChannelChangeDirection>(chandir),
            true /*skip non visible*/, true /*skip same callsign*/);
    }

    infoMap["chanid"]  = QString::number(chanid);
    infoMap["channum"] = m_dbChanidToChannum[chanid];

    QDateTime nowtime = MythDate::current();
    QDateTime latesttime = nowtime.addSecs(6*60*60);
    QDateTime browsetime = MythDate::fromString(infoMap["dbstarttime"]);

    MSqlBindings bindings;
    bindings[":CHANID"] = chanid;
    bindings[":NOWTS"] = nowtime;
    bindings[":LATESTTS"] = latesttime;
    bindings[":BROWSETS"] = browsetime;
    bindings[":BROWSETS2"] = browsetime;

    QString querystr = " WHERE program.chanid = :CHANID ";
    switch (direction)
    {
        case BROWSE_LEFT:
            querystr += " AND program.endtime <= :BROWSETS "
                " AND program.endtime > :NOWTS ";
            break;

        case BROWSE_RIGHT:
            querystr += " AND program.starttime > :BROWSETS "
                " AND program.starttime < :LATESTTS ";
            break;

        default:
            querystr += " AND program.starttime <= :BROWSETS "
                " AND program.endtime > :BROWSETS2 ";
    };

    ProgramList progList;
    ProgramList dummySched;
    LoadFromProgram(progList, querystr, bindings, dummySched);

    if (progList.empty())
    {
        infoMap["dbstarttime"] = "";
        return;
    }

    const ProgramInfo *prog = (direction == BROWSE_LEFT) ?
        progList[progList.size() - 1] : progList[0];

    infoMap["dbstarttime"] = prog->GetScheduledStartTime(MythDate::ISODate);
}

void TVBrowseHelper::run()
{
    RunProlog();
    QMutexLocker locker(&m_lock);
    while (true)
    {
        while (m_list.empty() && m_run)
            m_wait.wait(&m_lock);

        if (!m_run)
            break;

        BrowseInfo bi = m_list.front();
        m_list.pop_front();

        PlayerContext *ctx = m_ctx;

        vector<uint> chanids;
        if (BROWSE_SAME == bi.m_dir)
        {
            if (!bi.m_chanId)
            {
                vector<uint> chanids_extra;
                uint sourceid = m_dbChanidToSourceid[m_chanId];
                QMultiMap<QString,uint>::iterator it;
                it = m_dbChannumToChanids.lowerBound(bi.m_chanNum);
                for ( ; (it != m_dbChannumToChanids.end()) &&
                          (it.key() == bi.m_chanNum); ++it)
                {
                    if (m_dbChanidToSourceid[*it] == sourceid)
                        chanids.push_back(*it);
                    else
                        chanids_extra.push_back(*it);
                }
                chanids.insert(chanids.end(),
                               chanids_extra.begin(),
                               chanids_extra.end());
            }
            m_chanNum   = bi.m_chanNum;
            m_chanId    = (chanids.empty()) ? bi.m_chanId : chanids[0];
            m_startTime = bi.m_startTime;
        }

        BrowseDirection direction = bi.m_dir;

        QDateTime lasttime = MythDate::fromString(m_startTime);
        QDateTime curtime  = MythDate::current();
        if (lasttime < curtime)
            m_startTime = curtime.toString(Qt::ISODate);

        QDateTime maxtime  = curtime.addSecs(m_dbBrowseMaxForward);
        if ((lasttime > maxtime) && (direction == BROWSE_RIGHT))
            continue;

        m_lock.unlock();

        // if browsing channel groups is enabled or
        // direction if BROWSE_FAVORITES
        // Then pick the next channel in the channel group list to browse
        // If channel group is ALL CHANNELS (-1), then bypass picking from
        // the channel group list
        if ((m_dbUseChannelGroups || (direction == BROWSE_FAVORITE)) &&
            (direction != BROWSE_RIGHT) && (direction != BROWSE_LEFT) &&
            (direction != BROWSE_SAME))
        {
            m_tv->m_channelGroupLock.lock();
            if (m_tv->m_channelGroupId > -1)
            {
                ChannelChangeDirection dir = CHANNEL_DIRECTION_SAME;
                if ((direction == BROWSE_UP) || (direction == BROWSE_FAVORITE))
                    dir = CHANNEL_DIRECTION_UP;
                else if (direction == BROWSE_DOWN)
                    dir = CHANNEL_DIRECTION_DOWN;

                uint chanid = ChannelUtil::GetNextChannel(
                    m_tv->m_channelGroupChannelList, m_chanId, 0, 0, dir);
                direction = BROWSE_SAME;

                m_tv->m_channelGroupLock.unlock();

                m_lock.lock();
                m_chanId  = chanid;
                m_chanNum.clear();
                m_lock.unlock();
            }
            else
                m_tv->m_channelGroupLock.unlock();
        }

        if (direction == BROWSE_FAVORITE)
            direction = BROWSE_UP;

        InfoMap infoMap;
        infoMap["dbstarttime"] = m_startTime;
        infoMap["channum"]     = m_chanNum;
        infoMap["chanid"]      = QString::number(m_chanId);

        m_tv->GetPlayerReadLock(0,__FILE__,__LINE__);
        bool still_there = false;
        for (uint i = 0; i < m_tv->m_player.size() && !still_there; i++)
            still_there |= (ctx == m_tv->m_player[i]);
        if (still_there)
        {
            if (!m_dbBrowseAllTuners)
            {
                GetNextProgram(direction, infoMap);
            }
            else
            {
                if (!chanids.empty())
                {
                    for (size_t i = 0; i < chanids.size(); i++)
                    {
                        if (TV::IsTunable(ctx, chanids[i]))
                        {
                            infoMap["chanid"] = QString::number(chanids[i]);
                            GetNextProgramDB(direction, infoMap);
                            break;
                        }
                    }
                }
                else
                {
                    uint orig_chanid = infoMap["chanid"].toUInt();
                    GetNextProgramDB(direction, infoMap);
                    while (!TV::IsTunable(ctx, infoMap["chanid"].toUInt()) &&
                           (infoMap["chanid"].toUInt() != orig_chanid))
                    {
                        GetNextProgramDB(direction, infoMap);
                    }
                }
            }
        }
        m_tv->ReturnPlayerLock(ctx);

        m_lock.lock();
        if (!m_ctx && !still_there)
            continue;

        m_chanNum = infoMap["channum"];
        m_chanId  = infoMap["chanid"].toUInt();

        if (((direction == BROWSE_LEFT) || (direction == BROWSE_RIGHT)) &&
            !infoMap["dbstarttime"].isEmpty())
        {
            m_startTime = infoMap["dbstarttime"];
        }

        if (!m_list.empty())
        {
            // send partial info to UI for appearance of responsiveness
            QCoreApplication::postEvent(
                m_tv, new UpdateBrowseInfoEvent(infoMap));
            continue;
        }
        m_lock.unlock();

        // pull in additional data from the DB...
        if (m_tv->m_channelGroupId > -1 && m_dbUseChannelGroups)
            infoMap["channelgroup"] = ChannelGroup::GetChannelGroupName(m_tv->m_channelGroupId);
        else
            infoMap["channelgroup"] = QObject::tr("All channels");

        QDateTime startts = MythDate::fromString(m_startTime);
        RecordingInfo recinfo(m_chanId, startts, false);
        recinfo.ToMap(infoMap);
        infoMap["iconpath"] = ChannelUtil::GetIcon(recinfo.GetChanID());

        m_lock.lock();
        if (m_ctx)
        {
            QCoreApplication::postEvent(
                m_tv, new UpdateBrowseInfoEvent(infoMap));
        }
    }
    RunEpilog();
}
