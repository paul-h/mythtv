
// myth
#include <libmyth/mythcontext.h>
#include <libmythui/mythuitext.h>
#include <libmythui/mythuibutton.h>
#include <libmythui/mythuitextedit.h>
#include <libmythui/mythuispinbox.h>

// mytharchive
#include "importeditmetadata.h"

ImportEditMetadataDialog::ImportEditMetadataDialog(
    MythScreenStack *parent, ImportItem *sourceImportItem) :
    MythScreenType(parent, "ImportEditMetadataDialog"),
    m_sourceImportItem(sourceImportItem),
    m_titleEdit(nullptr),
    m_subtitleEdit(nullptr),
    m_descriptionEdit(nullptr),
    m_starttimeEdit(nullptr),
    m_categoryEdit(nullptr),
    m_seasonSpin(nullptr),
    m_episodeSpin(nullptr),
    m_yearSpin(nullptr),
    m_cancelButton(nullptr),
    m_okButton(nullptr)
{
}

bool ImportEditMetadataDialog::Create(void)
{
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("mytharchive-ui.xml", "import_editmetadata", this);

    if (!foundtheme)
        return false;

    bool err = false;
    UIUtilE::Assign(this, m_importTypeSelector, "importtype_selector", &err);
    UIUtilE::Assign(this, m_titleEdit, "title_edit", &err);
    UIUtilE::Assign(this, m_subtitleEdit, "subtitle_edit", &err);
    UIUtilE::Assign(this, m_descriptionEdit, "description_edit", &err);
    UIUtilE::Assign(this, m_starttimeEdit, "starttime_edit", &err);
    UIUtilE::Assign(this, m_categoryEdit, "category_edit", &err);
    UIUtilE::Assign(this, m_seasonSpin, "season_spin", &err);
    UIUtilE::Assign(this, m_episodeSpin, "episode_spin", &err);
    UIUtilE::Assign(this, m_yearSpin, "year_spin", &err);
    UIUtilE::Assign(this, m_okButton, "ok_button", &err);
    UIUtilE::Assign(this, m_cancelButton, "cancel_button", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'edit_metadata'");
        return false;
    }

    connect(m_okButton, SIGNAL(Clicked()), this, SLOT(okPressed()));
    connect(m_cancelButton, SIGNAL(Clicked()), this, SLOT(cancelPressed()));

    new MythUIButtonListItem(m_importTypeSelector, tr("IP Encoder"),    QString("IPEncoder"));
    new MythUIButtonListItem(m_importTypeSelector, tr("HD-PVR2"),       QString("HDPVR2"));
    new MythUIButtonListItem(m_importTypeSelector, tr("Intensity Pro"), QString("IntensityPro"));
    new MythUIButtonListItem(m_importTypeSelector, tr("Magewell"),      QString("Magewell"));
    m_importTypeSelector->SetValueByData(m_sourceImportItem->type);

    m_titleEdit->SetText(m_sourceImportItem->title);
    m_subtitleEdit->SetText(m_sourceImportItem->subtitle);
    m_descriptionEdit->SetText(m_sourceImportItem->description);
    m_starttimeEdit->SetText(m_sourceImportItem->startTime.toString(Qt::ISODate));
    m_categoryEdit->SetText(m_sourceImportItem->category);

    m_seasonSpin->SetRange(0, 999, 1);
    m_seasonSpin->SetValue(m_sourceImportItem->season);

    m_episodeSpin->SetRange(0, 999, 1);
    m_episodeSpin->SetValue(m_sourceImportItem->episode);

    m_yearSpin->SetRange(0, 9999, 1, 100);
    m_yearSpin->SetValue(m_sourceImportItem->year);

    BuildFocusList();

    SetFocusWidget(m_titleEdit);

    return true;
}

bool ImportEditMetadataDialog::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget()->keyPressEvent(event))
        return true;

    if (MythScreenType::keyPressEvent(event))
        return true;

    return false;
}

void ImportEditMetadataDialog::okPressed(void)
{
    m_sourceImportItem->type = m_importTypeSelector->GetDataValue().toString();
    m_sourceImportItem->title = m_titleEdit->GetText();
    m_sourceImportItem->subtitle = m_subtitleEdit->GetText();
    m_sourceImportItem->startTime = QDateTime::fromString(m_starttimeEdit->GetText(), Qt::ISODate);
    m_sourceImportItem->category = m_categoryEdit->GetText();
    m_sourceImportItem->description = m_descriptionEdit->GetText();
    m_sourceImportItem->season = m_seasonSpin->GetIntValue();
    m_sourceImportItem->episode = m_episodeSpin->GetIntValue();
    m_sourceImportItem->year = m_yearSpin->GetIntValue();

    emit haveResult(true, m_sourceImportItem);
    Close();
}

void ImportEditMetadataDialog::cancelPressed(void)
{
    emit haveResult(false, m_sourceImportItem);
    Close();
}

ImportEditMetadataDialog::~ImportEditMetadataDialog()
{
}
