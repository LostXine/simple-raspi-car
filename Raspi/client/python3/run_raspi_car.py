#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
@file: run_raspi_car.py
@time: 2018/8/12 0:19
"""

from driver import driver
import time


def run_raspi_car():
    print("==========Client Start==========")
    # if True:
    #    d = driver()
    #    d.open()
    with driver() as d:
        if d is not None:
            d.setStatus(motor=0.0, servo=0.0, dist=0x00, cam_pitch=0, cam_yaw=0, mode="stop")
            t = 10
            b = True
            try:
                print("Test 1/2: voltage mode")
                d.setStatus(mode="voltage")
                while True:
                    st = t * 0.1 - 1
                    sm = st
                    if abs(st) < 0.4:
                        sm = 0.4 if st > 0 else -0.4
                        if st == 0:
                            sm = 0
                    d.setStatus(motor=sm, servo=st)
                    print("Motor: %0.2f, Servo: %0.2f" % (sm, st))
                    if b:
                        t += 1
                    else:
                        t -= 1
                    if t > 20:
                        b = False
                        t -= 2
                    if t < 0:
                        b = True
                        t += 2
                    time.sleep(1)
                    # d.heartBeat()
                    d.getMotor()
                    d.getServo()
                    d.getMode()
                    time.sleep(1)
            except KeyboardInterrupt:
                d.setStatus(motor=0.0, servo=0.0, dist=0x00, mode="stop")
            try:
                print("Test 2/2: camera move")
                d.setStatus(motor=0.0, servo=0, mode="voltage")
                while True:
                    st = t * 0.1 - 1
                    sm = st
                    d.setStatus(cam_pitch=sm, cam_yaw=st)
                    print("Pitch: %0.2f, Yaw: %0.2f" % (sm, st))
                    if b:
                        t += 1
                    else:
                        t -= 1
                    if t > 20:
                        b = False
                        t -= 2
                    if t < 0:
                        b = True
                        t += 2
                    time.sleep(1)
                    # d.heartBeat()
                    d.getCamPitch()
                    d.getCamYaw()
                    time.sleep(1)
            except KeyboardInterrupt:
                d.setStatus(motor=0.0, servo=0.0, dist=0x00, cam_pitch=0, cam_yaw=0, mode="stop")
            d.close()
    print("==========Client Fin==========")
    return 0
               
    
if __name__ == '__main__':
    run_raspi_car()
