# -*- coding:utf-8 -*-

"""
@file: raspi_car_demo.py
@time: 2018/8/12 0:19
Depends: OpenCV(pip install opencv-python)
"""

import numpy as np
import cv2
import time
from collections import deque
from driver import driver


def put_text_center(img, text):
    size = cv2.getTextSize(text, cv2.FONT_HERSHEY_COMPLEX, 1, thickness=2)
    cv2.putText(img, text, (int(img.shape[1] / 2 - size[0][0] / 2), int(img.shape[0] / 5)), cv2.FONT_HERSHEY_COMPLEX, 1,
                (128, 128, 128), thickness=2)
    return img


def put_text_left_top(img, text):
    cv2.rectangle(img, (0, 0), (47, 10), (0, 100, 0), -1)
    cv2.putText(img, 'FPS:%0.1f' % text, (3, 8), cv2.FONT_HERSHEY_SIMPLEX, 0.3, (255, 255, 255))
    return img


def get_no_cam_img():
    img = np.zeros((480, 640, 3), np.uint8)
    img[...] = 225
    img = put_text_center(img, 'No Camera')
    return img


class DriveConsole:
    def __init__(self, d):
        self._motor = 0
        self._servo = 0
        self._cam_p = 0
        self._cam_y = 0
        self._driver = d

    def _draw_status(self, img, text, text_idx, axis):
        text_size = 0.6
        text_font = cv2.FONT_HERSHEY_DUPLEX
        text_box = (260, 10)  # cv2.getTextSize(text, text_font, text_size, thickness=3)[0]
        text_broader = 8
        # draw broader
        cv2.rectangle(img, (text_idx[0] - text_broader, text_idx[1] - text_box[1] - text_broader),
                      (text_box[0] + text_idx[0] + text_broader, text_idx[1] + text_box[0] + text_broader),
                      (120, 120, 120), 2, -1)
        graph_idx = (text_idx[0] + text_broader, text_idx[1] + text_broader)
        graph_size = [text_box[0] - 2 * text_broader] * 2
        # draw graph
        # x-axis
        cv2.rectangle(img, (graph_idx[0], graph_idx[1] + graph_size[0]//2),
                      (graph_idx[0] + graph_size[0], graph_idx[1] + graph_size[0]//2 + 1), (0, 0, 0))
        # y-axis
        cv2.rectangle(img, (graph_idx[0] + graph_size[1] // 2, graph_idx[1]),
                      (graph_idx[0] + graph_size[1] // 2 + 1, graph_size[1] + graph_idx[1]), (0, 0, 0))
        # draw point
        circle_center = (graph_idx[0] + graph_size[0] // 2 + round(axis[1] * graph_size[0] // 2),
                         graph_idx[1] + graph_size[1] // 2 - round(axis[0] * graph_size[1] // 2))
        cv2.circle(img, circle_center, 5, (0, 0, 0), 4)
        cv2.circle(img, circle_center, 2, (100, 230, 55), 4)
        # draw text
        cv2.putText(img, text, text_idx, text_font, text_size, (50, 120, 25), thickness=2)
        cv2.putText(img, text, text_idx, text_font, text_size, (100, 230, 55), thickness=1)
        return img

    def draw_control_ui(self, img):
        return self._draw_status(img, 'Motor: %0.2f Servo: %0.2f' % (self._motor, self._servo),
                                 (20, img.shape[0] - 300), (self._motor, self._servo))

    def draw_camera_ui(self, img):
        return self._draw_status(img, 'Pitch: %0.2f Yaw: %0.2f' % (self._cam_p, self._cam_y),
                                 (img.shape[1] - 280, img.shape[0] - 300), (self._cam_p, self._cam_y))

    def forward(self):
        self._motor += 0.1
        self._motor = max(min(self._motor, 1), -1)
        self._driver.setMotor(self._motor)

    def backward(self):
        self._motor -= 0.1
        self._motor = max(min(self._motor, 1), -1)
        self._driver.setMotor(self._motor)

    def left(self):
        if self._servo > 0:
            self._servo = 0
        else:
            self._servo -= 0.1
        self._servo = max(min(self._servo, 1), -1)
        self._driver.setServo(self._servo)

    def right(self):
        if self._servo < 0:
            self._servo = 0
        else:
            self._servo += 0.1
        self._servo = max(min(self._servo, 1), -1)
        self._driver.setServo(self._servo)

    def cam_up(self):
        self._cam_p += 0.1
        self._cam_p = max(min(self._cam_p, 1), -1)
        self._driver.setCamPitch(self._cam_p)

    def cam_down(self):
        self._cam_p -= 0.1
        self._cam_p = max(min(self._cam_p, 1), -1)
        self._driver.setCamPitch(self._cam_p)

    def cam_left(self):
        self._cam_y += 0.1
        self._cam_y = max(min(self._cam_y, 1), -1)
        self._driver.setCamYaw(self._cam_y)

    def cam_right(self):
        self._cam_y -= 0.1
        self._cam_y = max(min(self._cam_y, 1), -1)
        self._driver.setCamYaw(self._cam_y)


def raspi_car_demo():
    with driver() as d:
        if d is None:
            return 1
        cam = cv2.VideoCapture(-1)
        no_cam = get_no_cam_img()
        dc = DriveConsole(d)
        key_dict = {key: func for key, func in zip('WSADIKLJwsadiklj',
                                                   [dc.forward, dc.backward, dc.left, dc.right,
                                                    dc.cam_up, dc.cam_down, dc.cam_left, dc.cam_right] * 2)}
        # calculate fps
        fps_list = deque(maxlen=20)
        fps_list.append(0.1)
        try:
            while True:
                fps_start = time.clock()
                if cam.isOpened():
                    _, img = cam.read()
                else:
                    img = no_cam.copy()
                fps = len(fps_list) / sum(fps_list)
                img = put_text_left_top(img, fps)
                img = dc.draw_control_ui(img)
                img = dc.draw_camera_ui(img)
                cv2.imshow('raspberry pi car', img)
                key = cv2.waitKey(100) & 0xff
                if key in (13, 27, ord('q'), ord('Q')):
                    break
                if chr(key) in key_dict:
                    key_dict[chr(key)]()
                fps_list.append(time.clock() - fps_start)
        except KeyboardInterrupt:
            pass
        cv2.destroyAllWindows()


if __name__ == '__main__':
    raspi_car_demo()
