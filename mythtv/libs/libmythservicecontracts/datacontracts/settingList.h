//////////////////////////////////////////////////////////////////////////////
// Program Name: settingList.h
// Created     : Jan. 15, 2010
//                                                                            
// Copyright (c) 2010 David Blain <dblain@mythtv.org>
//                                          
// Licensed under the GPL v2 or later, see LICENSE for details
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SETTINGLIST_H_
#define SETTINGLIST_H_

#include <QString>
#include <QVariantMap>

#include "serviceexp.h" 
#include "datacontracthelper.h"

namespace DTC
{

class SERVICE_PUBLIC SettingList : public QObject
{
    Q_OBJECT
    Q_CLASSINFO( "version"      , "1.0" );

    // Q_CLASSINFO Used to augment Metadata for properties. 
    // See datacontracthelper.h for details

    Q_CLASSINFO( "Settings", "type=QString;name=String");

    Q_PROPERTY( QString     HostName READ HostName WRITE setHostName )
    Q_PROPERTY( QVariantMap Settings READ Settings )

    PROPERTYIMP_REF   ( QString,     HostName )
    PROPERTYIMP_RO_REF( QVariantMap, Settings )

    public:

        static inline void InitializeCustomTypes();

        Q_INVOKABLE SettingList(QObject *parent = nullptr)
            : QObject( parent )               
        {
        }
        
        void Copy( const SettingList *src )
        {
            m_HostName = src->m_HostName;
            m_Settings = src->m_Settings;
        }

    private:
        Q_DISABLE_COPY(SettingList);
};

inline void SettingList::InitializeCustomTypes()
{
    qRegisterMetaType< SettingList* >();
}

} // namespace DTC

#endif
