#!/usr/bin/python
# -*- coding: UTF-8 -*-

from lxml import etree
html = ''
f = open('www.cnbeta.com.html', 'r')
html = f.read()

#'''
page = etree.HTML(html.lower().decode('utf-8'))
hrefs = page.xpath(u'//a')
result_file = open('lxml_test_reuslt.txt', 'w')

for href in hrefs:
    result_file.write('%s\n' % href.attrib)
    if href.text:
        result_file.write('%s\n' % href.text.encode('utf-8'))#.encode('utf-8')

result_file.close()
#'''

f.close()