/* Implementation of the WMRServer class.
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


#include <iostream>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <fstream>
#include <cstdlib>
#include <string.h>

#include "wmrserver.h"

// the version of the protocol we understand
#define WMR_PROTOCOL_VERSION "1"

#define ADD_STR(list,s)  list += s; list += "[]:[]";

// error messages
#define ERROR_TOKEN_COUNT      "Invalid token count"
#define ERROR_MYSQL_QUERY      "Mysql Query Error"
#define ERROR_MYSQL_ROW        "Mysql Get Row Error"
#define ERROR_FILE_OPEN        "Cannot open file"
#define ERROR_TEMPLATE_OPEN    "Cannot open template file"
#define ERROR_GNUPLOT_FAILED   "Gnuplot failed to run or returned an error"

// some default plot options to use 
// FIXME these should be read from a config file somewhere or DB settings
#define OUTPUTFILE "/home/mythtv/plot.png"
#define DATADIR    "/var/log/wmr928/"
#define XSIZE      "610"
#define YSIZE      "300"

// directory where we can find the template files
// FIXME 
#define TEMPLATEDIR "/usr/share/mythtv/wmr928/"

// where to save the temporory template file
// FIXME
#define TEMPLATEFILE "/home/mythtv/gnuplot.dat"

// wmr928d server address and port
// FIXME
#define WMR928SERVER "192.168.1.20"
#define WMR928PORT   5555

// max image file size we can handle
#define MAX_IMAGE_SIZE (800*600*3)

// path to find the fonts
#define FONT_PATH "/usr/share/mythtv/"
#define FONT_NAME "/usr/share/fonts/dejavu/DejaVuSans.ttf"
#define FONT_SIZE "10"

MYSQL   g_dbConn;
string  g_zmversion = "";
string  g_password = "";
string  g_server = "";
string  g_database = "";
string  g_webPath = "";
string  g_user = "";
string  g_webUser = "";
string  g_binPath = "";

time_t  g_lastDBKick = 0;

void loadWMRConfig(const string &configfile)
{
#if 0
    cout << "loading zm config from " << configfile << endl;
    FILE *cfg;
    char line[512];
    char val[250];

    if ( (cfg = fopen(configfile.c_str(), "r")) == NULL )
    {
        fprintf(stderr,"Can't open %s\n", configfile.c_str());
        exit(1);
    }

    while ( fgets( line, sizeof(line), cfg ) != NULL )
    {
        char *line_ptr = line;
        // Trim off any cr/lf line endings
        size_t chomp_len = strcspn( line_ptr, "\r\n" );
        line_ptr[chomp_len] = '\0';

        // Remove leading white space
        size_t white_len = strspn( line_ptr, " \t" );
        line_ptr += white_len;

        // Check for comment or empty line
        if ( *line_ptr == '\0' || *line_ptr == '#' )
            continue;

        // Remove trailing white space
        char *temp_ptr = line_ptr+strlen(line_ptr)-1;
        while ( *temp_ptr == ' ' || *temp_ptr == '\t' )
        {
            *temp_ptr-- = '\0';
            temp_ptr--;
        }

        // Now look for the '=' in the middle of the line
        temp_ptr = strchr( line_ptr, '=' );
        if ( !temp_ptr )
        {
            fprintf(stderr,"Invalid data in %s: '%s'\n", configfile.c_str(), line );
            continue;
        }

        // Assign the name and value parts
        char *name_ptr = line_ptr;
        char *val_ptr = temp_ptr+1;

        // Trim trailing space from the name part
        do
        {
            *temp_ptr = '\0';
            temp_ptr--;
        }
        while ( *temp_ptr == ' ' || *temp_ptr == '\t' );

        // Remove leading white space from the value part
        white_len = strspn( val_ptr, " \t" );
        val_ptr += white_len;

        strncpy( val, val_ptr, strlen(val_ptr)+1 );
        if ( strcasecmp( name_ptr, "ZM_DB_HOST" ) == 0 )       g_server = val;
        else if ( strcasecmp( name_ptr, "ZM_DB_NAME" ) == 0 )  g_database = val;
        else if ( strcasecmp( name_ptr, "ZM_DB_USER" ) == 0 )  g_user = val;
        else if ( strcasecmp( name_ptr, "ZM_DB_PASS" ) == 0 )  g_password = val;
        else if ( strcasecmp( name_ptr, "ZM_PATH_WEB" ) == 0 ) g_webPath = val;
        else if ( strcasecmp( name_ptr, "ZM_PATH_BIN" ) == 0 ) g_binPath = val;
        else if ( strcasecmp( name_ptr, "ZM_WEB_USER" ) == 0 ) g_webUser = val;
        else if ( strcasecmp( name_ptr, "ZM_VERSION" ) == 0 ) g_zmversion = val;
    }
    fclose(cfg);
#endif
}

void connectToDatabase(void)
{
#if 0
    if (!mysql_init(&g_dbConn))
    {
        cout << "Error: Can't initialise structure: " <<  mysql_error(&g_dbConn) << endl;
        exit(mysql_errno(&g_dbConn));
    }

    if (!mysql_real_connect(&g_dbConn, g_server.c_str(), g_user.c_str(),
         g_password.c_str(), 0, 0, 0, 0))
    {
        cout << "Error: Can't connect to server: " <<  mysql_error(&g_dbConn) << endl;
        exit(mysql_errno( &g_dbConn));
    }

    if (mysql_select_db(&g_dbConn, g_database.c_str()))
    {
        cout << "Error: Can't select database: " << mysql_error(&g_dbConn) << endl;
        exit(mysql_errno(&g_dbConn));
    }
#endif
}

void kickDatabase(bool debug)
{
#if 0
    if (time(NULL) < g_lastDBKick + DB_CHECK_TIME)
        return;

    if (debug)
        cout << "Kicking database connection" << endl;

    g_lastDBKick = time(NULL);

    if (mysql_query(&g_dbConn, "SELECT NULL;") == 0)
    {
        MYSQL_RES *res = mysql_store_result(&g_dbConn);
        if (res)
            mysql_free_result(res);
        return;
    }

    cout << "Lost connection to DB - trying to reconnect" << endl;

    // failed so try to reconnect to the DB
    mysql_close(&g_dbConn);
    connectToDatabase();
#endif
}

WMRServer::WMRServer(int sock, bool debug)
{
    if (debug)
        cout << "Using server protocol version '" << WMR_PROTOCOL_VERSION << "'\n";

    m_sock = sock;
    m_debug = debug;
}

WMRServer::~WMRServer()
{
    if (m_debug)
        cout << "WMRServer destroyed\n";
}

void WMRServer::tokenize(const string &command, vector<string> &tokens)
{
    string token = "";
    tokens.clear();
    string::size_type startPos = 0;
    string::size_type endPos = 0;

    while((endPos = command.find("[]:[]", startPos)) != string::npos)
    {
        token = command.substr(startPos, endPos - startPos);
        tokens.push_back(token);
        startPos = endPos + 5;
    }

    // make sure we add the last token
    if (endPos != command.length())
    {
        token = command.substr(startPos);
        tokens.push_back(token);
    }
}

void WMRServer::processRequest(char* buf, int nbytes)
{
#if 0
    // first 8 bytes is the length of the following data
    char len[9];
    memcpy(len, buf, 8);
    len[8] = '\0';
    int dataLen = atoi(len);
#endif

    buf[nbytes] = '\0';
    string s(buf+8);
    vector<string> tokens;
    tokenize(s, tokens);

    if (tokens.size() == 0)
        return;

    if (m_debug)
        cout << "Processing: '" << tokens[0] << "'" << endl; 

    if (tokens[0] == "HELLO")
        handleHello();
    else if (tokens[0] == "GET_CURRENT_CONDITIONS")
        handleGetCurrentConditions();
    else if (tokens[0] == "GET_CHART")
        handleGetChart(tokens);
    else
        send("UNKNOWN_COMMAND");
}

bool WMRServer::send(const string s) const
{
    // send length
    uint32_t len = s.size();
    char buf[9];
    sprintf(buf, "%8d", len);
    int status = ::send(m_sock, buf, 8, MSG_NOSIGNAL);
    if (status == -1)
        return false;

    // send message
    status = ::send(m_sock, s.c_str(), s.size(), MSG_NOSIGNAL);
    if ( status == -1 )
        return false;
    else
        return true;
}

bool WMRServer::send(const string s, const unsigned char *buffer, int dataLen) const
{
    // send length
    uint32_t len = s.size();
    char buf[9];
    sprintf(buf, "%8d", len);
    int status = ::send(m_sock, buf, 8, MSG_NOSIGNAL);
    if (status == -1)
        return false;

    // send message
    status = ::send(m_sock, s.c_str(), s.size(), MSG_NOSIGNAL);
    if ( status == -1 )
        return false;

    // send data
    status = ::send(m_sock, buffer, dataLen, MSG_NOSIGNAL);
    if ( status == -1 )
        return false;

    return true;
}

void WMRServer::sendError(string error)
{
    string outStr("");
    ADD_STR(outStr, string("ERROR - ") + error);
    send(outStr);
}

bool WMRServer::getCurrentConditions(string &s)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    portno = WMR928PORT;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        cout << "ERROR opening socket" << endl;
        return false;
    }

    server = gethostbyname(WMR928SERVER);
    if (server == NULL) 
    {
        fprintf(stderr,"ERROR, no such host\n");
        return false;
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        return false;
    }

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
    {
        perror("ERROR reading from socket");
        return false;
    }

    if (m_debug)
        printf("%s\n",buffer);

    string temp(buffer);

    s = temp;
    return true;
}

void WMRServer::setDefaultPlotOptions(void)
{
    m_plotOptions.outputFile = OUTPUTFILE;
    m_plotOptions.dataDir = DATADIR;
    m_plotOptions.startTime = "20070929000000";
    m_plotOptions.endTime = "20070930000000";
    m_plotOptions.resolution = "min5";
    m_plotOptions.xsize = XSIZE;
    m_plotOptions.ysize = YSIZE;
    m_plotOptions.xformat = "%H:%M\\n%a\\n%d.%b";
    m_plotOptions.yformat = "%.1f °C";
    m_plotOptions.additional = "";
    m_plotOptions.fontname = FONT_NAME;
	m_plotOptions.fontsize = FONT_SIZE;
}

static void replaceString(string &in, string oldStr, string newStr)
{
    string::size_type loc;
    while((loc = in.find(oldStr, 0)) != string::npos)
        in.replace(loc, oldStr.size(), newStr);
}

string WMRServer::formatDateTime(string datetime)
{
    string result("");
    struct tm t;
    char buf[100];

    bzero(&t, sizeof(t));
    t.tm_year = atoi(datetime.substr(0, 4).c_str()) - 1900;
    t.tm_mon = atoi(datetime.substr(4, 2).c_str()) - 1;
    t.tm_mday = atoi(datetime.substr(6, 2).c_str());
    t.tm_hour = atoi(datetime.substr(8, 2).c_str());
    t.tm_min = atoi(datetime.substr(10, 2).c_str());
    t.tm_sec = atoi(datetime.substr(12, 2).c_str());

    if (strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M", &t) > 0)
        result = string(buf);

    return result;
}

bool WMRServer::createTemplate(const string &filename)
{
    string line;

    // open the output template file
    ofstream ofs(TEMPLATEFILE);

    if (ofs == false)
    {
        cout << "Failed to open the output template file: '"
             << TEMPLATEFILE << "'" << endl;
        return false;
    }

    // load the template file line by line
    ifstream ifs(string(TEMPLATEDIR + filename).c_str());

    if (ifs == false)
    {
        cout << "Failed to open the input template file: '"
             << TEMPLATEDIR + filename << "'" << endl;
        return false;
    }

    while(getline(ifs, line))
    {
        // do some option substitutions
        replaceString(line, "%OUTPUT%", m_plotOptions.outputFile);
        replaceString(line, "%DATADIR%", m_plotOptions.dataDir);
        replaceString(line, "%STARTTIME%", formatDateTime(m_plotOptions.startTime));
        replaceString(line, "%ENDTIME%", formatDateTime(m_plotOptions.endTime));
        replaceString(line, "%XRANGEFROM%", m_plotOptions.startTime);
        replaceString(line, "%XRANGETO%", m_plotOptions.endTime);
        replaceString(line, "%RES%", m_plotOptions.resolution);
        replaceString(line, "%XSIZE%", m_plotOptions.xsize);
        replaceString(line, "%YSIZE%", m_plotOptions.ysize);
        replaceString(line, "%XFORMAT%", m_plotOptions.xformat);
        replaceString(line, "%YFORMAT%", m_plotOptions.yformat);
        replaceString(line, "%ADDITIONAL%", m_plotOptions.additional);
        replaceString(line, "%FONTNAME%", m_plotOptions.fontname);
        replaceString(line, "%FONTSIZE%", m_plotOptions.fontsize);

        // save the line to the new template file
        ofs << line << endl;
    }

    return true;
}

bool WMRServer::runGnuplot(void)
{
    string command("gnuplot ");
    command += TEMPLATEFILE;
    int res = system(command.c_str());

    if (res == -1)
        return false;

    res = WEXITSTATUS(res);

    if (res != 0)
        return false;

    return true;
}

void WMRServer::handleHello()
{
    // just send OK so the client knows all is well
    // followed by the protocol version we understand
    string outStr("");
    ADD_STR(outStr, "OK");
    ADD_STR(outStr, WMR_PROTOCOL_VERSION);
    send(outStr);
}

void WMRServer::handleGetCurrentConditions(void)
{
    string outStr("");
    string s("");
    string line("");

    getCurrentConditions(s);

    ADD_STR(outStr, "OK")

    for (uint x = 0; x < s.length(); x++)
    {
        if (s[x] == '\n')
        {
            if (line.length() > 0)
                ADD_STR(outStr, line);

            line = "";
        }
        else
            line += s[x];
    }

    send(outStr);
}

void WMRServer::handleGetChart(vector<string> tokens)
{
    string outStr("");

    ADD_STR(outStr, "OK")

    if (tokens.size() != 7)
    {
        sendError(ERROR_TOKEN_COUNT);
        return;
    }

    string type(tokens[1]);
    string startTime(tokens[2]);
    string endTime(tokens[3]);
    string res(tokens[4]); 
    string xsize(tokens[5]);
    string ysize(tokens[6]);

    if (m_debug)
        cout << "Getting chart type: " << type  << endl
             << "starttime: " << startTime  << endl
             << "endtime: " << endTime << endl
             << "resolution: " << res << endl
             << "size: " << xsize << "x" << ysize 
             << endl;

    // setup the plot options
    setDefaultPlotOptions();

    m_plotOptions.startTime = startTime;
    m_plotOptions.endTime = endTime;
    m_plotOptions.resolution = res;
    m_plotOptions.xsize = xsize;
    m_plotOptions.ysize = ysize;

    if (res == "day1")
        m_plotOptions.xformat = "%a\\n\%d.%b";
    else if (res == "month1")
    {
        m_plotOptions.xformat = "%b\\n%Y";
        m_plotOptions.additional = "set mxtics 2\nset grid mxtics\n";
    }

    // create template file with given parameters 
    if (!createTemplate(type + ".dat"))
    {
        sendError(ERROR_TEMPLATE_OPEN + string(" - ") + type);
        return;
    }

    // run gnuplot
    if (!runGnuplot())
    {
        sendError(ERROR_GNUPLOT_FAILED);
        return;
    }

    // send created png to client
    static unsigned char buffer[MAX_IMAGE_SIZE];
    char str[100];

    // try to open the char png file
    FILE *fd;
    int fileSize = 0;
    if ((fd = fopen(m_plotOptions.outputFile.c_str(), "r" )))
    {
        fileSize = fread(buffer, 1, sizeof(buffer), fd);
        fclose(fd);
    }
    else
    {
        cout << "Can't open " << m_plotOptions.outputFile << ": " 
             << strerror(errno) << endl;
        sendError(ERROR_FILE_OPEN + string(" - ") + m_plotOptions.outputFile + 
                " : " + strerror(errno));
        return;
    }

    if (m_debug)
        cout << "PNG size: " <<  fileSize << endl;

    // get the file size
    sprintf(str, "%d", fileSize);
    ADD_STR(outStr, str)

    // send the data
    send(outStr, buffer, fileSize);
}
