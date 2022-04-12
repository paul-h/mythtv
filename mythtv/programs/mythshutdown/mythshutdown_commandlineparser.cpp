#include <QString>

#include "mythcorecontext.h"
#include "mythshutdown_commandlineparser.h"

MythShutdownCommandLineParser::MythShutdownCommandLineParser() :
    MythCommandLineParser(MYTH_APPNAME_MYTHSHUTDOWN)
{ MythShutdownCommandLineParser::LoadArguments(); }

void MythShutdownCommandLineParser::LoadArguments(void)
{
    add("-l", "oldlock", false, "disable shutdown", "")
        ->SetRemoved("Please use the full argument '--lock' instead.");

    addHelp();
    addVersion();
    addLogging("none", LOG_ERR);

    CommandLineArg::AllowOneOf( QList<CommandLineArg*>()
             << add(QStringList{"-p", "--startup"},
                "startup", false,
                "Check startup status",
                "Check startup status\n"
                "   returns 0 - automatic startup\n"
                "           1 - manual startup")

        // backend shutdown options
        << add(QStringList{"-w", "--setwakeup"},
               "setwakeup", "",
               "Set the wakeup time (yyyy-MM-ddThh:mm:ss) "
               "default is in local time", "")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-t", "--setscheduledwakeup"},
               "setschedwakeup", false,
               "Set wakeup time to the next scheduled recording", "")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-q", "--shutdown"},
               "shutdown", false,
               "Apply wakeup time to nvram and shutdown.", "")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-x", "--safeshutdown"},
               "safeshutdown", false,
               "Check if shutdown is possible, and shutdown", "")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-s", "--status"},
               "status", 1,
               "check current status",
               "check current status depending on input\n"
               "   input 0 - dont check recording status\n"
               "         1 - do check recording status\n\n"
               " returns 0 - Idle\n"
               "         1 - Transcoding\n"
               "         2 - Commercial Detection\n"
               "         4 - Grabbing EPG data\n"
               "         8 - Recording (only valid if input=1)\n"
               "        16 - Locked\n"
               "        32 - Jobs running or pending\n"
               "        64 - In daily wakeup/shutdown period\n"
               "       128 - Less than 15 minutes to next wakeup period\n"
               "       255 - Setup is running")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-c", "--check"},
               "check", 1,
               "Check whether shutdown is possible",
               "Check whether shutdown is possible depending on input\n"
               "   input 0 - dont check recording status\n"
               "         1 - do check recording status\n\n"
               " returns 0 - OK to shut down\n"
               "         1 - not OK, idle check reset")
               ->SetGroup("Backend Shutdown")
        << add("--lock" , "lock", false, "disable shutdown", "")
               ->SetGroup("Backend Shutdown")
        << add(QStringList{"-u", "--unlock"}, "unlock", false, "enable shutdown", "")
               ->SetGroup("Backend Shutdown")

        // frontend shutdown options
        << add("--lockfrontend" , "lockfrontend", false,
               "disable frontend shutdown (only useful on frontend only systems).", "")
               ->SetGroup("Frontend Shutdown")
        << add("--unlockfrontend", "unlockfrontend", false,
               "enable frontend shutdown (only useful on frontend only systems).", "")
               ->SetGroup("Frontend Shutdown")
        << add("--checkfrontend",
               "checkfrontend", false,
               "Check whether this frontend system can shutdown. (only useful on frontend only systems)",
               " returns 0 - OK to shut down (unlocked)\n"
               "         1 - not OK to shutdown (locked)")
               ->SetGroup("Frontend Shutdown"));

    // The localtime command line parameter exists solely to make scripts
    // using this executable self documenting.

    CommandLineArg::AllowOneOf( QList<CommandLineArg*>()
         << add("--utc",
                "utc", false,
                "Specify that the wakeup time is in utc", "")
         << add("--localtime",
                "localtime", false,
                "Specify that the wakeup time is in local time", "") );
}

