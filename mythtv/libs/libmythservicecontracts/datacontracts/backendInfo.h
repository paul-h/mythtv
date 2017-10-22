//////////////////////////////////////////////////////////////////////////////
// Program Name: backendInfo.h
// Created     : Dec. 15, 2015
//
// Copyright (c) 2015 Bill Meek, from: 2010 David Blain <dblain@mythtv.org>
//
// Licensed under the GPL v2 or later, see COPYING for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BACKENDINFO_H_
#define BACKENDINFO_H_

#include "serviceexp.h"
#include "datacontracthelper.h"

#include "buildInfo.h"
#include "envInfo.h"
#include "logInfo.h"

namespace DTC
{

class SERVICE_PUBLIC BackendInfo : public QObject
{
    Q_OBJECT

    Q_CLASSINFO( "version"    , "1.0" );

    Q_PROPERTY( QObject*    Build   READ Build )
    Q_PROPERTY( QObject*    Env     READ Env   )
    Q_PROPERTY( QObject*    Log     READ Log   )

    PROPERTYIMP_PTR( BuildInfo, Build )
    PROPERTYIMP_PTR( EnvInfo,   Env   )
    PROPERTYIMP_PTR( LogInfo,   Log   );

    public:

        static inline void InitializeCustomTypes();

    public:

        BackendInfo(QObject *parent = 0)
            : QObject   ( parent ),
              m_Build   ( NULL   ),
              m_Env     ( NULL   ),
              m_Log     ( NULL   )
        {
        }

        BackendInfo( const BackendInfo &src )
            : m_Build  ( NULL ),
              m_Env    ( NULL ),
              m_Log    ( NULL )
        {
            Copy( src );
        }

        void Copy( const BackendInfo &src )
        {
            // We always need to make sure the child object is
            // created with the correct parent *

            if (src.m_Build)
                Build()->Copy( src.m_Build );

            if (src.m_Env)
                Env()->Copy( src.m_Env  );

            if (src.m_Log)
                Log()->Copy( src.m_Log  );

        }
};

typedef BackendInfo* BackendInfoPtr;

} // namespace DTC

Q_DECLARE_METATYPE( DTC::BackendInfo  )
Q_DECLARE_METATYPE( DTC::BackendInfo* )

namespace DTC
{
inline void BackendInfo::InitializeCustomTypes()
{
    qRegisterMetaType< BackendInfo  >();
    qRegisterMetaType< BackendInfo* >();

    BuildInfo::InitializeCustomTypes();
    EnvInfo  ::InitializeCustomTypes();
    LogInfo  ::InitializeCustomTypes();
}
}

#endif
