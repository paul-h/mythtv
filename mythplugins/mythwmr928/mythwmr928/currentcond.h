/* ============================================================
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef WMRCURRENTCOND_H
#define WMRCURRENTCOND_H


// mythtv
#include <mythscreentype.h>

//mythwmr928
#include "wmrclient.h"

class MythUIText;

class CurrentCond : public MythScreenType
{
    Q_OBJECT

public:

    CurrentCond(MythScreenStack *parent, QString name);
    ~CurrentCond();

    bool Create(void);
    bool keyPressEvent(QKeyEvent *);

  private slots:
    void updateTime();
    void updateStatus();
    void getCurrentCond(void);

  private:
    MythUIText* GetMythUIText(const QString &name, bool optional = false);

    bool        m_metric;

    QString     m_timeFormat;

    MythUIText *m_time_text;
    MythUIText *m_date_text;

    MythUIText *m_outsideTemp;
    MythUIText *m_outsideHum;
    MythUIText *m_outsideDew;

    MythUIText *m_insideTemp;
    MythUIText *m_insideHum;
    MythUIText *m_insideDew;

    MythUIText *m_windDir;
    MythUIText *m_windSpeed;
    MythUIText *m_windGust;

    MythUIText *m_rainRate;
    MythUIText *m_rainYest;
    MythUIText *m_rainTotal;

    MythUIText *m_pressure;
    MythUIText *m_sealevel;

    QTimer     *m_updateTimer;
    QTimer     *m_timeTimer;

    CurrentConditions *m_currentCond;
};

#endif
