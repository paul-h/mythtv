//////////////////////////////////////////////////////////////////////////////
// Program Name: videoLookupInfoList.h
// Created     : Jul. 19, 2011
//
// Copyright (c) 2011 Robert McNamara <rmcnamara@mythtv.org>
//
// Licensed under the GPL v2 or later, see LICENSE for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef VIDEOLOOKUPINFOLIST_H_
#define VIDEOLOOKUPINFOLIST_H_

#include <QVariantList>

#include "serviceexp.h"
#include "datacontracthelper.h"

#include "videoLookupInfo.h"

namespace DTC
{

class SERVICE_PUBLIC VideoLookupList : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "version", "1.0" );

    // Q_CLASSINFO Used to augment Metadata for properties. 
    // See datacontracthelper.h for details

    Q_CLASSINFO( "VideoLookups", "type=DTC::VideoLookup");
    Q_CLASSINFO( "AsOf"        , "transient=true"       );

    Q_PROPERTY( int          Count          READ Count           WRITE setCount          )
    Q_PROPERTY( QDateTime    AsOf           READ AsOf            WRITE setAsOf           )
    Q_PROPERTY( QString      Version        READ Version         WRITE setVersion        )
    Q_PROPERTY( QString      ProtoVer       READ ProtoVer        WRITE setProtoVer       )

    Q_PROPERTY( QVariantList VideoLookups READ VideoLookups )

    PROPERTYIMP       ( int         , Count           )
    PROPERTYIMP_REF   ( QDateTime   , AsOf            )
    PROPERTYIMP_REF   ( QString     , Version         )
    PROPERTYIMP_REF   ( QString     , ProtoVer        )

    PROPERTYIMP_RO_REF( QVariantList, VideoLookups );

    public:

        static inline void InitializeCustomTypes();

        Q_INVOKABLE VideoLookupList(QObject *parent = nullptr)
            : QObject( parent ),
              m_Count         ( 0      )
        {
        }

        void Copy( const VideoLookupList *src )
        {
            m_Count         = src->m_Count          ;
            m_AsOf          = src->m_AsOf           ;
            m_Version       = src->m_Version        ;
            m_ProtoVer      = src->m_ProtoVer       ;

            CopyListContents< VideoLookup >( this, m_VideoLookups, src->m_VideoLookups );
        }

        VideoLookup *AddNewVideoLookup()
        {
            // We must make sure the object added to the QVariantList has
            // a parent of 'this'

            auto *pObject = new VideoLookup( this );
            m_VideoLookups.append( QVariant::fromValue<QObject *>( pObject ));

            return pObject;
        }

    private:
        Q_DISABLE_COPY(VideoLookupList);
};

inline void VideoLookupList::InitializeCustomTypes()
{
    qRegisterMetaType< VideoLookupList* >();

    VideoLookup::InitializeCustomTypes();
}

} // namespace DTC

#endif
