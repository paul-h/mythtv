#ifndef IMPORTEDITMETADATA_H_
#define IMPORTEDITMETADATA_H_

// qt
#include <QKeyEvent>

// myth
#include <libmythui/mythscreentype.h>

// mytharchive
#include "archiveutil.h"

class MythUITextEdit;
class MythUIButton;

class ImportEditMetadataDialog : public MythScreenType
{

  Q_OBJECT

  public:

    ImportEditMetadataDialog(MythScreenStack *parent, ImportItem *sourceImportItem);
    ~ImportEditMetadataDialog();

    bool Create(void) override; // MythScreenType
    bool keyPressEvent(QKeyEvent *e) override; // MythScreenType

  signals:
    void haveResult(bool ok, ImportItem *item);

  public slots:

    void okPressed(void);
    void cancelPressed(void);
  private:
    ImportItem *m_sourceImportItem;

    MythUIButtonList *m_importTypeSelector;
    MythUITextEdit   *m_titleEdit;
    MythUITextEdit   *m_subtitleEdit;
    MythUITextEdit   *m_descriptionEdit;
    MythUITextEdit   *m_starttimeEdit;
    MythUITextEdit   *m_categoryEdit;
    MythUISpinBox    *m_seasonSpin;
    MythUISpinBox    *m_episodeSpin;
    MythUISpinBox    *m_yearSpin;

    MythUIButton *m_cancelButton;
    MythUIButton *m_okButton;
};

#endif
