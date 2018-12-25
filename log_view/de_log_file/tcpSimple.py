#!/usr/bin/python
# -*- coding:utf-8 -*-
import threading, sys, os, socket, time, struct, select, os, os.path

class R():
    def __init__(self):
        pass
    
    prog_str = "download log file %3d%% [%-50s]"
    
    @staticmethod
    def exit():
        pass
        #os.system("kill -9 " + str(os.getpid())) #杀掉进程
    @staticmethod
    def output(i):
        sys.stdout.write(chr(0x0d))
        sys.stdout.write(R.prog_str % (i, (i/2)*'='))
        sys.stdout.flush()

class Server(threading.Thread):
    def __init__(self, addr):
        maxBlock = 5
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
        self.sock.bind(addr)
        self.sock.listen(maxBlock)
        self.r = 1
    def clientHandle(self, client):
        packSize = 1024
        print client
        sock, addr= client
        data = sock.recv(packSize)
        print data
        sock.send(data)
        sock.shutdown(socket.SHUT_RDWR)
        sock.close()
    def run(self):
        sock = self.sock
        while self.r:
            try:
                infds, outfds, errfds = select.select([sock,],[],[],5)
                if len(infds):
                    client , addr = self.sock.accept()
                    self.clientHandle((client , addr))
            except:
                break

class Client(threading.Thread):
    def __init__(self, addr, destaddr):
        maxBlock = 5
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
        self.sock.bind(addr)
        self.destaddr = destaddr
        self.r = 1
    def run(self):
        sock = self.sock
        packSize = 1024
        sock.connect(self.destaddr) #阻塞
        while self.r:
            try:
                sock.send("hello")
                data = sock.recv(packSize)
                print data
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
                break
            except:
                break

class LogClient(threading.Thread):
    def __init__(self, addr, destaddr, name = "0808"):
        maxBlock = 5
        threading.Thread.__init__(self)
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock = sock
        self.sock.bind(addr)
        self.destaddr = destaddr
        self.name = name
        self.r = 1
    def readn(self, n):
        sock = self.sock
        data = ""
        while n != len(data):
            data += sock.recv(n - len(data))
            print "aa", repr(data)
        return data
    def run(self):
        sock = self.sock
        packSize = 1024
        sock.connect(self.destaddr) #阻塞
        content = struct.pack("16s", "F" * 16)
        #content = struct.pack(">4I", 0x70BBCD6A, 0xB861C167, 0x4FBA9159, 0x76F80242)
        data = struct.pack("<4sIII", "LOGT", 0x48, 1, len(content))
        data += content
	print "send:",repr(data)
        sock.send(data)
        #print data
#        data = sock.recv(packSize)
#        print repr(data)
        data = self.readn(16 + 2)
#        print repr(data)
        #print data

        content = ""
        data = struct.pack("<4sIII", "LOGT", 0x46, 1, len(content))
        data += content
        sock.send(data)
#        print repr(data)
#        data = sock.recv(16)
#        print repr(data)
        data = self.readn(16)
        a, b, c, length = struct.unpack("<4sIII", data)
        while len(data) < length + 16:
            data += sock.recv(length + 16 - len(data))
#            print len(data)
            R.output(int(len(data)*100/(length + 16)))
        logDir = "logdir." + time.strftime("%Y-%m-%d", time.localtime())+""
        logDir = ""
        #if(not os.path.isdir(logDir)):
        #    os.mkdir(logDir)
        fileName = logDir + "logfile." + time.strftime("%H.%M.%S", time.localtime()) + self.name
        file = open(fileName, "w")
        file.write(data[28:])
        file.close()
#        print repr(data), len(data)
        print
        print fileName, "download ok", "file length:", len(data[28:])
#        file = open("logfile.txt", "r")
#        file.seek(32, 0)
#        for i in range(1024):
#            data = file.read(6)
#            print repr(data), struct.unpack("<IBB", data)
        R.exit()

        while self.r:
            try:
                sock.send("hello")
                data = sock.recv(packSize)
                print data
                sock.shutdown(socket.SHUT_RDWR)
                sock.close()
                break
            except:
                break

def main(argv):
    if len(argv) == 1:
        print "python tcpSimple.py mac ip"
        return
    try:
#        server = Server(("0.0.0.0", 10010))
#        server.start()
        if len(argv) == 1:
            print "python tcpSimple.py ip"
            R.exit()
        if len(argv) == 3:
            client = LogClient(("0.0.0.0", 0), (argv[2], 22306), argv[1])
            client.start()
        else:
            client = LogClient(("0.0.0.0", 0), ("10.0.22.31", 22306))
            client.start()
    except:
        R.exit()
    while 1:
        try:
            time.sleep(1000)
        except:
            R.exit()

if __name__ == "__main__":
    main(sys.argv)
