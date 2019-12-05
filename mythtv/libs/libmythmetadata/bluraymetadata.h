#ifndef BLURAYMETADATA_H_
#define BLURAYMETADATA_H_

#include <QList>
#include <QPair>
#include <QString>

#include "mythtypes.h"
#include "mythmetaexp.h"
#include "libbluray/bluray.h"

class QStringList;

using BlurayTitles = QList< QPair < uint,QString > >;

struct meta_dl;
class META_PUBLIC BlurayMetadata : public QObject
{
    Q_DECLARE_TR_FUNCTIONS(BlurayMetadata);

  public:
    explicit BlurayMetadata(const QString &path)
        : m_path(path) {}
    ~BlurayMetadata();

    void toMap(InfoMap &metadataMap);

    bool OpenDisc(void);
    bool IsOpen() { return m_bdnav; };
    bool ParseDisc(void);

    QString      GetTitle(void) { return m_title; };
    QString      GetAlternateTitle(void) { return m_alttitle; };
    QString      GetDiscLanguage(void) { return m_language; };

    uint         GetCurrentDiscNumber(void) { return m_discnumber; };
    uint         GetTotalDiscNumber(void) { return m_disctotal; };

    uint         GetTitleCount(void) { return m_titles.count(); };
    BlurayTitles GetTitles(void) { return m_titles; };

    uint         GetThumbnailCount(void) { return m_images.count(); };
    QStringList  GetThumbnails(void) {return m_images; };

    bool         GetTopMenuSupported(void) { return m_topMenuSupported; };
    bool         GetFirstPlaySupported(void) { return m_firstPlaySupported; };

    uint32_t     GetNumHDMVTitles(void) { return m_numHDMVTitles; };
    uint32_t     GetNumBDJTitles(void) { return m_numBDJTitles; };
    uint32_t     GetNumUnsupportedTitles(void) { return m_numUnsupportedTitles; };

    bool         GetAACSDetected(void) { return m_aacsDetected; };
    bool         GetLibAACSDetected(void) { return m_libaacsDetected; };
    bool         GetAACSHandled(void) { return m_aacsHandled; };

    bool         GetBDPlusDetected(void) { return m_bdplusDetected; };
    bool         GetLibBDPlusDetected(void) { return m_libbdplusDetected; };
    bool         GetBDPlusHandled(void) { return m_bdplusHandled; };

  private:
    BLURAY              *m_bdnav               {nullptr};

    QString              m_title;
    QString              m_alttitle;
    QString              m_language;

    uint                 m_discnumber           {0};
    uint                 m_disctotal            {0};

    QString              m_path;

    BlurayTitles         m_titles;
    QStringList          m_images;

    bool                 m_topMenuSupported     {false};
    bool                 m_firstPlaySupported   {false};
    uint32_t             m_numHDMVTitles        {0};
    uint32_t             m_numBDJTitles         {0};
    uint32_t             m_numUnsupportedTitles {0};
    bool                 m_aacsDetected         {false};
    bool                 m_libaacsDetected      {false};
    bool                 m_aacsHandled          {false};
    bool                 m_bdplusDetected       {false};
    bool                 m_libbdplusDetected    {false};
    bool                 m_bdplusHandled        {false};
};

#endif
