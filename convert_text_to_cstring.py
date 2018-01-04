#!/usr/bin/env python
# coding=utf-8

import sys

file = open(sys.argv[1], 'r')

content = file.read()
lines = content.split('\n')
for line in lines:
    print "\"%s\\n\"" %(line)

file.close()

