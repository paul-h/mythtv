/*
	wmr928 settings.cpp
*/

#include <unistd.h>

// myth
#include <mythtv/mythcontext.h>

// mythwmr928
#include "wmrsettings.h"


static HostLineEdit *WMRServerIP()
{
    HostLineEdit *gc = new HostLineEdit("WMR928ServerIP");
    gc->setLabel(QObject::tr("IP address of the mythwmr928 server"));
    gc->setValue("127.0.0.1");
    gc->setHelpText(QObject::tr("Enter the IP address of the mythwmr928 server "
            "that this frontend should connect to."));
    return gc;
};

static HostLineEdit *WMRServerPort()
{
    HostLineEdit *gc = new HostLineEdit("WMR928ServerPort");
    gc->setLabel(QObject::tr("Port the server runs on"));
    gc->setValue("6549");
    gc->setHelpText(QObject::tr("Unless you've got good reason to, don't "
            "change this."));
    return gc;
};

static HostComboBox *WMRDateFormat()
{
    HostComboBox *gc = new HostComboBox("WMR928DateFormat");
    gc->setLabel(QObject::tr("Date format"));

    QDate sampdate = QDate::currentDate();
    QString sampleStr =
            QObject::tr("Samples are shown using today's date.");

    if (sampdate.month() == sampdate.day())
    {
        sampdate = sampdate.addDays(1);
        sampleStr =
                QObject::tr("Samples are shown using tomorrow's date.");
    }

    gc->addSelection(sampdate.toString("ddd - dd/MM"), "ddd - dd/MM");
    gc->addSelection(sampdate.toString("ddd MMM d"), "ddd MMM d");
    gc->addSelection(sampdate.toString("ddd MMMM d"), "ddd MMMM d");
    gc->addSelection(sampdate.toString("MMM d"), "MMM d");
    gc->addSelection(sampdate.toString("MM/dd"), "MM/dd");
    gc->addSelection(sampdate.toString("MM.dd"), "MM.dd");
    gc->addSelection(sampdate.toString("ddd d MMM"), "ddd d MMM");
    gc->addSelection(sampdate.toString("M/d/yyyy"), "M/d/yyyy");
    gc->addSelection(sampdate.toString("dd.MM.yyyy"), "dd.MM.yyyy");
    gc->addSelection(sampdate.toString("yyyy-MM-dd"), "yyyy-MM-dd");
    gc->addSelection(sampdate.toString("ddd MMM d yyyy"), "ddd MMM d yyyy"); 
    gc->addSelection(sampdate.toString("ddd yyyy-MM-dd"), "ddd yyyy-MM-dd");
    gc->addSelection(sampdate.toString("ddd dd MMM yyyy"), "ddd dd MMM yyyy");
    gc->setHelpText(QObject::tr("Your preferred date format to use on the events screens.") 
            + " " +  sampleStr);
    return gc;
}

static HostComboBox *WMRTimeFormat()
{
    HostComboBox *gc = new HostComboBox("WMR928TimeFormat");
    gc->setLabel(QObject::tr("Time format"));

    QTime samptime = QTime::currentTime();

    gc->addSelection(samptime.toString("hh:mm AP"), "hh:mm AP");
    gc->addSelection(samptime.toString("hh:mm"), "hh:mm");
    gc->addSelection(samptime.toString("hh:mm:ss"), "hh:mm:ss");

    gc->setHelpText(QObject::tr("Your preferred time format to display "
                                "on the events screens."));
    return gc;
}

WMRSettings::WMRSettings()
{
    VerticalConfigurationGroup* vcg1 = new VerticalConfigurationGroup(false);
    vcg1->setLabel(QObject::tr("MythWmr928 Settings"));
    vcg1->addChild(WMRServerIP());
    vcg1->addChild(WMRServerPort());
    vcg1->addChild(WMRDateFormat());
    vcg1->addChild(WMRTimeFormat());
    addChild(vcg1);
}
