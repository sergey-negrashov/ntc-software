import zmq
from scipy.signal import fftconvolve, lfilter, firwin
import ntc_pb2
from PyQt4 import QtCore, QtGui
import numpy as np
import math


class ZmqReceiver(QtCore.QObject):

    message = QtCore.pyqtSignal(str)

    v1_update = QtCore.pyqtSignal(list)
    v2_update = QtCore.pyqtSignal(list)

    h1_update = QtCore.pyqtSignal(list)
    h2_update = QtCore.pyqtSignal(list)

    ar_update = QtCore.pyqtSignal(float, float)

    def __init__(self):
        QtCore.QObject.__init__(self)
        self.port = 5530
        self.context = zmq.Context()
        self.socket = self.context.socket(zmq.SUB)
        self.socket.bind("tcp://*:%s" % self.port)
        self.socket.setsockopt(zmq.SUBSCRIBE, "0")
        self.actual_times = actual_times = np.linspace(0.0, 204.8, num=974)
        self.fir_coef = [0.005836310228843312,
            0.018235835282369148,
            0.026122022209528667,
            0.04327394282699852,
            0.05959485293788556,
            0.07821170914051696,
            0.0949159385180802,
            0.10895605062142762,
            0.1179999319917522,
            0.12122948160286849,
            0.1179999319917522,
            0.10895605062142762,
            0.0949159385180802,
            0.07821170914051696,
            0.05959485293788556,
            0.04327394282699852,
            0.026122022209528667,
            0.018235835282369148,
            0.005836310228843312
            ]


    def receive_waveform(self):
        topic = self.socket.recv()
        proto = self.socket.recv()
        mp = ntc_pb2.MotorPosition()
        mp.ParseFromString(proto)

        aveVolt1 = np.zeros(974).tolist()
        v1count = 0
        aveVolt2 = np.zeros(974).tolist()
        v2count = 0

        cross_time1 = []
        cross_time2 = []

        for event in mp.events:
            for chan in event.channel:
                if chan.channel == 0:
                    v1count +=1
                elif chan.channel == 1:
                    v2count +=1
                else:
                    continue
                voltage = chan.voltage[:-50]
                voltage = self.get_rid_of_random_god_damn_spikes(voltage)
                time = chan.time[:-50]
                voltage = np.interp(self.actual_times, time, voltage)
                voltage = lfilter(self.fir_coef, 1.0, voltage).tolist()
                cross_time, max_val= self.pulse_50_percent_time(voltage,time)

                if chan.channel == 0:
                    for i in range(0,974):
                        aveVolt1[i] += voltage[i]
                    if cross_time != -1:
                        cross_time1.append(cross_time)

                elif chan.channel == 1:
                    for i in range(0,974):
                        aveVolt2[i] += voltage[i]
                    if cross_time != -1:
                        cross_time2.append(cross_time)

        for i in range(0,974):
            aveVolt2[i] /= v2count
            aveVolt1[i] /= v1count
        self.v2_update.emit(aveVolt2)
        self.v1_update.emit(aveVolt1)

        self.h1_update.emit(cross_time1)
        self.h2_update.emit(cross_time2)

        self.ar_update.emit(np.mean(cross_time1) - np.mean(cross_time2), mp.motorPos/4000.0)


    def loop(self):
        while(True):
            self.receive_waveform()

    def get_rid_of_random_god_damn_spikes(self,voltages_in):
        voltages = voltages_in[:]
        for i in range(1, len(voltages) - 1):
            if math.fabs(voltages_in[i] - voltages_in[i - 1]) > 2 and math.fabs(voltages_in[i + 1] - voltages_in[i]) > 2:
                voltages[i] = voltages[i - 1]
        return voltages


    def pulse_50_percent_time(self, voltages, times):
        max_index = voltages.index(max(voltages))
        max_value = max(voltages)
        mean = np.mean(voltages[0: max_index - 20 if max_index - 20 > 0 else 1] +  voltages[max_index + 20 if max_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages) - 1])
        voltages = [v - mean for v in voltages]
        max_index = voltages.index(max(voltages))
        max_value = max(voltages)
        target = (max_value)/2.0
        for index in range(max_index - 40, max_index)[::-1]:
            if voltages[index] >= target and voltages[index - 1] <= target:
                #print [times[index - 1], times[index], np.interp(target, [voltages[index - 1], voltages[index]], [times[index - 1], times[index]]), target]
                return np.interp(target, [voltages[index - 1], voltages[index]], [times[index - 1], times[index]]), max_value
        return -1, -1