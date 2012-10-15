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

MythUIText* CurrentCond::GetMythUIText(const QString &name, bool optional)
{
    MythUIText *text = dynamic_cast<MythUIText *> (GetChild(name));

    if (!optional && !text)
        throw name;

    return text;
}

bool CurrentCond::Create(void)
{
    bool foundtheme = false;

    // Load the theme for this screen
    foundtheme = LoadWindowFromXML("wmr928-ui.xml", "currentconditions", this);

    if (!foundtheme)
        return false;

    try
    {
        m_time_text = GetMythUIText("time_text");
        m_date_text = GetMythUIText("date_text");

        m_outsideTemp = GetMythUIText("outside_temp");
        m_outsideHum = GetMythUIText("outside_hum");
        m_outsideDew = GetMythUIText("outside_dew");

        m_insideTemp = GetMythUIText("inside_temp");
        m_insideHum = GetMythUIText("inside_hum");
        m_insideDew = GetMythUIText("inside_dew");

        m_windSpeed = GetMythUIText("wind_speed");
        m_windGust = GetMythUIText("wind_gust");
        m_windDir = GetMythUIText("wind_dir");

        m_rainRate = GetMythUIText("rain_rate");
        m_rainYest = GetMythUIText("rain_yest");
        m_rainTotal = GetMythUIText("rain_total");

        m_pressure = GetMythUIText("pressure");
        m_sealevel = GetMythUIText("pressure_sealevel");
    }
    catch (const QString name)
    {
        LOG(VB_GENERAL, LOG_ERR, QString("Theme is missing a critical theme element ('%1')")
                                         .arg(name));
        Close();
    }

    m_timeTimer->start(TIME_UPDATE_TIME);
    m_updateTimer->start(100);

    updateTime();

    return true;
}

void CurrentCond::updateTime(void)
{
    QString s = QTime::currentTime().toString(m_timeFormat);

    if (s != m_time_text->GetText())
        m_time_text->SetText(s);

    s = QDateTime::currentDateTime().toString("dddd\ndd MMM yyyy");

    if (s != m_date_text->GetText())
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
