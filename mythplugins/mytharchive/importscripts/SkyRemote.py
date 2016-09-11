#!/usr/bin/env python
#
# Python script to get recording list from a Sky+HD Box via uPnP
#
# January 2015


import httplib, sys, time, os, json, re, getopt
import xml.dom.minidom
from socket import *
import HTMLParser

skyAddress = "192.168.1.120"                            # IP Address of Sky+HD Box
skyPort =    "49153"
skyChannelsFile = "/etc/mythtv/sky_channel_guide.txt"   # Location of Channel Data
sleepTime = 2.0                                         # Amount of time to sleep between changing channel,
                                                        # and checking if change was successful.
DEFAULT_IP = "239.255.255.250"
DEFAULT_PORT = 1900
MAX_RECV = 8192
UPNP_VERSION = '1.0'
STARS = '****************************************************************'
ENUM_HOSTS = {}

#############################################################
# Write an xml file to disc

def WriteXMLToFile(myDOM, filename):
    #Save the XML file to disk for use later on
    f=open(filename, 'w')

    if sys.hexversion >= 0x020703F0:
        f.write(myDOM.toprettyxml(indent="    ", encoding="UTF-8"))
    else:
        f.write(myDOM.toxml(encoding="UTF-8"))

    f.close()

#############################################################
# Returns the text contents from a given XML element.

def getText(node):
    """Returns the text contents from a given XML element."""
    if node.childNodes.length > 0:
        return node.childNodes[0].data
    else:
        return ""

#Create new UDP socket on ip, bound to port
def createNewListener(ip, port):
    try:
        newsock = socket(AF_INET, SOCK_DGRAM,IPPROTO_UDP)
        newsock.setsockopt(SOL_SOCKET, SO_REUSEADDR, 1)
        newsock.bind((ip, port))
        return newsock
    except:
        return False

#Send network data
def send(ip, port, data, socket):
    try:
        socket.sendto(data, (ip, port))
        return True
    except Exception, e:
        print "sendto method failed for %s:%d : %s" % (ip, port, e)
        return False

#Listen for network data
def listen(size, socket):
    try:
        return socket.recv(size)
    except:
        print "Listen method failed: %s" % e
        return False

#Parse a URL, return the host and the page
def parseURL(url):
    delim = '://'
    host = False
    page = False

    #Split the host and page
    try:
        (host,page) = url.split(delim)[1].split('/',1)
        page = '/' + page
    except:
        #If '://' is not in the url, then it's not a full URL, so assume that it's just a relative path
        page = url

    return (host,page)

#Pull the header info for the specified HTTP header - case insensitive
def parseHeader(data, header):
    delimiter = "%s:" % header
    defaultRet = False

    lowerDelim = delimiter.lower()
    dataArray = data.split("\r\n")

    #Loop through each line of the headers
    for line in dataArray:
        lowerLine = line.lower()
        #Does this line start with the header we're looking for?
        if lowerLine.startswith(lowerDelim):
            try:
                return line.split(':',1)[1].strip()
            except:
                print "Failure parsing header data for %s" % header
    return defaultRet

#Parses SSDP notify and reply packets, and populates the ENUM_HOSTS dict
def parseSSDPInfo(data):
    hostFound = False
    foundLocation = False
    messageType = False
    xmlFile = False
    host = False
    page = False
    upnpType = None
    knownHeaders = {
            'NOTIFY' : 'notification',
            'HTTP/1.1 200 OK' : 'reply'
    }

    #Is the SSDP packet a notification, a reply, or neither?
    for text,messageType in knownHeaders.iteritems():
        if data.upper().startswith(text):
            break
        else:
            messageType = False

    #If this is a notification or a reply message...
    if messageType != False:
        #Get the host name and location of it's main UPNP XML file
        xmlFile = parseHeader(data,"LOCATION")
        upnpType = parseHeader(data,"SERVER")

        (host,page) = parseURL(xmlFile)

        if upnpType.find('SKY') == -1:
            return False

        #Update xmlFile to be description0.xml. Sky has two service points, and for now we only support the SkyPlay endpoint
        #xmlFile = "http://"+host+"/description0.xml"

        #Sanity check to make sure we got all the info we need
        if xmlFile == False or host == False or page == False:
            print 'ERROR parsing received header:'
            print STARS
            print data
            print STARS
            print ''
            return False

        #Get the protocol in use (i.e., http, https, etc)
        protocol = xmlFile.split('://')[0]+'://'

        #Check if we've seen this host before; add to the list of hosts if:
        #   1. This is a new host
        #   2. We've already seen this host, but the uniq hosts setting is disabled
        for hostID,hostInfo in ENUM_HOSTS.iteritems():
            if hostInfo['name'] == host:
                hostFound = True
                return False

        if hostFound == False:
            #Get the new host's index number and create an entry in ENUM_HOSTS
            index = len(ENUM_HOSTS)
            ENUM_HOSTS[index] = {
                        'name' : host,
                        'dataComplete' : False,
                        'proto' : protocol,
                        'xmlFile' : xmlFile,
                        'serverType' : None,
                        'upnpServer' : upnpType,
                        'deviceList' : {}
            }

        #Print out some basic device info
        print "Found a Sky Box at "+host[:-6]
        print "xmlFile: %s" % xmlFile
        return "complete"

#Actively search for UPNP devices
def msearch(ip, port, st, msearchHeaders):

    #Build the request
    request = "M-SEARCH * HTTP/1.1\r\n"\
              "HOST:%s:%d\r\n"\
              "ST:%s\r\n" % (ip, port, st)
    for header,value in msearchHeaders.iteritems():
        request += header + ':' + value + "\r\n"    
    request += "\r\n" 

    print "Entering discovery mode for '%s', Ctl+C to stop..." % st
    print ''

    server = createNewListener('', port)
    if server == False:
        print 'Failed to bind port %d' % port
        return

    send(ip, port, request, server)

    while True:
        try:
            parseSSDPInfo(listen(1024, server))
        except Exception, e:
            print 'Discover mode halted... '
            server.close()
            break

# Parse through tab delimited channel listing and return HEX Channel No
# 
def updateSkyChannels():

    try:
        conn = httplib.HTTPConnection("tv.sky.com")
        conn.request("GET", "/channel/index")
        response = conn.getresponse()
        data = response.read()
        conn.close()
    except Exception as inst:
        sys.exit("Error Fetching Sky Channel Data: %s" % inst)

    channels = json.loads(data)

    channellist = channels["init"] ["channels"]

    try:
        if os.path.isfile(skyChannelsFile):
            os.remove(skyChannelsFile)
        channelFile = open(skyChannelsFile, 'w')

    except Exception as inst:
        sys.exit("Error Creating Channel File: %s" % inst)

    channelFile.write ("No\tDEC\tHEX\tGenre\tHD\tFree/Premium\tChannel Name\n")

    for channel in channellist:
        lcn = channel["lcn"]
        no = channel["c"][1]
        dec = channel["c"][0]
        hex2 = channel["c"][0]
        genre = channel["c"][2]
        hd = channel["c"][3]
        fta = channel["pt"]
        name = channel["t"]

        channelFile.write ("%s\t%s\t%X\t%s\t%s\t%s\t%s\n" % (no, dec, int(hex2), genre, hd, fta, name))

    channelFile.close()

#Extract the contents of a single XML tag from the data
def extractSingleTag(data, tag):
    startTag = "<%s" % tag
    endTag = "</%s>" % tag

    try:
        tmp = data.split(startTag)[1]
        index = tmp.find('>')
        if index != -1:
            index += 1
            return tmp[index:].split(endTag)[0].strip()
    except:
        pass
        return None

def sendSOAP(hostName,serviceType,controlURL,actionName,actionArguments):
    print "hostName: %s" % hostName
    print "serviceType: %s" % serviceType
    print "controlURL: %s" % controlURL
    print "actionName: %s" % actionName
    print "actionArguments: %s" % actionArguments

    soapEnd = re.compile('<\/.*:envelope>')
    argList = ''
    soapResponse = ''

    if '://' in controlURL:
        urlArray = controlURL.split('/',3)
    if len(urlArray) < 4:
        controlURL = '/'
    else:
        controlURL = '/' + urlArray[3]


    soapRequest = 'POST %s HTTP/1.1\r\n' % controlURL

    #Check if a port number was specified in the host name; default is port 80
    if ':' in hostName:
        hostNameArray = hostName.split(':')
        host = hostNameArray[0]
        try:
            port = int(hostNameArray[1])
        except:
            print 'Invalid port specified for host connection:',hostName[1]
            return False
    else:
        host = hostName
        port = 80

    #Create a string containing all of the SOAP action's arguments and values
    for arg,(val,dt) in actionArguments.iteritems():
        argList += '<%s>%s</%s>' % (arg,val,arg)

    #Create the SOAP request
    soapBody = '<?xml version="1.0" encoding="utf-8"?>'\
               '<s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">'\
               '<s:Body>'\
               '<u:%s xmlns:u="%s">'\
               '%s'\
               '</u:%s>'\
               '</s:Body>'\
               '</s:Envelope>' % (actionName,serviceType,argList,actionName)

    actualServiceType = serviceType[:-2] + ":2"

    #Specify the headers to send with the request
    headers = {
        'Host':hostName[:-6],
        'Content-Length':len(soapBody),
        'Content-Type':'text/xml; charset="utf-8"',
        'User-Agent':'SKYAndroid',
        'SOAPACTION':'"%s#%s"' % (actualServiceType,actionName),
        'connection':'close'
        }


    #Generate the final payload
    for head,value in headers.iteritems():
        soapRequest += '%s: %s\r\n' % (head,value)

    soapRequest += '\r\n%s' % soapBody

    #print soapRequest

    #Send data and go into recieve loop
    try:
        sock = socket(AF_INET,SOCK_STREAM)
        sock.connect((host,port))
        sock.send(soapRequest)
        while True:
            data = sock.recv(MAX_RECV)
            if not data:
                break
            else:
                soapResponse += data
                if soapEnd.search(soapResponse.lower()) != None:
                    break
        sock.close()

        (header,body) = soapResponse.split('\r\n\r\n',1)

        if not header.upper().startswith('HTTP/1.1 200'):
            print 'SOAP request failed with error code:',header.split('\r\n')[0].split(' ',1)[1]
            errorMsg = extractSingleTag(body,'errorDescription')
            if errorMsg:
                print 'SOAP error message:',errorMsg
                print body
                return False
        else:
            return body
    except Exception, e:
        print 'Caught socket exception:',e
        sock.close()
        return False
    except KeyboardInterrupt:
        sock.close()

    return False

# Send Standby command to skyHD Box
#
def standby():

    addr = skyAddress
    port = int(skyPort)
    data = 'SKY 000.001\n'

    sock = socket(AF_INET, SOCK_STREAM)
    sock.connect((addr, port))
    sock.send(data)

    return True

def findSkyChannel(channelFind):

    if not os.path.isfile(skyChannelsFile):
        print "Data File '%s' Does Not Exist.  Fetching New Data" % skyChannelsFile
        updateSkyChannels()

    try:
        channelsFile = open (skyChannelsFile, "r")
        for line in channelsFile:
            rawdata = line.split('\t')
            if rawdata[0] == channelFind:
                channelsFile.close()
                return (rawdata[2], rawdata[6])

        channelsFile.close()
        return ("0", "Not Found")

    except Exception as inst:
        sys.exit("Error in findSkyChannel: %s" % inst)

# Get Current Channel from skyHD Box
#
def getSkyChannel():

    host = skyAddress + ':' + skyPort
    serviceType = "urn:schemas-nds-com:service:SkyPlay:2"
    controlURL = "http://%s:%s/444D5276-3247-536B-7943-0019fb0f62f4SkyPlay" % (skyAddress, skyPort)
    actionName = "GetMediaInfo"
    actionArguments = {'InstanceID': ('0', 'ui4')}

    response = sendSOAP(host, serviceType, controlURL, actionName, actionArguments)

    # print "++++++++++++++++++"
    # print response

    if (response):
        res = -1
        dom = xml.dom.minidom.parseString(response)
        channel = dom.getElementsByTagName('CurrentURI')

        if channel[0].firstChild != None:
            res = channel[0].firstChild.data.split('//')[1]

        if res != -1:
            print "channel is: %s" % channel
        else:
            print "Cannot get current channel, maybe the SkyHD box is in standby?"

        return res

    return False

# Change channel on SkyHD Box via IP given HEX Channel ID
#
def changeSkyChannel(channelId):
    chanStr = 'xsi://%s' % channelId

    host = skyAddress + ':' + skyPort
    serviceType = "urn:schemas-nds-com:service:SkyPlay:2"
    controlURL = "http://%s:%s/444D5276-3247-536B-7943-0019fb0f62f4SkyPlay" % (skyAddress, skyPort)
    actionName = "SetAVTransportURI"
    actionArguments = {'InstanceID': ('0', 'ui4'), 'CurrentURIMetaData': ('', 'string'), 'CurrentURI': (chanStr, 'string')}

    response = sendSOAP(host, serviceType, controlURL, actionName, actionArguments)

#    print "++++++++++++++++++"
#    print response

    time.sleep(sleepTime)

    if channelId == getSkyChannel():
        return 0

    return 1

# Test play file on skyHD Box
#
def testPlayFile(filename):

    host = skyAddress + ':' + skyPort
    serviceType = "urn:schemas-nds-com:service:SkyPlay:2"
    controlURL = "http://%s:%s/444D5276-3247-536B-7943-0019fb0f62f4SkyPlay" % (skyAddress, skyPort)
    actionName = "SetAVTransportURI"
    currentURI = "%s?position=0&amp;speed=1" % filename
    actionArguments = {'InstanceID': ('0', 'ui4'), 'CurrentURI': (currentURI, 'string'), 'CurrentURIMetaData': ('NOT_IMPLEMTED', 'string') }

    response = sendSOAP(host, serviceType, controlURL, actionName, actionArguments)

    if response == False:
        return 1

    return 0

# Get planner from skyHD Box
#
def GetPlanner(xmlfile):

    host = skyAddress + ':' + skyPort
    serviceType = "urn:schemas-nds-com:service:SkyBrowse:2"
    controlURL = "http://%s:%s/444D5376-3247-536B-7953-0019fb0f62f4SkyBrowse" % (skyAddress, skyPort)
    actionName = "Browse"
    startingIndex = 0
    count = 0

    impl = xml.dom.minidom.getDOMImplementation()
    itemsDOM = impl.createDocument(None, "recordings", None)
    top_element = itemsDOM.documentElement

    while True:
        actionArguments = {'ObjectID': ('3', 'ui4'), 'BrowseFlag': ('BrowseDirectChildren', 'string'), 'Filter': ('*', 'string'), 'RequestedCount': ('25', 'ui4'), 'StartingIndex': (str(startingIndex), 'ui4'), 'SortCriteria': ('', 'string') }

        response = sendSOAP(host, serviceType, controlURL, actionName, actionArguments)

        if (response):
            #print response
            #h = HTMLParser.HTMLParser()
            #response = h.unescape(response)
            response = response.replace('&lt;', '<')
            response = response.replace('&gt;', '>')
            response = response.replace('&amp;', '&')
            response = response.replace('&quot;', '"')
            response = response.replace('&apos;', '\'')

            #print response
            dom = xml.dom.minidom.parseString(response)
            totalMatches = int(getText(dom.getElementsByTagName('TotalMatches')[0]))
            numberReturned = int(getText(dom.getElementsByTagName('NumberReturned')[0]))
            #print "Total Matches: %s" % totalMatches
            #print "Number Returned: %s" % numberReturned

            if numberReturned == 0:
                print "ERROR: 0 Matches Returned!"
                break;

            startingIndex += numberReturned

            print dom.toprettyxml(indent="    ", encoding="UTF-8")

            items = dom.getElementsByTagName('item')
            for item in items:
                count += 1
                recordingNode = itemsDOM.createElement("recording")

                status = int(getText(item.getElementsByTagName('vx:X_recStatus')[0]))
                node = itemsDOM.createElement("status")
                node.appendChild(itemsDOM.createTextNode(str(status)))
                recordingNode.appendChild(node)
                print "Status: %s" % status

                node = itemsDOM.createElement("No")
                node.appendChild(itemsDOM.createTextNode(str(count)))
                recordingNode.appendChild(node)
                print "No: %s" % count

                node = itemsDOM.createElement("title")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('dc:title')[0])))
                recordingNode.appendChild(node)
                print "title: %s" % getText(item.getElementsByTagName('dc:title')[0])

                node = itemsDOM.createElement("season")
                node.appendChild(itemsDOM.createTextNode("0"))
                recordingNode.appendChild(node)
                print "season: %s" % "0"

                node = itemsDOM.createElement("episode")
                node.appendChild(itemsDOM.createTextNode("0"))
                recordingNode.appendChild(node)
                print "episode: %s" % "0"

                node = itemsDOM.createElement("channelno")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('upnp:channelNr')[0])))
                recordingNode.appendChild(node)
                print "channelno: %s" % getText(item.getElementsByTagName('upnp:channelNr')[0])

                node = itemsDOM.createElement("channelname")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('upnp:channelName')[0])))
                recordingNode.appendChild(node)
                print "channelname: %s" % getText(item.getElementsByTagName('upnp:channelName')[0])

                startTime = ""
                if len(item.getElementsByTagName('upnp:scheduledStartTime')) > 0 :
                    startTime = getText(item.getElementsByTagName('upnp:scheduledStartTime')[0])
                else:
                    startTime = getText(item.getElementsByTagName('upnp:recordedStartDateTime')[0])

                node = itemsDOM.createElement("starttime")
                node.appendChild(itemsDOM.createTextNode(startTime))
                recordingNode.appendChild(node)
                print "start time: %s" % startTime 

                node = itemsDOM.createElement("duration")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('upnp:scheduledDuration')[0])))
                recordingNode.appendChild(node)
                print "duration: %s" % getText(item.getElementsByTagName('upnp:scheduledDuration')[0])

                actualDuration = ""
                if status == 1 or status == 2 or status == 3:
                    actualDuration = "N/A"
                else:
                    actualDuration = getText(item.getElementsByTagName('upnp:recordedDuration')[0])
                node = itemsDOM.createElement("actualduration")
                node.appendChild(itemsDOM.createTextNode(actualDuration))
                recordingNode.appendChild(node)
                print "actual duration: %s" % actualDuration

                node = itemsDOM.createElement("description")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('dc:description')[0])))
                recordingNode.appendChild(node)
                print "description: %s" % getText(item.getElementsByTagName('dc:description')[0])

                node = itemsDOM.createElement("filename")
                node.appendChild(itemsDOM.createTextNode(getText(item.getElementsByTagName('res')[0])))
                recordingNode.appendChild(node)
                print "filename: %s" % getText(item.getElementsByTagName('res')[0])

                node = itemsDOM.createElement("category")
                category = int(getText(item.getElementsByTagName('vx:X_genre')[0]))
                if category == 1:
                    strCategory = "Specialist"
                elif category == 2:
                    strCategory = "Kids"
                elif category == 3:
                    strCategory = "Entertainment"
                elif category == 4:
                    strCategory = "Unknown"
                elif category == 5:
                    strCategory = "News"
                elif category == 6:
                    strCategory = "Movies"
                elif category == 7:
                    strCategory = "Sports"
                elif category == 8:
                    strCategory = "Lifestyle"
                elif category == 9:
                    strCategory = "Adult"
                elif category == 10:
                    strCategory = "Gaming-Dating"
                elif category == 11:
                    strCategory = "Documentaries"
                elif category == 12:
                    strCategory = "Unknown"
                elif category == 13:
                    strCategory = "Unknown"
                elif category == 16:
                    strCategory = "Music"
                else:
                    strCategory = "Unknown"

                node.appendChild(itemsDOM.createTextNode(strCategory))
                recordingNode.appendChild(node)
                print "category: %s" % strCategory
                print STARS

                top_element.appendChild(recordingNode)
                WriteXMLToFile(itemsDOM, xmlfile)

            if count >= totalMatches:
                break;
        else:
            print "NO RESONSE from soap request!!"
            return 1;

    print "### Exiting with 0 result ###"
    return 0

def changeChannel(channel):
    channelId, channelName = findSkyChannel(channel)
    print "channelId: %s, channelName: %s" % (channelId, channelName)
    if channelId == "0":
        print "Unable to find HEX channel No.  Exiting"
        return 1

    if channelId == getSkyChannel():
        print "Sky+HD is already tuned to %s (%s - %s).  No Need to Change" % (channelName, channel, channelId)
        return 0

    if changeSkyChannel(channelId) == 0:
        print "Sky+HD Channel Changed to %s (%s - %s)" % (channelName, channel, channelId)
        return 0
    else:
        print "Sky+HD Channel Change to %s (%s - %s) unsuccessfull" % (channelName, channel, channelId)
        return 1

def testMSearch():
    msearchHeaders = {
        'MAN' : '"ssdp:discover"',
        'MX'  : '3'  
    }
    st = 'urn:schemas-nds-com:device:SkyServe:2'
    msearch(DEFAULT_IP, DEFAULT_PORT, st, msearchHeaders)

    time.sleep(4)
    st = 'urn:schemas-nds-com:device:SkyRemote:2'
    msearch(DEFAULT_IP, DEFAULT_PORT, st, msearchHeaders)

    time.sleep(4)
    st = 'urn:schemas-nds-com:device:SkyControl:2'
    msearch(DEFAULT_IP, DEFAULT_PORT, st, msearchHeaders)

def usage():
    print '\nUsage: %s --changechannel <channel> - Change Channel on Sky+HD Box\n' % sys.argv[0]
    print 'or  : %s --update    - Update Channel Data\n' % sys.argv[0]
    print 'or  : %s --play <filename> - Start a recording playing on the Sky+HD Box\n' % sys.argv[0]

#
# Main Program
#
if __name__ == '__main__':

   # process any command line options
    try:
        opts, args = getopt.getopt(sys.argv[1:], "c:g:hl:p:stu", ["changechannel=", "getplanner=", "help", "progresslog=", "playfile=" , "search", "standby", "update"])
    except getopt.GetoptError:
        # print usage and exit
        usage()
        sys.exit(2)

    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit(0)
        if o in ("-c", "--changechannel"):
            channel = str(a)
            changeChannel(channel)
            sys.exit()

        if o in ("-u", "--update"):
            print "Manually updating Sky Channel Data"
            updateSkyChannels()
            sys.exit(0)

        if o in ("-g", "--getplanner"):
            print "Retrieving planner data"
            xmlFile = str(a)
            res = GetPlanner(xmlFile)
            sys.exit(res)

        if o in ("-p", "--playfile"):
            print "Starting playback of %s" % str(a)
            filename = str(a)
            res = testPlayFile(filename)
            sys.exit(res)

        if o in ("-l", "--progresslog"):
            progresslog = str(a)
            write("passed progress log file: " + a)

        if o in ("-s", "--search"):
            print "Search for SkyHD boxes"
            testMSearch()
            sys.exit(0)

        if o in ("-t", "--standby"):
            print "Send Standby to SkyHD boxes"
            standby()
            sys.exit(0)
