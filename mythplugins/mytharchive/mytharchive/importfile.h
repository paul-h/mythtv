#ifndef IMPORTFILE_H_
#define IMPORTFILE_H_

// c++
#include <vector>

// qt
#include <QList>
#include <QStringList>

// mythtv
#include <libmythui/mythscreentype.h>
#include <libmythbase/mythdate.h>

// mytharchive
#include "archiveutil.h"

class ProgramInfo;
class MetadataLookup;
class MythUIText;
class MythUIButton;
class MythUIButtonList;
class MythUIButtonListItem;

class ImportFile : public MythScreenType
{

  Q_OBJECT

  public:
    ImportFile(MythScreenStack *parent);

    ~ImportFile(void);

    bool Create() override; // MythScreenType
    bool keyPressEvent(QKeyEvent *e) override; // MythScreenType

   signals:
    void haveResult(bool ok);

  public slots:
    void OKPressed(void);
    void cancelPressed(void);

    void showMenu(void);
    void selectAll(void);
    void clearAll(void);

    void setCategory(MythUIButtonListItem *item);
    void titleChanged(MythUIButtonListItem *item);
    void toggleSelected(MythUIButtonListItem *item);

    void playFile(void);
    void recordFile(void);
    void editFileMetadata(void);
    void editorClosed(bool ok, ImportItem *item);

  private:
    void Init(void) override; // MythScreenType
    void updateRecordingList(void);
    void updateSelectedList(void);
    void updateCategorySelector(void);
    void getRecordingList(void);
    QString decodeXML(const QString &str);
    int decodeDuration(const QString &str);
    bool doPlayFile(ImportItem *importItem, bool startPlayback);
    void doGetRecording(void);
    void createConfigFile(const QString &filename);

    QList<ImportItem *> m_recordingList;
    QList<ImportItem *> m_selectedList;
    QStringList         m_categories;

    MythUIButtonList   *m_recordingButtonList;
    MythUIButton       *m_okButton;
    MythUIButton       *m_cancelButton;
    MythUIButtonList   *m_categorySelector;
    MythUIText         *m_titleText;
    MythUIText         *m_datetimeText;
    MythUIText         *m_filesizeText;
    MythUIText         *m_durationText;
    MythUIText         *m_filenameText;
    MythUIText         *m_descriptionText;
    MythUIImage        *m_previewImage;

    friend class GetImportRecordingListThread;
    friend class GetImportRecordingThread;
};

#endif
