#!/usr/bin/python
# -*- coding: utf-8 -*-
import time, struct, sys, binascii, threading, tomato, re
#import xlwt

class Filter(threading.Thread):
    def __init__(self, fileName = "logfile.txt"):
        threading.Thread.__init__(self)
        self.fileName = fileName
        #self.wbk = xlwt.Workbook()
        #self.sheet1 = self.wbk.add_sheet(u"日志1")
        #self.sheet2 = self.wbk.add_sheet(u"日志2")
        #self.sheet3 = self.wbk.add_sheet(u"日志3")
    def run(self):
        file = open(self.fileName, "r")
        fileFilter = open(self.fileName + ".filter", "w")
        
        data = file.read(16)
        if data[0:4] == "star":
            data += file.read(32 - 16)
        else:
            data = file.read(12)
            if data[0:4] == "star":
                data += file.read(32 - 12)
            else:
                data = file.read(32)
        #data = file.read(32)
        head, tm, mNum, bNum, sSize, bSize = struct.unpack("<4sIIIII", data[0:24])
        file.seek(32, 0)
        newLine = "\r\n"
        wData = "m" * 60 + newLine
        
        #self.sheet1.write(0, 0, u"时间")
        #self.sheet1.write(0, 1, u"数据")
        for i in range(mNum):
            data = file.read(6)
            data = struct.unpack("<IBB", data)
            if(0 == data[0]):
                continue
            wData += time.ctime(data[0]) + str(data[1:]) + newLine
         #   self.sheet1.write(i + 1, 0, time.ctime(data[0]))
         #   self.sheet1.write(i + 1, 1, str(data[1:]))

        file.seek(int(32 + 6 * mNum), 0)
        wData += "b" * 60 + newLine
        cLength = 0
        
        #self.sheet2.write(0, 0, u"时间")
        #self.sheet2.write(0, 1, u"头部")
        #self.sheet2.write(0, 2, u"命令")
        #self.sheet2.write(0, 3, u"数据")
        
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
            ##self.sheet2.write(i + 1, 0, str(pdata[0]))
            #self.sheet2.write(i + 1, 2, str(pdata[1]))
            #self.sheet2.write(i + 1, 0, time.ctime(pdata[2]))
            #self.sheet2.write(i + 1, 1, str(rHead))
            #self.sheet2.write(i + 1, 3, binascii.b2a_hex(content[0:cLength]))
        
        file.seek(int(32 + 6 * mNum + bNum * bSize), 0)
        data = file.read(sSize)
        file.close()
        wData += "s" * 60 + newLine
        r = re.compile("\$@")
        arr = r.split(data)
        i = 1
        
        re0 = re.compile("::(.*)::(.*)")
        
        #self.sheet3.write(0, 0, u"时间")
        #self.sheet3.write(0, 1, u"协议")
        #self.sheet3.write(0, 2, u"数据")
        for item in arr[1:len(arr)-1]:
            wData += time.ctime(int(item[8:16], 16)) + " " + str(int(item[8:16], 16)) + "--->" + item[16:] + newLine
            #self.sheet3.write(i, 0, time.ctime(int(item[8:16], 16)))
            ##self.sheet3.write(i, 1, str(int(item[8:16], 16)))
            a0 = item[16:len(item[16:])]
            m = re0.search(a0)
            #if m:
                #print m.group(1)
                #self.sheet3.write(i, 1, m.group(1))
                #self.sheet3.write(i, 2, m.group(2))
                #else:
                #self.sheet3.write(i, 1, "error")
                #self.sheet3.write(i, 2, a0)
            ##self.sheet3.write(i, 3, str(rHead))
            ##self.sheet3.write(i, 4, binascii.b2a_hex(content[0:cLength]))
            i += 1
        fileFilter.write(wData)
        fileFilter.close()
        #self.wbk.save(self.fileName + ".xls")
        print self.fileName + ".filter"

def main(argv):
    if len(argv) == 2:
        filter = Filter(argv[1])
        filter.run()
    else:
        print "python logFilter.py file_name"

if __name__ == "__main__":
    main(sys.argv)
