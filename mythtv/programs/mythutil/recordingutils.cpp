// C++ includes
#include <iostream>
#include <sys/stat.h>

// Qt
#include <QFileInfo>
#include <QScopedPointer>

// libmyth* includes
#include "exitcodes.h"
#include "mythlogging.h"
#include "remoteutil.h"
#include "remotefile.h"
#include "mythsystem.h"

// Local includes
#include "recordingutils.h"

static QString CreateProgramInfoString(const ProgramInfo &pginfo)
{
    QDateTime recstartts = pginfo.GetRecordingStartTime();
    QDateTime recendts   = pginfo.GetRecordingEndTime();

    QString timedate = QString("%1 - %2")
        .arg(MythDate::toString(
                 recstartts, MythDate::kDateTimeFull | MythDate::kSimplify))
        .arg(MythDate::toString(recendts, MythDate::kTime));

    QString title = pginfo.GetTitle();

    QString extra;

    if (!pginfo.GetSubtitle().isEmpty())
    {
        extra = QString(" ~ ") + pginfo.GetSubtitle();
        int maxll = max(title.length(), 20);
        if (extra.length() > maxll)
            extra = extra.left(maxll - 3) + "...";
    }

    return QString("%1%2 - %3").arg(title).arg(extra).arg(timedate);
}

static int CheckRecordings(const MythUtilCommandLineParser &cmdline)
{
    cout << "Checking Recordings" << endl;

    ProgramInfo *p;
    std::vector<ProgramInfo *>  *recordingList = RemoteGetRecordedList(-1);
    int recordingCount = 0;
    int missingFileCount = 0;
    int zeroByteCount = 0;
    int missingSeetableCount = 0;
    bool foundFile = false;
    bool fixSeektable = cmdline.toBool("fixseektable");

    if (recordingList && !recordingList->empty())
    {
        vector<ProgramInfo *>::iterator i = recordingList->begin();
        for ( ; i != recordingList->end(); ++i)
        {
            p = *i;
            // ignore live tv and deleted recordings
            if (p->GetRecordingGroup() == "LiveTV" ||
                p->GetRecordingGroup() == "Deleted")
            {
                i = recordingList->erase(i);
                --i;
                continue;
            }

            cout << "Checking: " << qPrintable(CreateProgramInfoString(*p)) << endl;
            recordingCount++;
            foundFile = true;

            QString url = p->GetPlaybackURL();

            if (url.startsWith('/'))
            {
                QFileInfo fi(url);
                if (!fi.exists())
                {
                    cout << "File not found" << endl;
                    missingFileCount++;
                    foundFile = false;
                }
                else
                {
                    if (fi.size() == 0)
                    {
                        cout << "File was found but has zero length" << endl;
                        zeroByteCount++;
                    }
                }
            }
            else if (url.startsWith("myth:"))
            {
                if (!RemoteFile::Exists(url))
                {
                    cout << "File not found" << endl;
                    missingFileCount++;
                    foundFile = false;
                }
                else
                {
                    RemoteFile rf(url);
                    if (rf.GetFileSize() == 0)
                    {
                        cout << "File was found but has zero length" << endl;
                        zeroByteCount++;
                    }
                }
            }

            frm_pos_map_t posMap;
            p->QueryPositionMap(posMap, MARK_GOP_BYFRAME);
            if (posMap.isEmpty())
                p->QueryPositionMap(posMap, MARK_GOP_START);
                if (posMap.isEmpty())
                    p->QueryPositionMap(posMap, MARK_KEYFRAME);

            if (posMap.isEmpty())
            {
                cout << "No seektable found" << endl;

                missingSeetableCount++;

                if (foundFile && fixSeektable)
                {
                    QString command = QString("mythcommflag --rebuild --chanid %1 --starttime %2")
                                              .arg(p->GetChanID())
                                              .arg(p->GetRecordingStartTime(MythDate::ISODate));
                    cout << "Running - " << qPrintable(command) << endl;
                    QScopedPointer<MythSystem> cmd(MythSystem::Create(command));
                    cmd->Wait(0);
                }
            }

            cout << "-------------------------------------------------------------------" << endl;
        }
    }

    cout << endl << endl << "SUMMARY" << endl;
    cout << "Recordings           : " << recordingCount << endl;
    cout << "Missing Recordings   : " << missingFileCount << endl;
    cout << "Zero byte Recordings : " << zeroByteCount << endl;
    cout << "Missing Seektables   : " << missingSeetableCount << endl;

    return GENERIC_EXIT_OK;
}

void registerRecordingUtils(UtilMap &utilMap)
{
    utilMap["checkrecordings"]         = &CheckRecordings;
}

/* vim: set expandtab tabstop=4 shiftwidth=4: */
