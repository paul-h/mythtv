#ifndef LYRICSDATA_H_
#define LYRICSDATA_H_

// C/C++
#include <cstdint>

// qt
#include <QObject>

// mythtv
#include "mythmetaexp.h"
#include "mythcorecontext.h"
#include "musicmetadata.h"

class LyricsData;

class META_PUBLIC LyricsLine
{
  public:
    LyricsLine() :
        Time(0), Lyric("") { }
    LyricsLine(int time, const QString &lyric) :
        Time(time), Lyric(lyric) { }

    int Time;
    QString Lyric;

    QString toString(bool syncronized)
    {
        if (syncronized)
            return formatTime() + Lyric;

        return Lyric;
    }

  private:
    QString formatTime(void)
    {
        QString res;
        int minutes = Time / (1000 * 60);
        int seconds = Time  % (1000 * 60) / 1000;
        int hundredths = (Time % 1000) / 10;

        res.sprintf("[%02d:%02d.%02d]", minutes, seconds, hundredths);
        return res;
    }
};

class META_PUBLIC LyricsData : public QObject
{
  Q_OBJECT

  public:
    LyricsData();
    explicit LyricsData(MusicMetadata *parent);
    LyricsData(MusicMetadata *parent, const QString &grabber, const QString &artist,
               const QString &album, const QString &title, bool syncronized);

    ~LyricsData();

    QString grabber(void) { return m_grabber; }
    void setGrabber(const QString &grabber) { m_grabber = grabber; }

    QString artist(void) { return m_artist; }
    void setArtist(const QString &artist) { m_artist = artist; }

    QString album(void) { return m_album; }
    void setAlbum(const QString &album) { m_album = album; }

    QString title(void) { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QMap<int, LyricsLine*>* lyrics(void) { return &m_lyricsMap; }
    void setLyrics(const QStringList &lyrics);

    bool syncronized(void) { return m_syncronized; }
    void setSyncronized(bool syncronized ) { m_syncronized = syncronized; }

    bool changed(void) { return m_changed; }
    void setChanged(bool changed) { m_changed = changed; }

    enum Status
    {
      STATUS_NOTLOADED = 0, // not looked for any lyrics yet
      STATUS_SEARCHING,     // search is taking place
      STATUS_FOUND,         // search completed and some lyrics have been found
      STATUS_NOTFOUND       // search completed but no lyrics have been found
    };

    Status getStatus(void) { return m_status; }

    void clear(void);
    void clearLyrics(void);
    void findLyrics(const QString &grabber);
    void save(void);

    void customEvent(QEvent *event) override; // QObject

  signals:
    void statusChanged(LyricsData::Status status, const QString &message);

  private:
    void loadLyrics(const QString &xmlData);
    QString createLyricsXML(void);

    QMap<int, LyricsLine*> m_lyricsMap;

    MusicMetadata *m_parent;

    Status m_status;

    QString m_grabber;
    QString m_artist;
    QString m_album;
    QString m_title;
    bool m_syncronized;
    bool m_changed;
};

Q_DECLARE_METATYPE(LyricsLine*)

#endif
