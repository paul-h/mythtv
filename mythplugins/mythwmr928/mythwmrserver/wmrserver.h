/* Definition of the ZMServer class.
 * ============================================================
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

#ifndef WMRSERVER_H
#define WMRSERVER_H


#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <mysql/mysql.h>

using namespace std;

extern void loadWMRConfig(const string &configfile);
extern void connectToDatabase(void);
extern void kickDatabase(bool debug);

// these are shared by all WMRServer's
extern MYSQL   g_dbConn;
extern string  g_wmrversion;
extern string  g_password;
extern string  g_server;
extern string  g_database;
extern string  g_webPath;
extern string  g_user;
extern string  g_webUser;
extern string  g_binPath;

#define DB_CHECK_TIME 60
extern time_t  g_lastDBKick;

typedef struct
{
    string outputFile;
    string dataDir;
    string startTime;
    string endTime;
    string resolution;
    string xsize;
    string ysize;
    string xformat;
    string yformat;
    string additional;
	string fontname;
	string fontsize;
} PlotOptions;

class WMRServer
{
  public:
    WMRServer(int sock, bool debug);
    ~WMRServer();

    void processRequest(char* buf, int nbytes);

  private:
    string getWMRSetting(const string &setting);
    bool getCurrentConditions(string &s);
    bool send(const string s) const;
    bool send(const string s, const unsigned char *buffer, int dataLen) const;
    void sendError(string error);
    void tokenize(const string &command, vector<string> &tokens);
    void setDefaultPlotOptions(void);
    string formatDateTime(string datetime);
    bool createTemplate(const string &filename);
    bool runGnuplot(void);

    void handleHello(void);
    void handleGetCurrentConditions(void);
    void handleGetChart(vector<string> tokens);

    bool                 m_debug;
    int                  m_sock;
    PlotOptions          m_plotOptions;
};


#endif
