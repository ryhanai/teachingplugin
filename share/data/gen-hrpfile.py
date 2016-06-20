#!/usr/bin/python

import sys
import re

pdef = re.compile('DEF (.*) Group')
ppoint = re.compile('.*point.*')
plbr = re.compile('.*\{.*')
prbr = re.compile('.*\}.*')

def parseDEF(tag, f):
    print tag
    lev = 0
    while True:
        l = f.readline()
        print lev, l
        m = prbr.match(l)
        if m:
            if lev == 0:
                break
            else:
                lev -= 1
        m = plbr.match(l)
        if m:
            lev += 1

def parsePoint(f):
    while True:
        l = f.readline()
        m = prbr.match(l)
        if m:
            break
        print l

def parse(f):
    while True:
        l = f.readline()
        m = pdef.search(l)
        if m:
            parseDEF(m.group(1), f)


if __name__ == '__main__':
    if len(sys.argv) != 2:
        quit()
        
    name = sys.argv[1]
    ftemplate = open('templateHrp.wrl', 'r')
    fout = open(name + 'Hrp.wrl', 'w')
    for l in ftemplate.readlines():
        fout.write(l.replace('_PARTSNAME', name))
    ftemplate.close()
    fout.close()
