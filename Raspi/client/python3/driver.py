#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
@file: driver.py
@time: 2018/8/12 0:19
"""

import threading
import socket
import json
import time
import traceback


def dash_print(txt, num=7):
    return '-' * num + txt + '-' * num


class driver:

    def __enter__(self):
        print(dash_print('Driver No.%d Init' % id(self)))
        try: 
            self.__dst = ('127.0.0.1', 61551)
            self.__conf = {}
            self.__sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.__sock.settimeout(1.0)
            self.__keepRunning = True
            self.__recv_t = threading.Thread(target=self.recv_thread)
            self.__recv_t.start()
        except:
            traceback.print_exc() 
            print(dash_print('Driver Init Failed'))
            return None
        print(dash_print('Driver Init Done'))
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()
        print(dash_print('Driver No.%d End' % id(self)))

    def __launch(self):
        self.__sock.sendto(json.dumps(self.__conf).encode('utf-8'), self.__dst)
        self.__conf = {}
    
    def __setMotor(self, motor):
        # motor: from -1 to 1
        self.__conf['sm'] = round(max(min(motor, 1), -1) * 100)

    def __setServo(self, steer):
        # steer: from -1 to 1
        self.__conf['ss'] = round(max(min(steer, 1), -1) * 100)

    def __setDist(self, dt):
        # distance: from 0 to 0xffffffff
        self.__conf['sd'] = dt

    def __setMode(self, md):
        if md == 'voltage':
            self.__conf['so'] = 1
        if md == 'distance':
            self.__conf['so'] = 3
        if md == 'stop':
            self.__conf['so'] = 0

    def getMode(self):
        self.__conf['rq'] = 0
        self.__launch()

    def getMotor(self):
        self.__conf['rq'] = 1
        self.__launch()

    def getServo(self):
        self.__conf['rq'] = 2
        self.__launch()

    def setStatus(self, **dt):
        self.__conf['uid'] = round(time.time(), 5)
        if 'motor' in dt:
            self.__setMotor(dt['motor'])
        if 'servo' in dt:
            self.__setServo(dt['servo'])
        if 'dist' in dt:
            self.__setDist(dt['dist'])
        if 'mode' in dt:
            self.__setMode(dt['mode'])
        self.__launch()

    def close(self):
        self.__keepRunning = False
        self.__recv_t.join()
        self.__sock.close()
    
    def heartBeat(self):
        self.setStatus()

    def parse_feedback(self, js):
        try:
            obj = json.loads(js.decode('utf-8'))
            """
            if not 'uid' in obj:
                print("Uid is needed")
                return 2
            elif self.__uid > obj['uid']:
                print("Uid %0.3f is behind %0.3f, drop %s." % (obj['uid'], self.__uid, js))
            else:
                self.__uid = float(obj['uid']
            """
            return 0
        except:
            traceback.print_exc()
            return 1

    def recv_thread(self):
        while (self.__keepRunning):
            try:
                res = self.__sock.recvfrom(1024)
                print("recv: %s, from %s:%d" % (res[0], res[1][0], res[1][1]))
                if self.parse_feedback(res[0]):
                    print("Error feedback: %s" % res[0])
            except socket.timeout as e:
                pass


if __name__ == '__main__':
    print(dash_print("Simple Raspi Car Driver"))
    print("Usage: import driver.py and using driver() to start.")
    print("-" * 10)

