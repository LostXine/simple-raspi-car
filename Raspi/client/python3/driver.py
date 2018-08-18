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

    def __init__(self):
        self.direct_launch = True
        self.__dst = ('127.0.0.1', 61551)
        self.__conf = {}
        self.__sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.__sock.settimeout(1.0)
        self.__keepRunning = False
        self.__recv_t = threading.Thread(target=self.recv_thread)
        print(dash_print('Driver No.%d Init' % id(self)))

    def __enter__(self):
        if self.__keepRunning:
            print('Driver No.%d is running.' % id(self))
        else:
            self.open()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.close()

    def __del__(self):
        self.__sock.close()
        print(dash_print('Driver No.%d Deleted' % id(self)))

    def open(self):
        print(dash_print('Driver No.%d Open' % id(self)))
        try:
            self.__keepRunning = True
            self.__recv_t.start()
        except:
            traceback.print_exc()
            print(dash_print('Driver Open Failed'))
            return None
        print(dash_print('Driver Open Done'))

    def close(self):
        self.__keepRunning = False
        self.__recv_t.join()
        print(dash_print('Driver No.%d Closed' % id(self)))

    def __launch(self):
        self.__sock.sendto(json.dumps(self.__conf).encode('utf-8'), self.__dst)
        self.__conf = {}

    def __set_code(self, code, v):
        self.__conf[code] = round(max(min(v, 1), -1) * 100)
        if self.direct_launch:
            self.__launch()

    def setMotor(self, motor):
        self.__set_code('sm', motor)

    def setServo(self, steer):
        self.__set_code('ss', steer)

    def setCamPitch(self, steer):
        self.__set_code('scp', steer)

    def setCamYaw(self, steer):
        self.__set_code('scy', steer)

    def setMode(self, md):
        if md == 'voltage':
            self.__conf['so'] = 1
        if md == 'stop':
            self.__conf['so'] = 0

    def __query_code(self, code):
        self.__conf['rq'] = code
        self.__launch()

    def getMode(self):
        self.__query_code(0)

    def getMotor(self):
        self.__query_code(1)

    def getServo(self):
        self.__query_code(2)
        
    def getCamPitch(self):
        self.__query_code(3)

    def getCamYaw(self):
        self.__query_code(4)

    def setStatus(self, **dt):
        self.__conf['uid'] = round(time.time(), 5)
        self.direct_launch = False
        if 'motor' in dt:
            self.setMotor(dt['motor'])
        if 'servo' in dt:
            self.setServo(dt['servo'])
        if 'mode' in dt:
            self.setMode(dt['mode'])
        if 'cam_pitch' in dt:
            self.setCamPitch(dt['cam_pitch'])
        if 'cam_yaw' in dt:
            self.setCamYaw(dt['cam_yaw'])
        self.__launch()
        self.direct_launch = True
    
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
        while self.__keepRunning:
            try:
                res = self.__sock.recvfrom(1024)
                print("recv: %s, from %s:%d" % (res[0], res[1][0], res[1][1]))
                if self.parse_feedback(res[0]):
                    print("Error feedback: %s" % res[0])
            except socket.timeout:
                pass
            except:
                traceback.print_exc()
                self.__keepRunning = False
        print(dash_print('Driver No.%d Recv closed' % id(self)))


if __name__ == '__main__':
    print(dash_print("Simple Raspi Car Driver"))
    print("Usage: import driver.py and using driver() to start.")
    print("-" * 10)

