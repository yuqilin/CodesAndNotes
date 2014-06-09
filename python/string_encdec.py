#!/usr/bin/python
# -*- coding: UTF-8 -*-

# file: example4.py
import string

# 这个是 str 的字符串
s = '关关雎鸠'

# 这个是 unicode 的字符串
u = unicode('关关雎鸠', 'utf-8')

# 输出 str 字符串, 显示是乱码
print s   # 鍏冲叧闆庨笭

# 输出 unicode 字符串，显示正确
print u.encode('utf-8')  # 关关雎鸠
