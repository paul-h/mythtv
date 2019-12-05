/*
    archiveutil.h

    some shared functions and types
*/

#ifndef ARCHIVEUTIL_H_
#define ARCHIVEUTIL_H_


#include <cstdint>

// qt
#include <QString>
#include <QMetaType>
#include <QDateTime>

class ProgramInfo;
class MetadataLookup;

enum ARCHIVEDESTINATION
{
    AD_DVD_SL = 0,
    AD_DVD_DL = 1,
    AD_DVD_RW = 2,
    AD_FILE   = 3
};

Q_DECLARE_METATYPE (ARCHIVEDESTINATION);

struct ArchiveDestination
{
    ARCHIVEDESTINATION type;
    const char *name;
    const char *description;
    int64_t freeSpace;
};

extern struct ArchiveDestination ArchiveDestinations[];
extern int ArchiveDestinationsCount;

struct EncoderProfile
{
    QString name;
    QString description;
    float bitrate;
};

struct ThumbImage
{
    QString caption;
    QString filename;
    qint64  frame;
};

struct ArchiveItem
{
    int     id;
    QString type;
    QString title;
    QString subtitle;
    QString description;
    QString startDate;
    QString startTime;
    QString filename;
    int64_t size;
    int64_t newsize;
    int duration;
    int cutDuration;
    EncoderProfile *encoderProfile;
    QString fileCodec;
    QString videoCodec;
    int videoWidth, videoHeight;
    bool hasCutlist;
    bool useCutlist;
    bool editedDetails;
    QList<ThumbImage*> thumbList;
};

class ImportItem
{
  public:
    ImportItem() : id(-1), season(0), episode(0), year(0), size(0), duration(0), actualDuration(0) {}
    int     id;
    QString type;
    QString title;
    QString subtitle;
    QString description;
    QDateTime startTime;
    uint season;
    uint episode;
    uint year;
    QString certification;
    QString chanNo;
    QString chanSign;
    QString chanName;
    QString status;
    QString filename;
    QString category;
    int64_t size;
    uint duration;
    uint actualDuration;
};

QString formatSize(int64_t sizeKB, int prec = 2);
QString formatTime(int seconds);
QString getTempDirectory(bool showError = false);
void checkTempDirectory();
bool extractDetailsFromFilename(const QString &inFile,
                                QString &chanID, QString &startTime);
ProgramInfo *getProgramInfoForFile(const QString &inFile);
bool getFileDetails(ArchiveItem *a);
void recalcItemSize(ArchiveItem *item);
QString getBaseName(const QString &filename);
void showWarningDialog(const QString &msg);

QString filenameFromMetadataLookup(MetadataLookup *lookup);

Q_DECLARE_METATYPE(EncoderProfile *)
Q_DECLARE_METATYPE(ArchiveItem *)
Q_DECLARE_METATYPE(ImportItem *)

#endif
