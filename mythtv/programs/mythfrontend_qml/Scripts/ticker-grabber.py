#!/usr/bin/env python
# -*- coding: utf-8 -*-
#
# Python script to download and process Rss Feeds and create a ticker.xml file for use by the scroller in mythfrontend_qml
#

import os, sys
from glob import glob
import feedparser
from lxml import etree

if __name__ == '__main__':

    if len(sys.argv) < 2:
        print "Missing output filename!"
        sys.exit(1)

    outFile = sys.argv[1]

    # A list to hold all headlines
    tickeritems = etree.XML(u'<items></items>')

    # List of RSS feeds that we will fetch and combine
    feedurls = [
        ('BBC Weather Observations',   'http://open.live.bbc.co.uk/weather/feeds/en/2644547/observations.rss'),
        ('BBC Weather 3 Day Forecast', 'http://open.live.bbc.co.uk/weather/feeds/en/2644547/3dayforecast.rss'),
        ('BBC News - UK',              'http://feeds.bbci.co.uk/news/uk/rss.xml')
    ]

    # Iterate over the feed urls
    idNo = 1
    for title,url in feedurls:
        print "Processing feed: " + title
        item = etree.SubElement(tickeritems, "item")
        feed = feedparser.parse(url)
        etree.SubElement(item, "id").text = str(idNo)
        etree.SubElement(item, "category").text = unicode(title, 'utf-8')
        tickerText = ''
        itemCount = 0
        for newsitem in feed['items']:
            itemCount += 1

            if itemCount > 10:
                break

            if len(tickerText) > 0:
                tickerText = tickerText + "    ~    " + newsitem['title'] + " - " + newsitem['description']
            else:
                tickerText = newsitem['title'] + " - " + newsitem['description']

        etree.SubElement(item, "text").text = tickerText

        idNo += 1

    print "Saving xml file to %s" % outFile

    output = etree.tostring(tickeritems, encoding='UTF-8', pretty_print=True, xml_declaration=True)

    xmlFile = open(outFile, "w")
    xmlFile.write(output)
    xmlFile.close()

    sys.exit(0)
