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

// myth
#include <mythcontext.h>
#include <mythdbcon.h>
#include <mythuihelper.h>
#include <mythuitext.h>
#include <mythuispinbox.h>
#include <mythuibutton.h>
#include <mythuiimage.h>
#include <mythuibuttonlist.h>
#include <mythmainwindow.h>

// wmr928
#include "chartviewer.h"
#include "wmrclient.h"

const int CHART_UPDATE_TIME = 1000 * 60 * 5; // update the chart every 5 mins
const int TIME_UPDATE_TIME  = 1000 * 1;      // update the time every 1 second

ChartViewer::ChartViewer(MythScreenStack *parent, const QString &chartType,
                         const QString &name)
            :MythScreenType(parent, name)
{
    m_chartType = chartType;

    m_24hrsImage = NULL;
    m_7daysImage = NULL;
    m_4weeksImage = NULL;
    m_customImage = NULL;

    m_timeFormat = gCoreContext->GetSetting("TimeFormat", "h:mm AP");

    m_timeTimer = new QTimer(this);
    connect(m_timeTimer, SIGNAL(timeout()), this,
            SLOT(updateTime()));

    m_updateTimer = new QTimer(this);
    connect(m_updateTimer, SIGNAL(timeout()), this,
            SLOT(updateCharts()));
}

ChartViewer::~ChartViewer()
{
    delete m_timeTimer;
    delete m_updateTimer;
}

bool ChartViewer::Create(void)
{
    // Load the theme for this screen
    if (!LoadWindowFromXML("wmr928-ui.xml", "chartviewer", this))
        return false;

    bool err = false;

    UIUtilW::Assign(this, m_titleText, "title_text");
    UIUtilW::Assign(this, m_timeText, "time_text");
    UIUtilW::Assign(this, m_dateText, "date_text");
    UIUtilE::Assign(this, m_chartImage, "chart_image", &err);
    UIUtilE::Assign(this, m_chartList, "chart_list", &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'chartviewer'");
        return false;
    }

    connect(m_chartList, SIGNAL(itemSelected(MythUIButtonListItem*)),
            this, SLOT(chartChanged(MythUIButtonListItem *)));

    connect(m_chartList, SIGNAL(itemClicked(MythUIButtonListItem*)),
            this, SLOT(chartClicked(MythUIButtonListItem *)));

    BuildFocusList();

    SetFocusWidget(m_chartList);

    initChartList();

    m_timeTimer->start(TIME_UPDATE_TIME);
    m_updateTimer->start(100);

    m_customDate = QDate::currentDate().addDays(-365);
    m_customPeriod = 28;

    updateCharts();
    updateTime();

    return true;
}

void ChartViewer::updateTime(void)
{
    QString s = QTime::currentTime().toString(m_timeFormat);

    if (m_timeText && s != m_timeText->GetText())
        m_timeText->SetText(s);

    s = QDateTime::currentDateTime().toString("dddd\ndd MMM yyyy");

    if (m_dateText && s != m_dateText->GetText())
        m_dateText->SetText(s);
}

void ChartViewer::updateCharts()
{
    m_updateTimer->stop();

    if (m_chartType == "temp")
        getTempCharts();
    else if (m_chartType == "wind")
        getWindCharts();
    else if (m_chartType == "pressure")
        getPressureCharts();
    else if (m_chartType == "humidity")
        getHumidityCharts();
    else if (m_chartType == "rain")
        getRainCharts();

    m_updateTimer->start(CHART_UPDATE_TIME);
}

void ChartViewer::getTempCharts(void)
{
    if (m_titleText)
        m_titleText->SetText("Temperature Charts");

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime;
        QDateTime endTime = QDateTime::currentDateTime().addSecs(60 * 60 * 2);
        MythUIButtonListItem *item;
        QSize chartSize = m_chartImage->GetArea().size();

        // 24 hours chart
        startTime = QDateTime::currentDateTime().addDays(-1);
        wmr->getChart("temp", startTime, endTime, "min10", chartSize, &m_24hrsImage);

        if (m_24hrsImage)
        {
           item = m_chartList->GetItemAt(0);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_24hrsImage);
               item->SetImage(image);
           }
        }

        // 7 days chart
        startTime = QDateTime::currentDateTime().addDays(-7);
        wmr->getChart("temp", startTime, endTime, "hour1", chartSize, &m_7daysImage);

        if (m_7daysImage)
        {
           item = m_chartList->GetItemAt(1);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_7daysImage);
               item->SetImage(image);
           }
        }

        // 4 weeks chart
        startTime = QDateTime::currentDateTime().addDays(-28);
        wmr->getChart("temp-minmax", startTime, endTime, "day1", chartSize, &m_4weeksImage);

        if (m_4weeksImage)
        {
           item = m_chartList->GetItemAt(2);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_4weeksImage);
               item->SetImage(image);
           }
        }

        doCustomChart(m_customDate, m_customPeriod);
    }

    chartChanged(m_chartList->GetItemCurrent());
}

void ChartViewer::getWindCharts(void)
{
    if (m_titleText)
        m_titleText->SetText("Wind Speed Charts");

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime;
        QDateTime endTime = QDateTime::currentDateTime().addSecs(60 * 60 * 2);
        MythUIButtonListItem *item;
        QSize chartSize = m_chartImage->GetArea().size();

        // 24 hours chart
        startTime = QDateTime::currentDateTime().addDays(-1);
        wmr->getChart("wind", startTime, endTime, "min10", chartSize, &m_24hrsImage);

        if (m_24hrsImage)
        {
           item = m_chartList->GetItemAt(0);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_24hrsImage);
               item->SetImage(image);
           }
        }

        // 7 days chart
        startTime = QDateTime::currentDateTime().addDays(-7);
        wmr->getChart("wind", startTime, endTime, "hour1", chartSize, &m_7daysImage);

        if (m_7daysImage)
        {
           item = m_chartList->GetItemAt(1);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_7daysImage);
               item->SetImage(image);
           }
        }

        // 4 weeks chart
        startTime = QDateTime::currentDateTime().addDays(-28);
        wmr->getChart("wind-minmax", startTime, endTime, "day1", chartSize, &m_4weeksImage);

        if (m_4weeksImage)
        {
           item = m_chartList->GetItemAt(2);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_4weeksImage);
               item->SetImage(image);
           }
        }

        doCustomChart(m_customDate, m_customPeriod);
    }

    chartChanged(m_chartList->GetItemCurrent());
}

void ChartViewer::getHumidityCharts(void)
{
    if (m_titleText)
        m_titleText->SetText("Humidity Charts");

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime;
        QDateTime endTime = QDateTime::currentDateTime().addSecs(60 * 60 * 2);
        MythUIButtonListItem *item;
        QSize chartSize = m_chartImage->GetArea().size();

        // 24 hours chart
        startTime = QDateTime::currentDateTime().addDays(-1);
        wmr->getChart("hum", startTime, endTime, "min10", chartSize, &m_24hrsImage);

        if (m_24hrsImage)
        {
           item = m_chartList->GetItemAt(0);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_24hrsImage);
               item->SetImage(image);
           }
        }

        // 7 days chart
        startTime = QDateTime::currentDateTime().addDays(-7);
        wmr->getChart("hum", startTime, endTime, "hour1", chartSize, &m_7daysImage);

        if (m_7daysImage)
        {
           item = m_chartList->GetItemAt(1);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_7daysImage);
               item->SetImage(image);
           }
        }

        // 4 weeks chart
        startTime = QDateTime::currentDateTime().addDays(-28);
        wmr->getChart("hum-minmax", startTime, endTime, "day1", chartSize, &m_4weeksImage);

        if (m_4weeksImage)
        {
           item = m_chartList->GetItemAt(2);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_4weeksImage);
               item->SetImage(image);
           }
        }

        doCustomChart(m_customDate, m_customPeriod);
    }

    chartChanged(m_chartList->GetItemCurrent());
}

void ChartViewer::getPressureCharts(void)
{
    if (m_titleText)
        m_titleText->SetText("Barometric Pressure Charts");

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime;
        QDateTime endTime = QDateTime::currentDateTime().addSecs(60 * 60 * 2);
        MythUIButtonListItem *item;
        QSize chartSize = m_chartImage->GetArea().size();

        // 24 hours chart
        startTime = QDateTime::currentDateTime().addDays(-1);
        wmr->getChart("baro", startTime, endTime, "min10", chartSize, &m_24hrsImage);

        if (m_24hrsImage)
        {
           item = m_chartList->GetItemAt(0);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_24hrsImage);
               item->SetImage(image);
           }
        }

        // 7 days chart
        startTime = QDateTime::currentDateTime().addDays(-7);
        wmr->getChart("baro", startTime, endTime, "hour1", chartSize, &m_7daysImage);

        if (m_7daysImage)
        {
           item = m_chartList->GetItemAt(1);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_7daysImage);
               item->SetImage(image);
           }
        }

        // 4 weeks chart
        startTime = QDateTime::currentDateTime().addDays(-28);
        wmr->getChart("baro-minmax", startTime, endTime, "day1", chartSize, &m_4weeksImage);

        if (m_4weeksImage)
        {
           item = m_chartList->GetItemAt(2);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_4weeksImage);
               item->SetImage(image);
           }
        }

        doCustomChart(m_customDate, m_customPeriod);
    }

    chartChanged(m_chartList->GetItemCurrent());
}

void ChartViewer::getRainCharts(void)
{
    if (m_titleText)
        m_titleText->SetText("Rain Charts");

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime;
        QDateTime endTime = QDateTime::currentDateTime().addSecs(60 * 60 * 2);
        MythUIButtonListItem *item;
        QSize chartSize = m_chartImage->GetArea().size();

        // 24 hours chart
        startTime = QDateTime::currentDateTime().addDays(-1);
        wmr->getChart("rain", startTime, endTime, "hour1", chartSize, &m_24hrsImage);

        if (m_24hrsImage)
        {
           item = m_chartList->GetItemAt(0);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_24hrsImage);
               item->SetImage(image);
           }
        }

        // 7 days chart
        startTime = QDateTime::currentDateTime().addDays(-7);
        wmr->getChart("rain", startTime, endTime, "hour6", chartSize, &m_7daysImage);

        if (m_7daysImage)
        {
           item = m_chartList->GetItemAt(1);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_7daysImage);
               item->SetImage(image);
           }
        }

        // 4 weeks chart
        startTime = QDateTime::currentDateTime().addDays(-28);
        wmr->getChart("rain", startTime, endTime, "day1", chartSize, &m_4weeksImage);

        if (m_4weeksImage)
        {
           item = m_chartList->GetItemAt(2);
           if (item)
           {
               MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
               image->Assign(*m_4weeksImage);
               item->SetImage(image);
           }
        }

        doCustomChart(m_customDate, m_customPeriod);
    }

    chartChanged(m_chartList->GetItemCurrent());
}

bool ChartViewer::keyPressEvent(QKeyEvent *event)
{
    if (GetFocusWidget() && GetFocusWidget()->keyPressEvent(event))
        return true;

    QStringList actions;
    bool handled = GetMythMainWindow()->TranslateKeyPress("Global", event, actions);

    for (int i = 0; i < actions.size() && !handled; i++)
    {
        QString action = actions[i];
        handled = true;

        if (action == "INFO")
        {
            //
        }
        else
            handled = false;
    }

    if (!handled && MythScreenType::keyPressEvent(event))
        handled = true;

    return handled;
}

void ChartViewer::initChartList()
{
    new MythUIButtonListItem(m_chartList, "Last 24 Hours");
    new MythUIButtonListItem(m_chartList, "Last 7 Days");
    new MythUIButtonListItem(m_chartList, "Last 4 Weeks");
    new MythUIButtonListItem(m_chartList, "Custom Chart");
}

void ChartViewer::chartChanged(MythUIButtonListItem *item)
{
    if (!item)
        return;

    if (item->GetText() == "Last 24 Hours")
    {
        if (m_24hrsImage)
        {
            m_chartImage->SetImage(m_24hrsImage);
            m_24hrsImage->SetChanged(true);
        }
        else
        {
            m_chartImage->SetFilename("");
            m_chartImage->Load();
        }
    }
    else if (item->GetText() == "Last 7 Days")
    {
        if (m_7daysImage)
        {
            m_chartImage->SetImage(m_7daysImage);
            m_7daysImage->SetChanged(true);
        }
        else
        {
            m_chartImage->SetFilename("");
            m_chartImage->Load();
        }
    }
    else if (item->GetText() == "Last 4 Weeks")
    {
        if (m_4weeksImage)
        {
            m_chartImage->SetImage(m_4weeksImage);
            m_4weeksImage->SetChanged(true);
        }
        else
        {
            m_chartImage->SetFilename("");
            m_chartImage->Load();
        }
    }
    else if (item->GetText() == "Custom Chart")
    {
        if (m_customImage)
        {
            m_chartImage->SetImage(m_customImage);
            m_customImage->SetChanged(true);
        }
        else
        {
            m_chartImage->SetFilename("");
            m_chartImage->Load();
        }
    }
}

void ChartViewer::chartClicked(MythUIButtonListItem* item)
{
    (void) item;

    MythScreenStack *popupStack = GetMythMainWindow()->GetStack("popup stack");
    CustomChartDialog *dlg = new CustomChartDialog(popupStack);

    if (!dlg->Create())
    {
        delete dlg;
        return;
    }

    dlg->setStartDate(m_customDate);
    dlg->setPeriod(m_customPeriod);

    connect(dlg, SIGNAL(changed(QDate, int)), SLOT(doCustomChart(QDate, int)));

    popupStack->AddScreen(dlg);
}

void ChartViewer::doCustomChart(QDate date, int period)
{
    m_customDate = date;
    m_customPeriod = period;

    if (class WMRClient *wmr = WMRClient::get())
    {
        QDateTime startTime(date);
        QDateTime endTime = startTime.addDays(period);
        QSize chartSize = m_chartImage->GetArea().size();
        QString range;
        QString chartType;

        // choose which chart type we want based on the current chart and range
        if (m_chartType == "temp")
        {
            if (period == 1 || period == 7)
                chartType = "temp";
            else
                chartType = "temp-minmax";
        }
        else if (m_chartType == "wind")
            if (period == 1 || period == 7)
                chartType = "wind";
            else
                chartType = "wind-minmax";
        else if (m_chartType == "pressure")
            if (period == 1 || period == 7)
                chartType = "baro";
            else
                chartType = "baro-minmax";
        else if (m_chartType == "humidity")
            if (period == 1 || period == 7)
                chartType = "hum";
            else
                chartType = "hum-minmax";
        else if (m_chartType == "rain")
            chartType = "rain";

        switch (period)
        {
            case 1:
                range = "min10";
                break;
            case 7:
                range = "min10";
                break;
            case 28:
                range = "day1";
                break;
            case 365:
                range = "day1";
                break;
            default:
                range = "min10";
                break;
        }

        wmr->getChart(chartType, startTime, endTime, range, chartSize, &m_customImage);

        if (m_customImage)
        {
            MythUIButtonListItem *item = m_chartList->GetItemAt(3);
            if (item)
            {
                MythImage *image = GetMythMainWindow()->GetCurrentPainter()->GetFormatImage();
                image->Assign(*m_customImage);
                item->SetImage(image);
            }
        }

        chartChanged(m_chartList->GetItemCurrent());
    }
}

/*
---------------------------------------------------------------------
*/

CustomChartDialog::CustomChartDialog(MythScreenStack *parent)
                 :MythScreenType(parent, "CustomChartDialog"),
                  m_daySpin(NULL), m_monthSpin(NULL), m_yearSpin(NULL),
                  m_periodSelector(NULL), m_okButton(NULL)
{
}

bool CustomChartDialog::Create(void)
{
    if (!LoadWindowFromXML("wmr928-ui.xml", "customchartdialog", this))
        return false;

    bool err = false;

    UIUtilE::Assign(this, m_daySpin,      "day",     &err);
    UIUtilE::Assign(this, m_monthSpin,    "month",   &err);
    UIUtilE::Assign(this, m_yearSpin,     "year",    &err);
    UIUtilE::Assign(this, m_periodSelector,  "period",  &err);
    UIUtilE::Assign(this, m_okButton,     "ok",      &err);

    if (err)
    {
        LOG(VB_GENERAL, LOG_ERR, "Cannot load screen 'customchartdialog'");
        return false;
    }

    m_daySpin->SetRange(1, 31, 1);
    m_monthSpin->SetRange(1, 12, 1);
    m_yearSpin->SetRange(1900, 2099, 1);

    QDate now = QDate::currentDate();
    m_daySpin->SetValue(now.day());
    m_monthSpin->SetValue(now.month());
    m_yearSpin->SetValue(now.year());

    new MythUIButtonListItem(m_periodSelector, "Day");
    new MythUIButtonListItem(m_periodSelector, "Week");
    new MythUIButtonListItem(m_periodSelector, "Month");
    new MythUIButtonListItem(m_periodSelector, "Year");

    connect(m_okButton, SIGNAL(Clicked()), this, SLOT(okPressed()));

    BuildFocusList();

    return true;
}

CustomChartDialog::~CustomChartDialog(void)
{
}

QDate CustomChartDialog::getStartDate(void)
{
    QDate dtResult;

    int day = m_daySpin->GetIntValue();
    int month = m_monthSpin->GetIntValue();
    int year = m_yearSpin->GetIntValue();

    dtResult.setYMD(year, month,day);

    return dtResult;
}

void CustomChartDialog::setStartDate(const QDate &date)
{
    m_daySpin->SetValue(date.day());
    m_monthSpin->SetValue(date.month());
    m_yearSpin->SetValue(date.year());
}

int CustomChartDialog::getPeriod(void )
{
    QString period = m_periodSelector->GetValue();
    int days = 1;

    if (period == "Day")
        days = 1;
    else if (period == "Week")
        days = 7;
    else if (period == "Month")
        days = 28;
    else if (period == "Year")
        days = 365;

    return days;
}

void CustomChartDialog::setPeriod(int period)
{
    QString sPeriod = "Day";

    if (period == 1)
        sPeriod = "Day";
    else if (period == 7)
        sPeriod = "Week";
    else if (period == 28)
       sPeriod = "Month";
    else if (period == 365)
        sPeriod = "Year";

    m_periodSelector->SetValue(sPeriod);
}

void CustomChartDialog::okPressed(void )
{
    QDate date = getStartDate();
    int period = getPeriod();

    emit changed(date, period);

    Close();
}
