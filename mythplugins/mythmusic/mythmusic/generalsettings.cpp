// Qt
#include <QString>
#include <QDir>

// MythTV
#include <mythcorecontext.h>
#include <mythdialogbox.h>
#include <mythuifilebrowser.h>

// MythMusic
#include "musicplayer.h"

#include "generalsettings.h"

GeneralSettings::GeneralSettings(MythScreenStack *parent, const char *name)
        : MythScreenType(parent, name),
        m_musicLocation(NULL), m_musicAudioDevice(NULL),
        m_musicDefaultUpmix(NULL), m_musicCDDevice(NULL),
        m_nonID3FileNameFormat(NULL), m_ignoreID3Tags(NULL),
        m_tagEncoding(NULL), m_allowTagWriting(NULL),
        m_saveButton(NULL), m_cancelButton(NULL)
{
}

GeneralSettings::~GeneralSettings()
{

}

bool GeneralSettings::Create()
{
    bool err = false;

    // Load the theme for this screen
    if (!LoadWindowFromXML("musicsettings-ui.xml", "generalsettings", this))
        return false;

    UIUtilE::Assign(this, m_musicLocation, "musiclocation", &err);
    UIUtilW::Assign(this, m_findLocation, "findlocation", &err);
    UIUtilE::Assign(this, m_musicAudioDevice, "musicaudiodevice", &err);
    UIUtilE::Assign(this, m_musicDefaultUpmix, "musicdefaultupmix", &err);
    UIUtilE::Assign(this, m_musicCDDevice, "musiccddevice", &err);
    UIUtilE::Assign(this, m_nonID3FileNameFormat, "nonid3filenameformat", &err);
    UIUtilE::Assign(this, m_ignoreID3Tags, "ignoreid3tags", &err);
    UIUtilE::Assign(this, m_tagEncoding, "tagencoding", &err);
    UIUtilE::Assign(this, m_allowTagWriting, "allowtagwriting", &err);
    UIUtilW::Assign(this, m_resetDBButton, "resetdatabase", &err);
    UIUtilE::Assign(this, m_saveButton, "save", &err);
    UIUtilE::Assign(this, m_cancelButton, "cancel", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'generalsettings'");
        return false;
    }

    m_musicLocation->SetText(gCoreContext->GetSetting("MusicLocation"));
    m_musicAudioDevice->SetText(gCoreContext->GetSetting("MusicAudioDevice"));

    int loadMusicDefaultUpmix = gCoreContext->GetNumSetting("MusicDefaultUpmix", 0);
    if (loadMusicDefaultUpmix == 1)
        m_musicDefaultUpmix->SetCheckState(MythUIStateType::Full);

    m_musicCDDevice->SetText(gCoreContext->GetSetting("CDDevice"));

    m_nonID3FileNameFormat->SetText(gCoreContext->GetSetting("NonID3FileNameFormat"));

    int loadIgnoreTags = gCoreContext->GetNumSetting("Ignore_ID3", 0);
    if (loadIgnoreTags == 1)
        m_ignoreID3Tags->SetCheckState(MythUIStateType::Full);

    new MythUIButtonListItem(m_tagEncoding, tr("UTF-16"), qVariantFromValue(QString("utf16")));
    new MythUIButtonListItem(m_tagEncoding, tr("UTF-8"), qVariantFromValue(QString("utf8")));
    new MythUIButtonListItem(m_tagEncoding, tr("ASCII"), qVariantFromValue(QString("ascii")));
    m_tagEncoding->SetValueByData(gCoreContext->GetSetting("MusicTagEncoding"));

    int allowTagWriting = gCoreContext->GetNumSetting("AllowTagWriting", 1);
    if (allowTagWriting == 1)
        m_allowTagWriting->SetCheckState(MythUIStateType::Full);

    if (m_findLocation)
        connect(m_findLocation, SIGNAL(Clicked()), this, SLOT(slotLocationPressed()));

    if (m_resetDBButton)
        connect(m_resetDBButton, SIGNAL(Clicked()), this, SLOT(slotResetDB()));

    connect(m_saveButton, SIGNAL(Clicked()), this, SLOT(slotSave()));
    connect(m_cancelButton, SIGNAL(Clicked()), this, SLOT(Close()));

    m_musicLocation->SetHelpText(tr("This directory must exist, and the user "
                 "running MythMusic needs to have write permission "
                 "to the directory."));
    m_musicAudioDevice->SetHelpText(tr("Audio Device used for playback. 'default' "
                 "will use the device specified in MythTV"));
    m_musicDefaultUpmix->SetHelpText(tr("MythTV can upconvert stereo tracks to 5.1 audio. "
                 "Set this option to enable it by default. "
                 "You can enable or disable the upconversion "
                 "during playback at anytime."));
    m_musicCDDevice->SetHelpText(tr("CD-ROM device used for ripping/playback."));
    m_nonID3FileNameFormat->SetHelpText(tr("Directory and filename format used to grab "
                 "information if no ID3 information is found. Accepts "
                 "GENRE, ARTIST, ALBUM, TITLE, ARTIST_TITLE and "
                 "TRACK_TITLE."));
    m_ignoreID3Tags->SetHelpText(tr("If set, MythMusic will skip checking ID3 tags "
                 "in files and just try to determine Genre, Artist, "
                 "Album, and Track number and title from the "
                 "filename."));
    m_tagEncoding->SetHelpText(tr("Some MP3 players don't understand tags encoded in UTF-8 "
                 "or UTF-16, this setting allows you to change the encoding "
                 "format used. Currently applies only to ID3 tags."));
    m_allowTagWriting->SetHelpText(tr("If set, MythMusic will be allowed to update the "
                 "metadata in the file (e.g. ID3) to match the "
                 "database. This means allowing MythTV to write "
                 "to the file and permissions must be set "
                 "accordingly. Features such as ID3 playcounts "
                 "and ratings depend on this being enabled."));
    m_resetDBButton->SetHelpText(tr("This will clear all the MythMusic database tables allowing "
                 "for a fresh start.\nNOTE: You may lose any manual or automatic changes made to "
                 "a tracks metadata like rating or playcount unless you told MythMusic to "
                 "write those to the tag."));
    m_cancelButton->SetHelpText(tr("Exit without saving settings"));
    m_saveButton->SetHelpText(tr("Save settings and Exit"));

    BuildFocusList();

    SetFocusWidget(m_musicLocation);

    return true;
}

void GeneralSettings::slotResetDB(void)
{
    ShowOkPopup(tr("Are you sure you want to reset the music database?"),
                this, SLOT(slotDoResetDB(bool)), true);
}

void GeneralSettings::slotDoResetDB(bool ok)
{
    if (ok)
    {
        gPlayer->stop(true);

        MSqlQuery query(MSqlQuery::InitCon());

        query.prepare("TRUNCATE music_albumart");
        query.exec();
        query.prepare("TRUNCATE music_albums");
        query.exec();
        query.prepare("TRUNCATE music_artists");
        query.exec();
        query.prepare("TRUNCATE music_directories");
        query.exec();
        query.prepare("TRUNCATE music_genres");
        query.exec();
        query.prepare("TRUNCATE music_playlists");
        query.exec();
        query.prepare("TRUNCATE music_songs");
        query.exec();
        query.prepare("TRUNCATE music_stats");
        query.exec();

        gPlayer->stopCDWatcher();
        delete gMusicData;
        gMusicData = new MusicData;
        gPlayer->startCDWatcher();

        ShowOkPopup(tr("Music database has been cleared.\n"
                      "You must now scan, rip or import some tracks."));
    }
}

void GeneralSettings::slotLocationPressed(void)
{
    // need to remove the trailing '/' or else the file browser gets confused
    QString location = m_musicLocation->GetText();
    if (location.endsWith('/'))
        location.chop(1);

    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");
    MythUIFileBrowser *fb = new MythUIFileBrowser(popupStack, location);

    fb->SetTypeFilter(QDir::AllDirs | QDir::Readable);

    if (fb->Create())
    {
        fb->SetReturnEvent(this, "locationchange");
        popupStack->AddScreen(fb);
    }
    else
        delete fb;
}

void GeneralSettings::slotSave(void)
{
    // get the starting directory from the settings and remove all multiple
    // directory separators "/" and resolves any "." or ".." in the path.
    QString dir = m_musicLocation->GetText();

    if (!dir.isEmpty())
    {
        dir = QDir::cleanPath(dir);
        if (!dir.endsWith("/"))
            dir += "/";
    }

    gCoreContext->SaveSetting("MusicLocation", dir);
    gCoreContext->SaveSetting("CDDevice", m_musicCDDevice->GetText());
    gCoreContext->SaveSetting("MusicAudioDevice", m_musicAudioDevice->GetText());
    gCoreContext->SaveSetting("NonID3FileNameFormat", m_nonID3FileNameFormat->GetText());
    gCoreContext->SaveSetting("MusicTagEncoding", m_tagEncoding->GetValue());

    int saveMusicDefaultUpmix = (m_musicDefaultUpmix->GetCheckState() == MythUIStateType::Full) ? 1 : 0;
    gCoreContext->SaveSetting("MusicDefaultUpmix", saveMusicDefaultUpmix);

    int saveIgnoreTags = (m_ignoreID3Tags->GetCheckState() == MythUIStateType::Full) ? 1 : 0;
    gCoreContext->SaveSetting("Ignore_ID3", saveIgnoreTags);

    int allowTagWriting = (m_allowTagWriting->GetCheckState() == MythUIStateType::Full) ? 1 : 0;
    gCoreContext->SaveSetting("AllowTagWriting", allowTagWriting);

    gCoreContext->dispatch(MythEvent(QString("MUSIC_SETTINGS_CHANGED GENERAL_SETTINGS")));

    Close();
}

void GeneralSettings::customEvent(QEvent *event)
{
    if (event->type() == DialogCompletionEvent::kEventType)
    {
        DialogCompletionEvent *dce = (DialogCompletionEvent*)(event);
        if (dce->GetId() == "locationchange")
            m_musicLocation->SetText(dce->GetResultText());
    }
}

