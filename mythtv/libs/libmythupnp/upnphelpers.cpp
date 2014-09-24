
#include "upnphelpers.h"

#include "mythlogging.h"
#include "mythcorecontext.h"

namespace UPnPDateTime
{

QString DurationFormat(uint32_t msec)
{
    // Appendix D. Date&Time Syntax - UPnP ContentDirectory Service 2008, 2013
    // duration ::= 'P' [n 'D'] time
    // time ::= HH ':' MM ':' SS
    //
    // e.g.
    //"<upnp:scheduledDuration>P1D02:30:23</upnp:scheduledDuration>"
    // 1 day 2 Hours 30 Minutes 23 Seconds

    QString durationStr = "P%1%2";
    QString dayStr;
    if ( msec > (1000 * 3600 * 24) )
    {
        dayStr = QString("D%1").arg(msec % (1000 * 3600 * 24)); // 24 Hours
    }
    QString timeStr = UPnPDateTime::TimeFormat(msec);

    return durationStr.arg(dayStr).arg(timeStr);
}

QString TimeFormat(const QTime &time)
{
    QString timeStr = time.toString("HH:mm:ss");
    return timeStr;
}

QString TimeFormat(uint32_t msec)
{
    QString timeStr;
    timeStr.sprintf("%02d:%02d:%02d",
                        (msec / (1000 * 60 * 60)) % 24, // Hours
                        (msec / (1000 * 60)) % 60,      // Minutes
                        (msec / 1000) % 60);            // Seconds

    return timeStr;
}

QString DateTimeFormat(const QDateTime &dateTime)
{
    QString dateTimeStr = dateTime.toString(Qt::ISODate);
    return dateTimeStr;
}

QString NamedDayFormat(const QDateTime &dateTime)
{
    return UPnPDateTime::NamedDayFormat(dateTime.date());
}

QString NamedDayFormat(const QDate &date)
{
    // Note QDate::toString() and QDate::shortDayName() return localized strings
    // which are of no use to us.
    //
    // Valid values per the spec are MON, TUE, WED, THU etc
    switch (date.dayOfWeek())
    {
        case 1:
            return "MON";
        case 2:
            return "TUE";
        case 3:
            return "WED";
        case 4:
            return "THU";
        case 5:
            return "FRI";
        case 6:
            return "SAT";
        case 7:
            return "SUN";
        default:
            return "";
    }
}

QString resDurationFormat(uint32_t msec)
{
    // Appendix B.2 Resource Encoding Characteristics Properties
    //          B.2.1.4 res@duration

    QString resDurationStr;
    // H[H]:MM:SS[.FS]
    // H = Hours (1-2 digits),
    // M = Minutes (2 digits, 0 prefix)
    // S = Seconds (2 digits, 0 prefix)
    // FS = Fractional Seconds (milliseconds)
    resDurationStr.sprintf("%01d:%02d:%02d.%01d",
                (msec / (1000 * 60 * 60)) % 24, // Hours
                (msec / (1000 * 60)) % 60,      // Minutes
                (msec / 1000) % 60,             // Seconds
                msec % 1000);

    return resDurationStr;
}

};

namespace DLNA
{

QString DLNAProfileName(const QString &mimeType, const QSize &resolution,
                        const double videoFrameRate, const QString &container,
                        const QString &vidCodec, const QString &audioCodec)
{
    QString sProfileName;
    bool isHD = (resolution.height() >= 720);

    if (mimeType == "audio/mpeg")
    {
        sProfileName = "MP3X";
    }
    else if (mimeType == "audio/x-ms-wma")
    {
        sProfileName = "WMAFULL";
    }
    else if (mimeType == "audio/vnd.dolby.dd-raw")
    {
        sProfileName = "AC3";
    }
    else if (mimeType == "audio/mp4")
    {
        sProfileName = "AAC_ISO_320";
    }
    else if (mimeType == "video/x-matroska" || container == "matroska")
    {
        // TODO: We need to know the video and audio codecs before we can serve
        // up the correct profile.
        //
        // sAdditionalInfoList << "DLNA.ORG_PN=AVC_MKV_SOMETHING";
        if (vidCodec == "H264")
        {
            // TODO We need to know the H264 profile used, for now we go with High Profile
            //      as this covers all - devices supporting High Profile also support
            //      Medium Profile
            if (audioCodec == "AAC") // No HEAAC detection atm, it's a sub-profile of AAC
            {
                sProfileName = "AVC_MKV_HP_HD_AAC_MULT5";
            }
            else if (audioCodec == "AC3")
            {
                sProfileName = "AVC_MKV_HP_HD_AC3";
            }
            else if (audioCodec == "E-AC3") // Up to 35Mbps
            {
                sProfileName = "AVC_MKV_HP_HD_EAC3";
            }
            else if (audioCodec == "MP3")
            {
                sProfileName = "AVC_MKV_HP_HD_MPEG1_L3";
            }
            else if (audioCodec == "DTS") // Up to 55Mbps // No DTSE/DTSL detection atm, sub-profiles of DTS
            {
                sProfileName = "AVC_MKV_HP_HD_DTS";
            }
            else if (audioCodec == "MLP") // Up to 45Mbps
            {
                sProfileName = "AVC_MKV_HP_HD_MLP";
            }
        }
    }
    else if (mimeType == "video/mpeg")
    {
        // We currently only handle untranscoded files in TS containers
        // adhering to Digital TV standards. This should cover 75% of recordings
        // although not older recordings which were made from analogue sources.
        //
        // It's a starting point, and temporary until we can implement a more
        // robust way of determing the correct profile.

        //QString sBroadcastStandard = "";
        // HACK This is just temporary until we start storing more video
        // information in the database for each file and can determine this
        // stuff 'properly'
        QString sCountryCode = gCoreContext->GetLocale()->GetCountryCode();
        bool isNorthAmerica = (sCountryCode == "us" || sCountryCode == "ca" ||
                               sCountryCode == "mx"); // North America (NTSC/ATSC)

        if (container == "MPEG-2 PS")
        {
            if (isHD && audioCodec == "DTS")
                sProfileName = "MPEG_PS_HD_DTS";
            else if (isHD)
            {
                // Fallthough, no DLNA profiles
            }
            else if (audioCodec == "DTS")
                sProfileName = "MPEG_PS_SD_DTS";
            else
            {
                if (isNorthAmerica)
                    sProfileName = "MPEG_PS_NTSC";
                else
                    sProfileName = "MPEG_PS_PAL";
            }
        }
        else if (container == "MPEG-2 TS")
        {
            if (isNorthAmerica)
            {
                if (vidCodec == "H264")
                    sProfileName = "AVC_TS_NA_ISO";
                else if (isHD) // && videoCodec == "MPEG2VIDEO"
                    sProfileName = "MPEG_TS_HD_NA_ISO";
                else // if (videoCodec == "MPEG2VIDEO")
                    sProfileName = "MPEG_TS_SD_NA_ISO";
            }
            else // Europe standard (DVB)
            {
                if (vidCodec == "H264" || isHD) // All HD is AVC with DVB
                    sProfileName = "AVC_TS_EU_ISO";
                else // if (videoCodec == "MPEG2VIDEO")
                    sProfileName = "MPEG_TS_SD_EU_ISO";
            }
        }
        // Fall through
    }
    else if (mimeType == "image/jpeg")
    {
        if (resolution.width() <= 160 && resolution.height() <= 160)
            sProfileName = "JPEG_TN";
        else if (resolution.width() <= 640 && resolution.height() <= 480)
            sProfileName = "JPEG_SM";
        else if (resolution.width() <= 1024 && resolution.height() <= 768)
            sProfileName = "JPEG_MED";
        else if (resolution.width() <= 4096 && resolution.height() <= 4096)
            sProfileName = "JPEG_LRG";
    }
    else if (mimeType == "image/png")
    {
        if (resolution.width() <= 160 && resolution.height() <= 160)
            sProfileName = "PNG_TN";
        else if (resolution.width() <= 4096 && resolution.height() <= 4096)
            sProfileName = "PNG_LRG";
    }
    else if (mimeType == "image/gif")
    {
        if (resolution.width() <= 1600 && resolution.height() <= 1200)
            sProfileName = "GIF_LRG";
    }
    else
    {
        // Not a DLNA supported profile?
    }

    return sProfileName;
}

// NOTE The order of the DLNA args is mandatory - 7.4.1.3.17 MM protocolInfo values: 4th field
QString ProtocolInfoString(UPNPProtocol::TransferProtocol protocol,
                           const QString &mimeType, const QSize &resolution,
                           double videoFrameRate, const QString &container,
                           const QString &videoCodec, const QString &audioCodec,
                           bool isTranscoded)
{
    QString str;
    QString protocolStr;
    QStringList sAdditionalInfoList;

    //
    //  1st Field = protocol
    //

    if (protocol == UPNPProtocol::kHTTP)
        protocolStr = "http-get";
    else if (protocol == UPNPProtocol::kRTP)
        protocolStr = "rtsp-rtp-udp";

    //
    //  4th_field = pn-param [op-param] [ps-param] [ci-param] [flags-param] [ *(other-param)]
    //

    //
    // PN-Param (Profile Name)
    //
    QString sProfileName = DLNAProfileName(mimeType, resolution, videoFrameRate,
                                           container, videoCodec, audioCodec);
    if (!sProfileName.isEmpty())
        sAdditionalInfoList << QString("DLNA.ORG_PN=%1").arg(sProfileName);

    //
    // OP-Param (Operation Parameters)
    //
    sAdditionalInfoList << DLNA::OpParamString(protocol);

    //
    // PS-Param (Play Speed)
    //

        // Not presently used by MythTV

    //
    // CI-Param  - Optional, may be omitted if value is zero (0)
    //
    if (isTranscoded)
        sAdditionalInfoList << DLNA::ConversionIndicatorString(isTranscoded);

    //
    // Flags-Param
    //
    if (mimeType.startsWith("audio") || mimeType.startsWith("video"))
    {
        sAdditionalInfoList << DLNA::FlagsString(DLNA::ktm_s |
                                                 DLNA::ktm_b |
                                                 DLNA::kv1_5_flag);
    }
    else
    {
        sAdditionalInfoList << DLNA::FlagsString(DLNA::ktm_i |
                                                 DLNA::ktm_b |
                                                 DLNA::kv1_5_flag);
    }

    //
    // Build the complete string
    //
    QString sAdditionalInfo = "*";
    // If we have DLNA additional info and we have the mandatory PN param
    // then add it to the string. If the PN is missing then we must ignore the
    // rest
    if (!sAdditionalInfoList.isEmpty() &&
        sAdditionalInfoList.first().startsWith("DLNA.ORG_PN"))
        sAdditionalInfo = sAdditionalInfoList.join(";");
    str = QString( "%1:*:%2:%3" ).arg( protocolStr ).arg( mimeType ).arg( sAdditionalInfo );

    if (str.length() > 256)
        LOG(VB_GENERAL, LOG_WARNING, "DLNA::ProtocolInfoString() : ProtocolInfo string exceeds "
                                     "256 byte limit for compatibility with older UPnP devices. "
                                     "Consider omitting optional DLNA information such as ci-param");
    return str;
}


QString FlagsString(uint32_t flags)
{
    QString str;

    if (flags == 0)
        return str;

    // DLNA::kv1_5_flag must be set
    if ((flags & DLNA::ktm_s) && (flags & DLNA::ktm_i))
    {
        LOG(VB_GENERAL, LOG_ERR, "Programmer Error: 'Streaming' and 'Interactive' mode flags are mutally exclusive");
        flags &= ~(DLNA::ktm_s | DLNA::ktm_i);
    }

    // DLNA::kv1_5_flag must be set otherwise other flags are ignored
    if (flags & ~DLNA::kv1_5_flag)
        flags |= DLNA::kv1_5_flag;

    str.sprintf("DLNA.ORG_FLAGS=%08X000000000000000000000000", flags); // 8 HEX Digits, following by 24 zeros
    return str;
}

QString OpParamString(UPNPProtocol::TransferProtocol protocol)
{
    QString str = "DLNA.ORG_OP=%1%2";

    if (protocol == UPNPProtocol::kHTTP)
    {
        // Section 7.4.1.3.20
        str = str.arg("0")  // A-val - DLNA Time Seek header support (not currently supported)
                 .arg("1"); // B-val - HTTP Range support
    }
    else if (protocol == UPNPProtocol::kRTP)
    {
        // Section 7.4.1.3.21
        str = str.arg("0")  // A-val - Range Header Support
                 .arg("0"); // B-val - Must always zero
    }

    return str;
}

QString ConversionIndicatorString(bool wasConverted)
{
    QString str = "DLNA.ORG_CI=%1";

    return str.arg(wasConverted ? "1" : "0");
}

};
