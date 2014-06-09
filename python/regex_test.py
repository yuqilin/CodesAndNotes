#!/usr/bin/python
# -*- coding: UTF-8 -*-

import re
str = '  http://job.cnbeta.com/articles/267695.htm  '
website = 'cnbeta\.com'
rePatnStr = '^(http|https)://(.*\.' + website + ')(.+)$'
print rePatnStr
rePatnObj = re.compile(rePatnStr)
str = str.strip()
print str
match = rePatnObj.match(str)
if match:
    print match.groups()
else:
    print 'did not match'

print 'mailto:ugmbbc@gmail.com?subject=%E6%8A%95%E9%80%92%E8%B5%84%E8%AE%AF%EF%BC%9A'