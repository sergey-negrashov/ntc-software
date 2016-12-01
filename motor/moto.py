# -*- coding: utf-8 -*-

"""
Notes:

Motorized Xslide:
1.8º/step. 1 mm/360º. 5 microns/step

ex.
a.loop(20,21,5):
pause for 20s, move +21 steps, and repeat the last two until
the instructions were looped 5 times

a.zero():
returns the stage to absolute zero

a.absmove(21):
moves stage an +21 steps away from absolute zero regardless
of prior location.

Encoding as bytes is necessary. 
self.ser.write(bytes(cmd, 'utf-8'))

"""

import serial
import time
class moto:
    def __init__(self):
        self.ser = serial.Serial('/dev/ttyUSB0')
    def moveUp(self, numSteps):
        cmd = 'C E I1M' + str(numSteps) + ',R'
        self.ser.write(cmd)
        while True:
            time.sleep(0.1)
            if self.ser.read() == "^":
                break
    def zero(self):
        self.ser.write('C E I1M-0, R')
        while True:
            time.sleep(0.1)
            if self.ser.read() == "^":
                break

    def absmove(self, pos):
        cmd = 'C E IA1M' + str(pos) + ',R'
        self.ser.write(cmd)
        while True:
            time.sleep(0.1)
            if self.ser.read() == "^":
                break

    def paus(self, time):
        cmd = 'C E P' + str(time) + ',R'
        self.ser.write(cmd)
        while True:
            time.sleep(0.1)
            if self.ser.read() == "^":
                break

    def loop(self, sec, steps, loops):
        cmd = 'C E P' + str(sec*10) + ',I1M' + str(steps) + ',L' + str(loops) + ',R'
        self.ser.write(cmd)
        while True:
            time.sleep(0.1)
            if self.ser.read() == "^":
                break
        self.ser.write(bytes(cmd, 'utf-8'))


a = moto()
a.moveUp(10000)

