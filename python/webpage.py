import lxml.html    # python-lxml
import urlparse
import re

class WebPage:
    
    ###########################################
    #   WEBPAGE CONSTRUCTOR
    ###########################################
    def __init__(self, url, html):
        self.url = url
        self.html = html
        self.doc = lxml.html.fromstring(self.html)
        self.links = {}

    #######################################
    # parsing links from html page
    #######################################
    def parse_links(self):
        for elem, attr, link, pos in self.doc.iterlinks():
            absolute = urlparse.urljoin(self.url, link.strip())
            #print elem.tag ,attr, absolute, pos
            if elem.tag in self.links:
                self.links[elem.tag].append(absolute)
            else:
                self.links[elem.tag] = [absolute]
        return self.links

    # filter links
    def filter_links(self,tags=[],str_patterns=[]):
        #CHANGES Apr 17 2011 START
        patterns = []
        for p in str_patterns:
            patterns.append(re.compile(p))
        #CHANGES Apr 17 2011 E N D
        ##
        filterlinks = []
        if len(tags)>0:
            for tag in tags:
                for link in self.links[tag]:
                    if len(patterns) == 0:
                        pass
                        #filterlinks.append(link)
                    else:
                        for pattern in patterns:
                            if pattern.match(link)!=None:
                                filterlinks.append(link)
                                continue
        else:
            for k,v in self.links.items():
                for link in v:
                    if len(patterns) == 0:
                        pass
                        #filterlinks.append(link)
                    else:
                        for pattern in patterns:
                            if pattern.match(link)!=None:
                                filterlinks.append(link)
                                continue

        return list(set(filterlinks))

        return filterlinks    


    # form 
    def get_form(self, index):
        form = self.doc.forms[index]
        form.action = urlparse.urljoin(self.url, form.action)
        return form.action, form.fields

    #
    def get_html(self):
        return self.html

    
if __name__ == "__main__":
    import time
    from downloader import DownloadManager
    downloader = DownloadManager()

    url = "http://www.cs.colorado.edu/"
    error_msg, url, redirected_url, html = downloader.download(url)
    print error_msg, url, redirected_url, len(html)
    time.sleep(2)

    page = WebPage(url, html)
    page.parse_links()
    links = page.filter_links(tags=['a'],patterns=['^(http://www\.cs\.colorado\.edu)(/info.+)$'])

    elements = page.doc.findall('./body//div') 
    for e in elements:
        print "ELEMETNS =========================================="
        print lxml.html.tostring(e,pretty_print=True)
        print "ITEMS------------------------------------------"
        print e.items()
        print "TEXT-CONTENT-----------------------------------"
        print e.text_content()
        print "CLASSES-----------------------------------"
        classes = e.find_class("NewsHeadline")
        for c in classes:
            lxml.html.tostring(c)
    
