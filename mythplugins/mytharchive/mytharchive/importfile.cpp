
// c
#include <cstdlib>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>

// qt
#include <QDir>
#include <QKeyEvent>
#include <QTimer>
#include <QApplication>
#include <QDomElement>

// mythtv
#include <mythcontext.h>
#include <mythdb.h>
#include <mthread.h>
#include <programinfo.h>
#include <remoteutil.h>
#include <remotefile.h>
#include <mythtimer.h>
#include <mythuitext.h>
#include <mythuibutton.h>
#include <mythuiimage.h>
#include <mythuibuttonlist.h>
#include <mythmainwindow.h>
#include <mythprogressdialog.h>
#include <mythdialogbox.h>
#include <mythlogging.h>
#include <mythdate.h>
#include <mythsystem.h>
#include <mythdirs.h>
#include <exitcodes.h>
#include <mythsystemlegacy.h>
#include <metadata/metadatacommon.h>

// mytharchive
#include "importfile.h"
#include "importeditmetadata.h"
#include "archiveutil.h"
#include "logviewer.h"

#define FPS 25
#define STREAMURL "http://192.168.1.168:80/hdmi"

class GetImportRecordingListThread : public MThread
{
  public:
    GetImportRecordingListThread(ImportFile *parent) :
        MThread("GetImportRecordingList"), m_parent(parent)
    {
        QString command = gCoreContext->GetSetting("MythArchiveGetRecordingList");
        start();
    }

    virtual void run(void) override // MThread
    {
        RunProlog();
        m_parent->getRecordingList();
        RunEpilog();
    }

    ImportFile *m_parent;
};

class GetImportRecordingThread : public MThread
{
  public:
    GetImportRecordingThread(ImportFile *parent) :
        MThread("GetRecording"), m_parent(parent)
    {
        start();
    }

    virtual void run(void) override // MThread
    {
        RunProlog();
        m_parent->doGetRecording();
        RunEpilog();
    }

    ImportFile *m_parent;
};

ImportFile::ImportFile(
    MythScreenStack *parent) :
    MythScreenType(parent, "ImportFile"),
    m_recordingButtonList(nullptr),
    m_okButton(nullptr),
    m_cancelButton(nullptr),
    m_categorySelector(nullptr),
    m_titleText(nullptr),
    m_datetimeText(nullptr),
    m_filesizeText(nullptr),
    m_durationText(nullptr),
    m_descriptionText(nullptr),
    m_previewImage(nullptr)
{
}

ImportFile::~ImportFile(void)
{
    while (!m_recordingList.isEmpty())
        delete m_recordingList.takeFirst();
}

bool ImportFile::Create(void)
{
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("mytharchive-ui.xml", "import_file", this);

    if (!foundtheme)
        return false;

    bool err = false;
    UIUtilE::Assign(this, m_okButton, "ok_button", &err);
    UIUtilE::Assign(this, m_cancelButton, "cancel_button", &err);
    UIUtilE::Assign(this, m_categorySelector, "category_selector", &err);
    UIUtilE::Assign(this, m_recordingButtonList, "recordinglist", &err);

    UIUtilW::Assign(this, m_titleText, "progtitle", &err);
    UIUtilW::Assign(this, m_datetimeText, "progdatetime", &err);
    UIUtilW::Assign(this, m_descriptionText, "progdescription", &err);
    UIUtilW::Assign(this, m_filesizeText, "filesize", &err);
    UIUtilW::Assign(this, m_filenameText, "filename", &err);
    UIUtilW::Assign(this, m_previewImage, "preview_image", &err);
    UIUtilW::Assign(this, m_durationText, "duration", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'import_file'");
        return false;
    }

    connect(m_okButton, SIGNAL(Clicked()), this, SLOT(OKPressed()));
    connect(m_cancelButton, SIGNAL(Clicked()), this, SLOT(cancelPressed()));

    new MythUIButtonListItem(m_categorySelector, tr("All Recordings"));
    connect(m_categorySelector, SIGNAL(itemSelected(MythUIButtonListItem *)),
            this, SLOT(setCategory(MythUIButtonListItem *)));

    connect(m_recordingButtonList, SIGNAL(itemSelected(MythUIButtonListItem *)),
            this, SLOT(titleChanged(MythUIButtonListItem *)));
    connect(m_recordingButtonList, SIGNAL(itemClicked(MythUIButtonListItem *)),
            this, SLOT(toggleSelected(MythUIButtonListItem *)));

    BuildFocusList();

    SetFocusWidget(m_recordingButtonList);

    return true;
}

void ImportFile::Init(void)
{
    QString message = tr("Retrieving Recording List. Please Wait...");

    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");

    MythUIBusyDialog *busyPopup = new
            MythUIBusyDialog(message, popupStack, "getrecordingbusydialog");

    if (busyPopup->Create())
        popupStack->AddScreen(busyPopup, false);
    else
    {
        delete busyPopup;
        busyPopup = NULL;
    }

    GetImportRecordingListThread *thread = new GetImportRecordingListThread(this);

    while (thread->isRunning())
    {
        qApp->processEvents();
        usleep(2000);
    }

    if (m_recordingList.empty())
    {
        ShowOkPopup(tr("Failed to get recording list!"));
        if (busyPopup)
            busyPopup->Close();

        Close();
        return;
    }

    updateCategorySelector();
    updateSelectedList();
    updateRecordingList();

    if (busyPopup)
        busyPopup->Close();
}

bool ImportFile::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget()->keyPressEvent(event))
        return true;

    bool handled = false;
    QStringList actions;
    handled = GetMythMainWindow()->TranslateKeyPress("Archive", event, actions);

    for (int i = 0; i < actions.size() && !handled; i++)
    {
        QString action = actions[i];
        handled = true;

        if (action == "MENU")
        {
            showMenu();
        }
        else if (action == "PLAY")
        {
            playFile();
        }
        else if (action == "RECORD")
        {
            recordFile();
        }
        else if (action == "EDIT")
        {
            editFileMetadata();
        }
        else if (action == "INFO")
        {
            Init();
        }
        else
            handled = false;
    }

    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}

void ImportFile::showMenu()
{
    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");

    MythDialogBox *menuPopup = new MythDialogBox(tr("Menu"), popupStack, "actionmenu");

    if (menuPopup->Create())
        popupStack->AddScreen(menuPopup);

    menuPopup->SetReturnEvent(this, "action");

    menuPopup->AddButton(tr("Clear All"), &ImportFile::clearAll);
    menuPopup->AddButton(tr("Select All"), &ImportFile::selectAll);
}

void ImportFile::selectAll()
{

}

void ImportFile::clearAll()
{
    while (!m_selectedList.isEmpty())
         m_selectedList.takeFirst();
    m_selectedList.clear();

    updateRecordingList();
}

void ImportFile::toggleSelected(MythUIButtonListItem *item)
{
    if (item->state() == MythUIButtonListItem:: FullChecked)
    {
        int index = m_selectedList.indexOf(item->GetData().value<ImportItem *>());
        if (index != -1)
            m_selectedList.takeAt(index);
        item->setChecked(MythUIButtonListItem:: NotChecked);
    }
    else
    {
        int index = m_selectedList.indexOf(item->GetData().value<ImportItem *>());
        if (index == -1)
            m_selectedList.append(item->GetData().value<ImportItem *>());

        item->setChecked(MythUIButtonListItem::FullChecked);
    }
}

void ImportFile::titleChanged(MythUIButtonListItem *item)
{
    ImportItem *i;

    i = item->GetData().value<ImportItem *>();

    if (!i)
        return;

    if (m_titleText)
        m_titleText->SetText(i->title);

    if (m_datetimeText)
        m_datetimeText->SetText(MythDate::toString(i->startTime, MythDate::kDateTimeFull + MythDate::kAutoYear));

    if (m_descriptionText)
        m_descriptionText->SetText(i->description);

    if (m_filesizeText)
    {
        if (i->size > 0)
            m_filesizeText->SetText(formatSize(i->size / 1024));
        else
            m_filesizeText->SetText(tr("N/A"));
    }

    if (m_durationText)
    {
        if (i->actualDuration > 0)
            m_durationText->SetText(formatTime(i->actualDuration));
        else
            m_durationText->SetText(tr("N/A"));
    }

    if (m_filenameText)
        m_filenameText->SetText(i->filename);
}

void ImportFile::OKPressed()
{
    QString configFile = getTempDirectory() + "config/importjob.xml";

    createConfigFile(configFile);

    QString commandline;
    QString tempDir = gCoreContext->GetSetting("MythArchiveTempDir", "");

    if (tempDir == "")
        return;

    if (!tempDir.endsWith("/"))
        tempDir += "/";

    QString logDir = tempDir + "logs";

    // remove any existing logs
    myth_system("rm -f " + logDir + "/*.log");

    commandline = "mytharchivehelper -v none,jobqueue --logpath " + logDir + " --importfile "
                  "--infile \"" + configFile + "\"";
    uint flags = kMSRunBackground | kMSDontBlockInputDevs |
                 kMSDontDisableDrawing;
    uint retval = myth_system(commandline, flags);
    if (retval != GENERIC_EXIT_RUNNING && retval != GENERIC_EXIT_OK)
    {
        ShowOkPopup(tr("It was not possible to import the file. "
                       " An error occured when running 'mytharchivehelper'") );
        return;
    }

    showLogViewer();

    Close();
}

void ImportFile::cancelPressed()
{
    emit haveResult(false);
    Close();
}

void ImportFile::createConfigFile(const QString &filename)
{
    QDomDocument doc("mythimportjob");

    QDomElement root = doc.createElement("mythimport");
    doc.appendChild(root);

    // now loop though selected items and add them to the xml file
    ImportItem *i;
    for (int x = 0; x < m_selectedList.count(); x++)
    {
        i = m_selectedList.at(x);
        if (!i)
            continue;

        QDomElement job = doc.createElement("job");
        root.appendChild(job);

        QDomElement id = doc.createElement("id");
        id.appendChild(doc.createTextNode(QString("%1").arg(i->id)));
        job.appendChild(id);

        QDomElement type = doc.createElement("type");
        type.appendChild(doc.createTextNode(i->type));
        job.appendChild(type);

        QDomElement title = doc.createElement("title");
        title.appendChild(doc.createTextNode(i->title));
        job.appendChild(title);

        QDomElement subtitle = doc.createElement("subtitle");
        subtitle.appendChild(doc.createTextNode(i->subtitle));
        job.appendChild(subtitle);

        QDomElement description = doc.createElement("description");
        description.appendChild(doc.createTextNode(i->description));
        job.appendChild(description);

        QDomElement startTime = doc.createElement("starttime");
        startTime.appendChild(doc.createTextNode(i->startTime.toString(Qt::ISODate)));
        job.appendChild(startTime);

        QDomElement season = doc.createElement("season");
        season.appendChild(doc.createTextNode(QString("%1").arg(i->season)));
        job.appendChild(season);

        QDomElement episode = doc.createElement("episode");
        episode.appendChild(doc.createTextNode(QString("%1").arg(i->episode)));
        job.appendChild(episode);

        QDomElement year = doc.createElement("year");
        year.appendChild(doc.createTextNode(QString("%1").arg(i->year)));
        job.appendChild(year);

        QDomElement certification = doc.createElement("certification");
        certification.appendChild(doc.createTextNode(i->certification));
        job.appendChild(certification);

        QDomElement chanNo = doc.createElement("channo");
        chanNo.appendChild(doc.createTextNode(i->chanNo));
        job.appendChild(chanNo);

        QDomElement chanSign = doc.createElement("chansign");
        chanSign.appendChild(doc.createTextNode(i->chanSign));
        job.appendChild(chanSign);

        QDomElement chanName = doc.createElement("channame");
        chanName.appendChild(doc.createTextNode(i->chanName));
        job.appendChild(chanName);

        QDomElement status = doc.createElement("status");
        status.appendChild(doc.createTextNode(i->status));
        job.appendChild(status);

        QDomElement filename = doc.createElement("filename");
        filename.appendChild(doc.createTextNode(i->filename));
        job.appendChild(filename);

        QDomElement category = doc.createElement("category");
        category.appendChild(doc.createTextNode(i->category));
        job.appendChild(category);

        QDomElement size = doc.createElement("size");
        size.appendChild(doc.createTextNode(QString("%1").arg(i->size)));
        job.appendChild(size);

        QDomElement duration = doc.createElement("duration");
        duration.appendChild(doc.createTextNode(QString("%1").arg(i->duration)));
        job.appendChild(duration);

        QDomElement actualDuration = doc.createElement("actualduration");
        actualDuration.appendChild(doc.createTextNode(QString("%1").arg(i->actualDuration)));
        job.appendChild(actualDuration);
    }

    // finally save the xml to the file
    QFile f(filename);
    if (!f.open(QIODevice::WriteOnly))
    {
        LOG(VB_GENERAL, LOG_ERR,
            QString("ImportFile::createConfigFile: "
                    "Failed to open file for writing - %1") .arg(filename));
        return;
    }

    QTextStream t(&f);
    t << doc.toString(4);
    f.close();
}

void ImportFile::updateRecordingList(void)
{
    if (m_recordingList.empty())
        return;

    m_recordingButtonList->Reset();

    if (m_categorySelector)
    {
        ImportItem *i;
        for (int x = 0; x < m_recordingList.count(); x++)
        {
            i = m_recordingList.at(x);

            if (i->category == m_categorySelector->GetValue() ||
                m_categorySelector->GetValue() == tr("All Recordings"))
            {
                MythUIButtonListItem* item = new MythUIButtonListItem(
                    m_recordingButtonList,
                    i->title + " ~ " + MythDate::toString(i->startTime, MythDate::kDateTimeFull + MythDate::kAutoYear));
                item->setCheckable(true);

                if (m_selectedList.indexOf((ImportItem *) i) != -1)
                {
                    item->setChecked(MythUIButtonListItem::FullChecked);
                }
                else
                {
                    item->setChecked(MythUIButtonListItem::NotChecked);
                }

                item->SetData(qVariantFromValue(i));
            }
            qApp->processEvents();
        }
    }

    m_recordingButtonList->SetItemCurrent(m_recordingButtonList->GetItemFirst());
    titleChanged(m_recordingButtonList->GetItemCurrent());
}

void ImportFile::getRecordingList(void)
{
    while (!m_recordingList.isEmpty())
        delete m_recordingList.takeFirst();

    QString xmlFile = getTempDirectory() + "work/recordings.xml";
    QFileInfo fi(xmlFile);

    if (!fi.exists() || QDateTime::currentDateTime() > fi.lastModified().addSecs(3600))
    {
        // run the script to get list of recordings from the External Box
        QString command = gCoreContext->GetSetting("MythArchiveGetRecordingList");
        command.replace("%XMLFILE%", xmlFile);

        QScopedPointer<MythSystem> cmd(MythSystem::Create(command));
        cmd->Wait(0s);
        if (cmd.data()->GetExitCode() != GENERIC_EXIT_OK)
        {
            LOG(VB_GENERAL, LOG_ERR, QString("ERROR - Failed to get recording list"));
            LOG(VB_GENERAL, LOG_ERR, QString("Command exited with result: %1").arg(cmd.data()->GetExitCode()));
            LOG(VB_GENERAL, LOG_ERR, QString("Command was: %1").arg(command));
            //return;
        }
    }

    // load the xml file contains the recordings list details
    QDomDocument doc("recordings");
    QFile file(xmlFile);
    if (!file.open(QIODevice::ReadOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, "Could not open recordings file: " + xmlFile);
        return;
    }

    if (!doc.setContent(&file))
    {
        LOG(VB_GENERAL, LOG_ERR, "Could not load recordings file: " + xmlFile);
        file.close();
        return;
    }

    file.close();

    QDomNodeList recordingNodes = doc.elementsByTagName("recording");
    for (int x = 0; x < recordingNodes.count(); x++)
    {
        ImportItem *importItem = new ImportItem;
        QDomNode recordingNode = recordingNodes.item(x);
        QDomElement recordingElement = recordingNode.toElement();
        for (int y = 0; y < recordingElement.childNodes().count(); y++)
        {
            QDomNode node = recordingElement.childNodes().at(y);
            QDomElement e = node.toElement();
            if (e.nodeName() == "no")
                importItem->id = getFirstText(e).toInt();
            else if (e.nodeName() == "title")
                importItem->title = decodeXML(getFirstText(e));
            else if (e.nodeName() == "season")
                importItem->season = decodeXML(getFirstText(e)).toUInt();
            else if (e.nodeName() == "episode")
                importItem->episode = decodeXML(getFirstText(e)).toUInt();
            else if (e.nodeName() == "channelno")
                importItem->chanNo = decodeXML(getFirstText(e));
            else if (e.nodeName() == "channelsign")
                importItem->chanSign = decodeXML(getFirstText(e));
            else if (e.nodeName() == "channelname")
                importItem->chanName = decodeXML(getFirstText(e));
            else if (e.nodeName() == "starttime")
                importItem->startTime = MythDate::fromString(getFirstText(e));
            else if (e.nodeName() == "duration")
                importItem->duration = decodeDuration(getFirstText(e));
            else if (e.nodeName() == "actualduration")
                importItem->actualDuration = decodeDuration(getFirstText(e));
            else if (e.nodeName() == "description")
                importItem->description = decodeXML(getFirstText(e)) + QString("\n\nRecorded from %1 on %2 at %3.")
                    .arg(importItem->chanName).arg(importItem->startTime.toString("dddd dd MMMM yyyy"))
                    .arg(importItem->startTime.toString("hh:mm ap"));
            else if (e.nodeName() == "filesize")
                importItem->size = getFirstText(e).toInt();
            else if (e.nodeName() == "filename")
                importItem->filename = getFirstText(e);
            else if (e.nodeName() == "category")
                importItem->category = getFirstText(e);
        }

        if (importItem->chanSign.isEmpty())
            importItem->chanSign = importItem->chanName;

        if (importItem->category.isEmpty())
            importItem->category = importItem->title;

        if (m_categories.indexOf(importItem->category) == -1)
            m_categories.append(importItem->category);

        // default to using the IPEncoder
        importItem->type = "IPEncoder";

        m_recordingList.append(importItem);
    }
}

void ImportFile::updateCategorySelector(void)
{
    // sort and add categories to selector
    m_categories.sort();

    if (m_categorySelector)
    {
        new MythUIButtonListItem(m_categorySelector, tr("All Recordings"));
        m_categorySelector->SetItemCurrent(0);

        for (int x = 0; x < m_categories.count(); x++)
        {
            new MythUIButtonListItem(m_categorySelector, m_categories[x]);
        }
    }
}

void ImportFile::setCategory(MythUIButtonListItem *item)
{
    (void)item;
    updateRecordingList();
}

void ImportFile::updateSelectedList()
{
}

void ImportFile::playFile(void)
{
    MythUIButtonListItem *item = m_recordingButtonList->GetItemCurrent();

    if (!item)
        return;

    ImportItem *i = item->GetData().value<ImportItem *>();

    if (!i)
        return;

    doPlayFile(i, true);
}

bool ImportFile::doPlayFile(ImportItem *importItem, bool startPlayback)
{
    QString command = gCoreContext->GetSetting("MythArchivePlayFileCommand");
    command.replace("%FILENAME%", importItem->filename);

    QScopedPointer<MythSystem> cmd(MythSystem::Create(command));
    cmd->Wait(0s);
    if (cmd.data()->GetExitCode() != GENERIC_EXIT_OK)
    {
        LOG(VB_GENERAL, LOG_ERR, QString("ERROR - Failed to start playing file: %1").arg(importItem->filename));
        LOG(VB_GENERAL, LOG_ERR, QString("Command exited with result: %1").arg(cmd.data()->GetExitCode()));
        LOG(VB_GENERAL, LOG_ERR, QString("Command was: %1").arg(command));
        return false;
    }

    if (startPlayback)
        GetMythMainWindow()->HandleMedia("Internal", STREAMURL);

    return true;
}

void ImportFile::recordFile(void)
{
    MythUIButtonListItem *item = m_recordingButtonList->GetItemCurrent();

    if (!item)
        return;

    ImportItem *i = item->GetData().value<ImportItem *>();

    if (!i)
        return;

    if (!doPlayFile(i, false))
        return;

    QString message = tr("Getting List Of Recording.\nPlease Wait...");

    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");

    MythUIBusyDialog *busyPopup = new
            MythUIBusyDialog(message, popupStack, "recordingbusydialog");

    if (busyPopup->Create())
        popupStack->AddScreen(busyPopup, false);
    else
    {
        delete busyPopup;
        busyPopup = NULL;
    }

    GetImportRecordingThread *thread = new GetImportRecordingThread(this);
    while (thread->isRunning())
    {
        qApp->processEvents();
        usleep(2000);
    }

    if (busyPopup)
        busyPopup->Close();
}

void ImportFile::doGetRecording(void)
{
    MythUIButtonListItem *item = m_recordingButtonList->GetItemCurrent();

    if (!item)
        return;

    ImportItem *i = item->GetData().value<ImportItem *>();

    if (!i)
        return;

    uint duration = 60; //i->actualDuration;
    QString videoFile = getTempDirectory() + "work/video.ts";
    QString mxmlFile = getTempDirectory() + "work/video.mxml";

    // record the mp4 video stream
    QString recCommand = QString("mythffmpeg -y -i %1 -t %2 -acodec copy -vcodec copy %3")
                                 .arg(STREAMURL).arg(duration).arg(videoFile);

    QScopedPointer<MythSystem> cmd(MythSystem::Create(recCommand, kMSRunShell));
    cmd->Wait(0s);
    if (cmd.data()->GetExitCode() != GENERIC_EXIT_OK)
    {
        LOG(VB_GENERAL, LOG_ERR, QString("ERROR - ffmpeg exited with result: %1").arg(cmd.data()->GetExitCode()));
        return;
    }

    // create a mxml file with the metadata for this recording
    QStringList categories(i->category.split(','));
    MetadataLookup *lookup = new MetadataLookup(kMetadataVideo, kProbableTelevision, QVariant(), kLookupSearch, false, false, false, false, false,
                                                "", videoFile, i->title, categories, 0.0, i->subtitle, "", i->description, i->season, i->episode,
                                                i->startTime, 0,  i->chanNo, i->chanSign, i->chanName,
                                                i->certification, i->startTime.date().year(), i->startTime.date(), i->duration / 60, i->duration, 
                                                "", PeopleMap(), "", ArtworkMap(), DownloadMap());
    if (i->category == "Movies")
        lookup->SetVideoContentType(kContentMovie);
    else
        lookup->SetVideoContentType(kContentTelevision);

    QDomDocument mxmlDoc = CreateMetadataXML(lookup);

    // save the mxml to the file
    QFile f(mxmlFile);
    if (!f.open(QIODevice::WriteOnly))
    {
        LOG(VB_GENERAL, LOG_ERR, QString("ImportFile: Failed to open mxml file for writing - %1").arg(mxmlFile));
        return;
    }

    QTextStream t(&f);
    t << mxmlDoc.toString(4);
    f.close();

    // workout where to save the file in the Video storage group
    QString dstFile = filenameFromMetadataLookup(lookup);

    QString saveFilename;

    // copy the recording to the Video storage group
    saveFilename = gCoreContext->GenMythURL(gCoreContext->GetMasterHostName(), 0, dstFile + ".mp4", "Videos");

    bool result = RemoteFile::CopyFile(videoFile, saveFilename);
    if (!result)
    {
        LOG(VB_GENERAL, LOG_ERR, QString("ImportFile: Failed to copy video file to %1").arg(saveFilename));
        return;
    }

    // copy the metadata xml file to the Video storage group
    saveFilename = gCoreContext->GenMythURL(gCoreContext->GetMasterHostName(), 0, dstFile + ".mxml", "Videos");

    result = RemoteFile::CopyFile(mxmlFile, saveFilename);
    if (!result)
    {
        LOG(VB_GENERAL, LOG_ERR, QString("ImportFile: Failed to copy xml file to %1").arg(saveFilename));
        return;
    }
}

void ImportFile::editFileMetadata(void)
{
    MythUIButtonListItem *item = m_recordingButtonList->GetItemCurrent();

    if (!item)
        return;

    ImportItem *i = item->GetData().value<ImportItem *>();

    if (!i)
        return;

    MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();

    ImportEditMetadataDialog *editor = new ImportEditMetadataDialog(mainStack, i);

    connect(editor, SIGNAL(haveResult(bool, ImportItem *)),
            this, SLOT(editorClosed(bool, ImportItem *)));

    if (editor->Create())
        mainStack->AddScreen(editor);
}

void ImportFile::editorClosed(bool ok, ImportItem *item)
{
    MythUIButtonListItem *gridItem = m_recordingButtonList->GetItemCurrent();

    if (ok && item && gridItem)
    {
        // TODO should update the buttonlist item here
        titleChanged(gridItem);
    }
}

QString ImportFile::decodeXML(const QString &str)
{
    QString temp(str);

    temp.replace("&amp;", "&");
    temp.replace("&apos;", "'");
    temp.replace("&quot;", "\"");
    temp.replace("&lt;", "<");
    temp.replace("&gt;", ">");

    return temp;
}

int ImportFile::decodeDuration(const QString &str)
{
    int hours = str.mid(3, 2).toInt();
    int minutes = str.mid(6, 2).toInt();
    int seconds = str.mid(9, 2).toInt();

    return (hours * 3600) + (minutes * 60) + seconds;
}
