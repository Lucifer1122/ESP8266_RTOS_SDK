#!/usr/bin/python
# -*- coding: utf-8 -*-
import time, struct, sys, binascii, threading, tomato, re

class Filter(threading.Thread):
    def __init__(self, fileName = "logfile.txt"):
        threading.Thread.__init__(self)
        self.fileName = fileName
    def run(self):
        file = open(self.fileName, "r")
        fileFilter = open(self.fileName + ".filter", "w")
        data = file.read(32)
        head, tm, mNum, bNum, sSize, bSize = struct.unpack("<4sIIIII", data[0:24])
        file.seek(32, 0)
        newLine = "\r\n"
        wData = "m" * 60 + newLine
        for i in range(mNum):
            data = file.read(6)
            data = struct.unpack("<IBB", data)
            if(0 == data[0]):
                continue
            wData += time.ctime(data[0]) + str(data[1:]) + newLine

        file.seek(int(32 + 6 * mNum), 0)
        wData += "b" * 60 + newLine
        cLength = 0
        for i in range(bNum):
            data = file.read(bSize)
            pdata = struct.unpack("<2sII", data[0:10])
            rHead, content = tomato.R.unPackHead(data[10:])
            if(0 == pdata[2]):
                continue
            if rHead[-2] < 0:
                cLength = 0
            elif rHead[-2] > 128 - 35:
                cLength = 128 - 35
            else:
                cLength = rHead[-2]
            wData += str(pdata[0]) + str(pdata[1]) + time.ctime(pdata[2]) + str(rHead) + binascii.b2a_hex(content[0:cLength]) + newLine

        file.seek(int(32 + 6 * mNum + bNum * bSize), 0)
        data = file.read(sSize)
        file.close()
        wData += "s" * 60 + newLine
        r = re.compile("\$@")
        arr = r.split(data)
        for item in arr[1:len(arr)-1]:
            wData += time.ctime(int(item[8:16], 16)) + " " + str(int(item[8:16], 16)) + "--->" + item[16:] + newLine
        fileFilter.write(wData)
        fileFilter.close()

def main(argv):
    if len(argv) == 2:
        filter = Filter(argv[1])
        filter.run()

if __name__ == "__main__":
    main(sys.argv)
