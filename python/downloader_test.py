# coding=utf-8
import urllib2
import urllib
import cookielib
import socket
import lxml.html    # python-lxml # http://codespeak.net/lxml/lxmlhtml.html
import time
import re
import urlparse

from webpage import WebPage

SOCKET_DEFAULT_TIMEOUT = 30

class DownloadManager:

    def __init__(self, cookie = None, timeout = None):
        # cookie
        if cookie == None:
            self.cookie = cookielib.LWPCookieJar() 
        else:
            self.cookie = cookie
        opener = urllib2.build_opener(urllib2.HTTPRedirectHandler, urllib2.HTTPCookieProcessor(self.cookie))
        urllib2.install_opener(opener)
        # socket timeout
        if timeout == None:
            timeout = SOCKET_DEFAULT_TIMEOUT
            socket.setdefaulttimeout(timeout)
                
    def clean_cookie(self):
        self.cookie = cookielib.LWPCookieJar() 
        
    def download(self, url, data = None):  #download the html page from server. 

        redirected_url = None
        error_msg = None
        html = None
        # 1. URL Request Head
        user_agent = 'Mozilla/4.0 (compatible; MSIE 5.5; Windows NT)'
        headers = { 'User-Agent' : user_agent }
        if data != None:
            req = urllib2.Request(url, urllib.urlencode(data), headers)
        else:
            req = urllib2.Request(url, data, headers)
        # 2. URL Request
        try:
            response = urllib2.urlopen(req)
        except urllib2.URLError, e:
            if hasattr(e, 'reason'):
                #raise HtmlPageError('[fetch] failed to reach a server.' + \
                #            ' Reason: '+ str(e.reason) )
                error_msg = 'network-error'
            elif hasattr(e, 'code'):
                #raise HtmlPageError('[fetch] server couldn\'t fulfill the request.'+\
                #            ' Error code: '+ str(e.code) )
                error_msg = 'server-error'
            else:
                #raise HtmlPageError('[fetch] URLError, unknown reason.')
                error_msg = 'other-error'
        except KeyboardInterrupt:
            raise
        except:
            #raise HtmlPageError('[fetch] Unexpected urlopen() error: ' + 
            #            str(sys.exc_info()[0]) )
            error_msg = 'urlopen-error'
        
        # 3. Read Html 
        if error_msg != None:
            return error_msg, url, redirected_url, html
        try:
            html = response.read()
        except KeyboardInterrupt:
            raise
        except:
            #raise HtmlPageError('[fetch] Unexpected response.read() error: ' + \
            #            str(sys.exc_info()[0]) )
            error_msg = 'reading-error'

        # 4. check if there is a redirect
        if error_msg != None:
            f = open("undownloaded_urls.log", 'a')
            f.write(time.strftime("%Y/%m/%d %H:%M:%S") + "\t" + error_msg + "\t" + url + "\n")
            f.close()
            return error_msg, url, redirected_url, html

        nurl = response.geturl()
        if url != nurl:
            redirected_url = nurl
        else:
            redirected_url = None            
        
        # 5. generate lxml.html object for future processing
        #self.html = lxml.html.fromstring(the_page)
        return error_msg, url, redirected_url, html
    """
    def lxml_download(self, url):
        redirected_url = None
        error_msg = None
        html = None
        try:
            doc = lxml.html.parse(url).getroot()
            html = lxml.html.tostring(doc)
        except:
            error_msg = "lxml error"
        return error_msg, url, redirected_url, html
    """
        
if __name__ == "__main__":
    #'''
    url = "http://www.cnbeta.com/"
    downloader = DownloadManager()
    error_msg, url, redirected_url, html = downloader.download(url)
    print "error_msg=%s" %error_msg
    print "url=%s" %url
    print "redirected_url=%s" %redirected_url
    f = open("www.cnbeta.com.html",'w')
    f.write(html)
    f.close()
    webpage = WebPage(url, html)
    webpage.parse_links()

    website = 'cnbeta\.com'
    patnstr = '^(http|https)://(.*\.' + website + ')(.+)$';
    links = webpage.filter_links(tags=['a'], str_patterns=[patnstr])
    links.sort()

    f_filter_links = open('filter_links_cnbeta.txt', 'w')

    #print links
    f = open('links_regged_cnbeta.txt','w')
    for link in links:
        f_filter_links.write('%s\n' % link)
        f.write('%s\n' % link)
        for elem, attr, lnk, pos in webpage.doc.iterlinks():
            absolute = urlparse.urljoin(webpage.url, lnk.strip())
            if absolute == link and elem.text:
                f.write('%s\n' % elem.text.encode('utf-8'))
    f.close()
    f_filter_links.close()
    #'''

    url = 'http://www.geekpark.net/'
    downloader = DownloadManager()
    error_msg, url, redirected_url, html = downloader.download(url)
    print "error_msg=%s" %error_msg
    print "url=%s" %url
    print "redirected_url=%s" %redirected_url
    f = open("www.geekpark.net.html",'w')
    f.write(html)
    f.close()
    webpage = WebPage(url, html.decode('utf-8'))
    webpage.parse_links()

    website = 'geekpark\.net'
    patnstr = '^(http|https)://(.*\.' + website + ')(.+)$';
    links = webpage.filter_links(tags=['a'], str_patterns=[patnstr])
    links.sort()

    f_filter_links = open('filter_links_geekpark.txt', 'w')

    #print links
    f = open('links_regged_geekpark.txt','w')
    for link in links:
        f_filter_links.write('%s\n' % link)
        f.write('%s\n' % link)
        for elem, attr, lnk, pos in webpage.doc.iterlinks():
            absolute = urlparse.urljoin(webpage.url, lnk.strip())
            if absolute == link and elem.text:
                f.write('%s\n' % elem.text.encode('utf-8'))
    f.close()
    f_filter_links.close()

'''
    #links_with_keywords = []
    for tag in links:
        f.write('================\n')
        f.write('%s\n' % tag)
        if tag == 'a':
            for link in links[tag]:
                #print link.encode('utf-8')
                link = link.encode('utf-8')
                if rePatnObj.match(link):
                    if links_matched_rules.count(link) == 0:
                        links_matched_rules.append(link)
                        f.write('%s\n' % link)
        f.write('================\n')
    f.close()
'''