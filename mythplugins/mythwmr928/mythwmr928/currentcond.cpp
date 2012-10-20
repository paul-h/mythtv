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

#include <iostream>
#include <unistd.h>

// qt
#include <QString>

// myth
#include <mythcontext.h>
#include <mythdbcon.h>
#include <mythuitext.h>
#include <mythmainwindow.h>

// wmr928
#include "currentcond.h"
#include "wmrclient.h"

#define LOC      QString("CurrentCond: ")

const int COND_UPDATE_TIME = 1000 * 10; // update the current conditions every 10 seconds
const int TIME_UPDATE_TIME = 1000 * 1;    // update the time every 1 second

CurrentCond::CurrentCond(MythScreenStack *parent, QString name) 
            : MythScreenType(parent, name)
{
    m_metric = true;

    m_currentCond = new CurrentConditions;

    m_timeFormat = gCoreContext->GetSetting("TimeFormat", "h:mm AP");

    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, SIGNAL(timeout()), this,
            SLOT(updateTime()));

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()), this,
            SLOT(updateStatus()));
}

CurrentCond::~CurrentCond()
{
    delete m_timeTimer;
    delete m_updateTimer;
}

bool CurrentCond::Create(void)
{
    // Load the theme for this screen
    if (!LoadWindowFromXML("wmr928-ui.xml", "currentconditions", this))
        return false;

    bool err = false;

    UIUtilW::Assign(this, m_time_text,   "time_text");
    UIUtilW::Assign(this, m_date_text,   "date_text");

    UIUtilE::Assign(this, m_outsideTemp, "outside_temp", &err);
    UIUtilE::Assign(this, m_outsideHum,  "outside_hum",  &err);
    UIUtilE::Assign(this, m_outsideDew,  "outside_dew",  &err);

    UIUtilE::Assign(this, m_insideTemp,  "inside_temp",  &err);
    UIUtilE::Assign(this, m_insideHum,   "inside_hum",   &err);
    UIUtilE::Assign(this, m_insideDew,   "inside_dew",   &err);

    UIUtilE::Assign(this, m_windSpeed,   "wind_speed",   &err);
    UIUtilE::Assign(this, m_windGust,    "wind_gust",    &err);
    UIUtilE::Assign(this, m_windDir,     "wind_dir",     &err);

    UIUtilE::Assign(this, m_rainRate,    "rain_rate",    &err);
    UIUtilE::Assign(this, m_rainYest,    "rain_yest",    &err);
    UIUtilE::Assign(this, m_rainTotal,   "rain_total",   &err);

    UIUtilE::Assign(this, m_pressure,    "pressure",     &err);
    UIUtilE::Assign(this, m_sealevel,    "pressure_sealevel", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, LOC + "Cannot load screen 'currentconditions'");
        return false;
    }

    m_timeTimer->start(TIME_UPDATE_TIME);
    m_updateTimer->start(100);

    updateTime();

    return true;
}

void CurrentCond::updateTime(void)
{
    QString s = QTime::currentTime().toString(m_timeFormat);

    if (m_time_text && s != m_time_text->GetText())
        m_time_text->SetText(s);

    s = QDateTime::currentDateTime().toString("dddd\ndd MMM yyyy");

    if (m_date_text && s != m_date_text->GetText())
        m_date_text->SetText(s);
}

void CurrentCond::updateStatus()
{
    m_updateTimer->stop();
    getCurrentCond();
    m_updateTimer->start(COND_UPDATE_TIME);
}

void CurrentCond::getCurrentCond(void)
{
    if (class WMRClient *wmr = WMRClient::get())
    {
        wmr->getCurrentCond(m_currentCond);

        if (m_metric)
        {
            m_outsideTemp->SetText(QString("%1 °C").arg(m_currentCond->outsideTemp, 0, 'f', 1));
            m_outsideHum->SetText(QString("%1%").arg(m_currentCond->outsideHum));
            m_outsideDew->SetText(QString("%1 °C").arg(m_currentCond->outsideDew, 0, 'f', 1));
            m_insideTemp->SetText(QString("%1 °C").arg(m_currentCond->insideTemp, 0, 'f', 1));
            m_insideHum->SetText(QString("%1%").arg(m_currentCond->insideHum));
            m_insideDew->SetText(QString("%1 °C").arg(m_currentCond->insideDew, 0, 'f', 1));

            m_windSpeed->SetText(QString("%1 m/s").arg(m_currentCond->windSpeed, 0, 'f', 1));
            m_windGust->SetText(QString("%1 m/s").arg(m_currentCond->windGust, 0, 'f', 1));
            m_windDir->SetText(QString("%1° (%2)").arg(m_currentCond->windDir)
                                                  .arg(m_currentCond->windDir2));

            m_rainRate->SetText(QString("%1 mm/hour").arg(m_currentCond->rainRate, 0, 'f', 1));
            m_rainYest->SetText(QString("%1 mm").arg(m_currentCond->rainYest, 0, 'f', 1));
            m_rainTotal->SetText(QString("%1 mm").arg(m_currentCond->rainTotal, 0, 'f', 1));

            m_pressure->SetText(QString("%1 mb").arg(m_currentCond->pressure));
            m_sealevel->SetText(QString("%1 mb").arg(m_currentCond->sealevel));
        }
        else
        {

        }
    }
}

bool CurrentCond::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget() && GetFocusWidget()->keyPressEvent(event))
        return true;

    bool handled = false;

    QStringList actions;
    handled = GetMythMainWindow()->TranslateKeyPress("Global", event, actions);

    for (int i = 0; i < actions.size() && !handled; i++)
    {
        QString action = actions[i];
        handled = true;

        if (action == "INFO")
        {
            m_metric = !m_metric;
        }
        else
            handled = false;
    }

    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}
