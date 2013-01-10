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

#ifndef CHARTVIEWER_H
#define CHARTVIEWER_H

// mythtv
#include <mythscreentype.h>

//mythwmr928
#include "wmrclient.h"

class MythUIText;
class MythUIImage;
class MythUIButtonList;
class MythUIButtonListItem;

class ChartViewer : public MythScreenType
{
    Q_OBJECT

public:

    ChartViewer(MythScreenStack *parent, const QString &chartType,
                const QString &name);
    ~ChartViewer();

    bool Create(void);
    bool keyPressEvent(QKeyEvent *);

  private slots:
    void updateTime();
    void updateCharts();
    void getTempCharts(void);
    void getWindCharts(void);
    void getHumidityCharts(void);
    void getPressureCharts(void);
    void getRainCharts(void);

    void chartChanged(MythUIButtonListItem *item);
    void chartClicked(MythUIButtonListItem *item);

    void doCustomChart(QDate date, int period);

  private:
    void initChartList(void);

    QString            m_chartType;
    QString            m_timeFormat;

    MythUIText        *m_titleText;
    MythUIText        *m_timeText;
    MythUIText        *m_dateText;

    MythUIImage       *m_chartImage;
    MythUIButtonList  *m_chartList;

    QTimer            *m_updateTimer;
    QTimer            *m_timeTimer;

    MythImage         *m_24hrsImage;
    MythImage         *m_7daysImage;
    MythImage         *m_4weeksImage;
    MythImage         *m_customImage;

    QDate              m_customDate;
    int                m_customPeriod;
};

class CustomChartDialog: public MythScreenType
{
  Q_OBJECT

  public:

    CustomChartDialog(MythScreenStack *parent);
    ~CustomChartDialog();

    bool Create(void);

    QDate getStartDate(void);
    void setStartDate(const QDate &date);

    int getPeriod(void);
    void setPeriod(int period);

  signals:
    void changed(QDate date, int period);

  protected slots:
    void okPressed(void);

  private:

    MythUISpinBox    *m_daySpin;
    MythUISpinBox    *m_monthSpin;
    MythUISpinBox    *m_yearSpin;

    MythUIButtonList *m_periodSelector;

    MythUIButton     *m_okButton;
};

#endif
