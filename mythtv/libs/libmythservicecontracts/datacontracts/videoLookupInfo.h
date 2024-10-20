//////////////////////////////////////////////////////////////////////////////
// Program Name: VideoLookupInfo.h
// Created     : Jul. 19, 2011
//
// Copyright (c) 2011 Robert McNamara <rmcnamara@mythtv.org>
//
// Licensed under the GPL v2 or later, see LICENSE for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef VIDEOLOOKUPINFO_H_
#define VIDEOLOOKUPINFO_H_

#include <QString>
#include <QDateTime>

#include "serviceexp.h"
#include "datacontracthelper.h"

namespace DTC
{

/////////////////////////////////////////////////////////////////////////////

class SERVICE_PUBLIC ArtworkItem : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "version"    , "1.02" );

    Q_PROPERTY( QString         Type            READ Type             WRITE setType           )
    Q_PROPERTY( QString         Url             READ Url              WRITE setUrl            )
    Q_PROPERTY( QString         Thumbnail       READ Thumbnail        WRITE setThumbnail      )
    Q_PROPERTY( int             Width           READ Width            WRITE setWidth          )
    Q_PROPERTY( int             Height          READ Height           WRITE setHeight         )

    PROPERTYIMP_REF( QString    , Type           )
    PROPERTYIMP_REF( QString    , Url            )
    PROPERTYIMP_REF( QString    , Thumbnail      )
    PROPERTYIMP    ( int        , Width          )
    PROPERTYIMP    ( int        , Height          );

    public:

        static inline void InitializeCustomTypes();

        Q_INVOKABLE ArtworkItem(QObject *parent = nullptr)
                        : QObject         ( parent )
        {
            m_Width            = 0                      ;
            m_Height           = 0                      ;
        }

        void Copy( const ArtworkItem *src )
        {
            m_Type             = src->m_Type             ;
            m_Url              = src->m_Url              ;
            m_Thumbnail        = src->m_Thumbnail        ;
            m_Width            = src->m_Width            ;
            m_Height           = src->m_Height           ;
        }

    private:
        Q_DISABLE_COPY(ArtworkItem);
};

/////////////////////////////////////////////////////////////////////////////

class SERVICE_PUBLIC VideoLookup : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "version"    , "1.0" );

    Q_CLASSINFO( "Artwork", "type=DTC::ArtworkItem");

    Q_PROPERTY( QString         Title           READ Title            WRITE setTitle          )
    Q_PROPERTY( QString         SubTitle        READ SubTitle         WRITE setSubTitle       )
    Q_PROPERTY( int             Season          READ Season           WRITE setSeason         )
    Q_PROPERTY( int             Episode         READ Episode          WRITE setEpisode        )
    Q_PROPERTY( int             Year            READ Year             WRITE setYear           )
    Q_PROPERTY( QString         Tagline         READ Tagline          WRITE setTagline        )
    Q_PROPERTY( QString         Description     READ Description      WRITE setDescription    )
    Q_PROPERTY( QString         Certification   READ Certification    WRITE setCertification  )
    Q_PROPERTY( QString         Inetref         READ Inetref          WRITE setInetref        )
    Q_PROPERTY( QString         Collectionref   READ Collectionref    WRITE setCollectionref  )
    Q_PROPERTY( QString         HomePage        READ HomePage         WRITE setHomePage       )
    Q_PROPERTY( QDateTime       ReleaseDate     READ ReleaseDate      WRITE setReleaseDate    )
    Q_PROPERTY( float           UserRating      READ UserRating       WRITE setUserRating     )
    Q_PROPERTY( int             Length          READ Length           WRITE setLength         )
    Q_PROPERTY( QString         Language        READ Language         WRITE setLanguage       )
    Q_PROPERTY( QStringList     Countries       READ Countries        WRITE setCountries      )
    Q_PROPERTY( float           Popularity      READ Popularity       WRITE setPopularity     )
    Q_PROPERTY( int             Budget          READ Budget           WRITE setBudget         )
    Q_PROPERTY( int             Revenue         READ Revenue          WRITE setRevenue        )
    Q_PROPERTY( QString         IMDB            READ IMDB             WRITE setIMDB           )
    Q_PROPERTY( QString         TMSRef          READ TMSRef           WRITE setTMSRef         )

    Q_PROPERTY( QVariantList Artwork    READ Artwork )

    PROPERTYIMP_REF( QString    , Title          )
    PROPERTYIMP_REF( QString    , SubTitle       )
    PROPERTYIMP    ( int        , Season         )
    PROPERTYIMP    ( int        , Episode        )
    PROPERTYIMP    ( int        , Year           )
    PROPERTYIMP_REF( QString    , Tagline        )
    PROPERTYIMP_REF( QString    , Description    )
    PROPERTYIMP_REF( QString    , Certification  )
    PROPERTYIMP_REF( QString    , Inetref        )
    PROPERTYIMP_REF( QString    , Collectionref  )
    PROPERTYIMP_REF( QString    , HomePage       )
    PROPERTYIMP_REF( QDateTime  , ReleaseDate    )
    PROPERTYIMP    ( float      , UserRating     )
    PROPERTYIMP    ( int        , Length         )
    PROPERTYIMP_REF( QString    , Language       )
    PROPERTYIMP_REF( QStringList, Countries      )
    PROPERTYIMP    ( float      , Popularity     )
    PROPERTYIMP    ( int        , Budget         )
    PROPERTYIMP    ( int        , Revenue        )
    PROPERTYIMP_REF( QString    , IMDB           )
    PROPERTYIMP_REF( QString    , TMSRef         )

    PROPERTYIMP_RO_REF( QVariantList, Artwork)

    public:

        static inline void InitializeCustomTypes();

        Q_INVOKABLE VideoLookup(QObject *parent = nullptr)
                        : QObject         ( parent )
        {
            m_Season           = 0                      ;
            m_Episode          = 0                      ;
            m_Year             = 0                      ;
            m_UserRating       = 0.0                    ;
            m_Length           = 0.0                    ;
            m_Popularity       = 0.0                    ;
            m_Budget           = 0                      ;
            m_Revenue          = 0                      ;
        }

        void Copy( const VideoLookup *src )
        {
            m_Title            = src->m_Title            ;
            m_SubTitle         = src->m_SubTitle         ;
            m_Season           = src->m_Season           ;
            m_Episode          = src->m_Episode          ;
            m_Year             = src->m_Year             ;
            m_Tagline          = src->m_Tagline          ;
            m_Description      = src->m_Description      ;
            m_Certification    = src->m_Certification    ;
            m_Inetref          = src->m_Inetref          ;
            m_Collectionref    = src->m_Collectionref    ;
            m_HomePage         = src->m_HomePage         ;
            m_ReleaseDate      = src->m_ReleaseDate      ;
            m_UserRating       = src->m_UserRating       ;
            m_Length           = src->m_Length           ;
            m_Popularity       = src->m_Popularity       ;
            m_Budget           = src->m_Budget           ;
            m_Revenue          = src->m_Revenue          ;
            m_IMDB             = src->m_IMDB             ;
            m_TMSRef           = src->m_TMSRef           ;

            CopyListContents< ArtworkItem >( this, m_Artwork, src->m_Artwork );
        }

        ArtworkItem *AddNewArtwork()
        {
            auto *pObject = new ArtworkItem( this );
            Artwork().append( QVariant::fromValue<QObject *>( pObject ));

            return pObject;
        }

};

inline void ArtworkItem::InitializeCustomTypes()
{
    qRegisterMetaType< ArtworkItem*   >();
}

inline void VideoLookup::InitializeCustomTypes()
{
    qRegisterMetaType< VideoLookup* >();

    ArtworkItem::InitializeCustomTypes();
}

} // namespace DTC

#endif
