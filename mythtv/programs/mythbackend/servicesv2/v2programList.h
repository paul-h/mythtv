//////////////////////////////////////////////////////////////////////////////
// Program Name: programs.h
// Created     : Jan. 15, 2010
//
// Copyright (c) 2010 David Blain <dblain@mythtv.org>
//
// Licensed under the GPL v2 or later, see COPYING for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef V2PROGRAMLIST_H_
#define V2PROGRAMLIST_H_

#include <QDateTime>
#include <QString>
#include <QVariantList>

#include "libmythbase/http/mythhttpservice.h"

#include "v2programAndChannel.h"

class V2ProgramList : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "Version", "1.0" );

    Q_CLASSINFO( "Programs", "type=V2Program");
    Q_CLASSINFO( "AsOf"    , "transient=true"   );

    SERVICE_PROPERTY2( int         , StartIndex      )
    SERVICE_PROPERTY2( int         , Count           )
    SERVICE_PROPERTY2( int         , TotalAvailable  )
    SERVICE_PROPERTY2( QDateTime   , AsOf            )
    SERVICE_PROPERTY2( QString     , Version         )
    SERVICE_PROPERTY2( QString     , ProtoVer        )
    SERVICE_PROPERTY2( QVariantList, Programs      );

    public:

        Q_INVOKABLE V2ProgramList(QObject *parent = nullptr)
            : QObject         ( parent ),
              m_StartIndex    ( 0      ),
              m_Count         ( 0      ),
              m_TotalAvailable( 0      )
        {
        }

        void Copy( const V2ProgramList *src )
        {
            m_StartIndex    = src->m_StartIndex     ;
            m_Count         = src->m_Count          ;
            m_TotalAvailable= src->m_TotalAvailable ;
            m_AsOf          = src->m_AsOf           ;
            m_Version       = src->m_Version        ;
            m_ProtoVer      = src->m_ProtoVer       ;

            CopyListContents< V2Program >( this, m_Programs, src->m_Programs );
        }

        // This is here so that a routine in serviceutil (FillUpcomingList)
        // can fill the QVariantlist of programs. Unfortunately the standard
        // call Programs() generated by the macros returns a const QVariantlist.
        QVariantList& GetPrograms() {return m_Programs;}

        V2Program *AddNewProgram()
        {
            // We must make sure the object added to the QVariantList has
            // a parent of 'this'

            auto *pObject = new V2Program( this );
            m_Programs.append( QVariant::fromValue<QObject *>( pObject ));

            return pObject;
        }

    private:
        Q_DISABLE_COPY(V2ProgramList);
};

Q_DECLARE_METATYPE(V2ProgramList*)

#endif
