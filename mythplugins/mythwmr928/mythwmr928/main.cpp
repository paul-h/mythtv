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
#include <qapplication.h>

// myth
#include <mythcontext.h>
#include <mythplugin.h>
#include <mythpluginapi.h>
#include <mythversion.h>
#include <mythmainwindow.h>
#include <mythuihelper.h>
#include <myththemedmenu.h>

// wmr928
#include "currentcond.h"
#include "chartviewer.h"
#include "wmrsettings.h"
#include "wmrclient.h"

using namespace std;

void runCurrentCond(void);
void runTempViewer(void);
void runWindViewer(void);

void setupKeys(void)
{
//    REG_JUMP("ZoneMinder Console",    "", "", runZMConsole);
//    REG_JUMP("ZoneMinder Live View",  "", "", runZMLiveView);
//    REG_JUMP("ZoneMinder Events",     "", "", runZMEventView);
}

bool checkConnection(void)
{
    if (!WMRClient::get()->connected())
    {
        if (!WMRClient::setupWMRClient())
            return false;
    }

    return true;
}

int mythplugin_init(const char *libversion)
{
    if (!gContext->TestPopupVersion("mythwmr928",
                                    libversion,
                                    MYTH_BINARY_VERSION))
        return -1;

    setupKeys();

    return 0;
}

void runCurrentCond(void)
{
    if (!checkConnection())
        return;

    MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();

    CurrentCond *current = new CurrentCond(mainStack, "CurrentCond");

    if (current->Create())
        mainStack->AddScreen(current);
}

void runChartViewer(const QString &charttype)
{
    if (!checkConnection())
        return;

    MythScreenStack *mainStack = GetMythMainWindow()->GetMainStack();

    ChartViewer *chart = new ChartViewer(mainStack, charttype, "ChartViewer");

    if (chart->Create())
        mainStack->AddScreen(chart);
}

void MenuCallback(void *data, QString &selection)
{
    (void) data;

    QString sel = selection.toLower();

    if (sel == "wmr_currentcond")
        runCurrentCond();
    else if (sel == "wmr_temperature")
        runChartViewer("temp");
    else if (sel == "wmr_wind")
        runChartViewer("wind");
    else if (sel == "wmr_pressure")
        runChartViewer("pressure");
    else if (sel == "wmr_humidity")
        runChartViewer("humidity");
    else if (sel == "wmr_rain")
        runChartViewer("rain");
}

void runMenu(QString which_menu)
{
    QString themedir = GetMythUI()->GetThemeDir();

    MythThemedMenu *diag = new MythThemedMenu(themedir, which_menu, 
                                              GetMythMainWindow()->GetMainStack(),
                                              "wmr928 menu");

    diag->setCallback(MenuCallback, NULL);
    diag->setKillable();

    if (diag->foundTheme())
    {
        GetMythMainWindow()->GetMainStack()->AddScreen(diag);
    }
    else
    {
        cerr << "Couldn't find theme " << qPrintable(themedir) << endl;
    }
}

int mythplugin_run(void)
{
    // setup a connection to the mythwmrserver
    if (!WMRClient::setupWMRClient())
    {
        return -1;
    }

    runMenu("wmr928menu.xml");

    return 0;
}

int mythplugin_config(void)
{
    WMRSettings settings;
    settings.exec();

    return 0;
}
