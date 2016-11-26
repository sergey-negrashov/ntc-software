#!/usr/bin/env python

# embedding_in_qt4.py --- Simple Qt4 application embedding matplotlib canvases
#
# Copyright (C) 2005 Florent Rougon
#               2006 Darren Dale
#
# This file is an example program for matplotlib. It may be used and
# modified with no restriction; raw copies as well as modified versions
# may be distributed without limitation.

from __future__ import unicode_literals
import sys
import os
import random
from matplotlib.backends import qt_compat
from PyQt4 import QtGui, QtCore
import numpy as np
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

from zmq_receiver import  ZmqReceiver

progname = os.path.basename(sys.argv[0])
progversion = "0.1"


class MplCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)
        # We want the axes cleared every time plot() is called
        self.axes.hold(False)

        self.compute_initial_figure()

        #
        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QtGui.QSizePolicy.Expanding,
                                   QtGui.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

    def compute_initial_figure(self):
        pass


class VoltageCanvas(MplCanvas):


    def __init__(self, *args, **kwargs):
        MplCanvas.__init__(self, *args, **kwargs)
        timer = QtCore.QTimer(self)
        self.title = "Voltage"
        self.v = [0]*974
        self.t = map(lambda x: x*0.2, range(0,974))

    def set_title(self, title):
        self.title = title

    def compute_initial_figure(self):
        self.axes.plot([0, 0, 0, 0], [0, 0, 0, 0], 'r')

    def update_figure(self, newSamples):
        self.v = newSamples
        self.t = map(lambda x: x*0.2, range(0,len(self.v)))
        self.axes.plot(self.t, self.v, 'r')
        self.axes.set_title(self.title)
        self.axes.set_xlabel("Time (ns)")
        self.axes.set_ylabel("Voltage (mV)")
        self.draw()

class HistCanvas(MplCanvas):
    def __init__(self, *args, **kwargs):
        MplCanvas.__init__(self, *args, **kwargs)
        self.title = "Histogram"
        self.xlabel = "Voltage"
        self.dat = [0]*100

    def set_title(self, title):
        self.title = title

    def set_xlabel(self, label):
        self.xlabel = label

    def compute_initial_figure(self):
        self.axes.hist(range(0,100))

    def update_figure(self, values):
        self.dat  = values
        self.axes.hist(self.dat)
        self.axes.set_title(self.title)
        self.axes.set_xlabel(self.xlabel)
        self.axes.set_ylabel("Hits")
        self.draw()

class DiffCanvas(MplCanvas):
    def __init__(self, *args, **kwargs):
        MplCanvas.__init__(self, *args, **kwargs)
        self.title = "Voltage"
        self.timeDifference = []
        self.position = []

    def set_title(self, title):
        self.title = title

    def compute_initial_figure(self):
        self.axes.plot([0, 0, 0, 0], [0, 0, 0, 0], 'r*')

    def update_figure(self, new_diff, pos):
        self.position.append(pos)
        self.timeDifference.append(new_diff)
        self.axes.plot(self.position, self.timeDifference, 'r*')
        self.axes.set_title(self.title)
        self.axes.set_xlabel("Motor Position (cm)")
        self.axes.set_ylabel("Time difference between arrivals(ns)")
        self.draw()


class ApplicationWindow(QtGui.QMainWindow):
    def __init__(self):
        QtGui.QMainWindow.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowTitle("application main window")
        self.main_widget = QtGui.QWidget(self)
        mainLayout = QtGui.QVBoxLayout(self.main_widget)

        voltLayout = QtGui.QHBoxLayout(self.main_widget)
        mainLayout.addLayout(voltLayout)
        self.v1 = VoltageCanvas(self.main_widget, width=5, height=4, dpi=100)
        self.v2 = VoltageCanvas(self.main_widget, width=5, height=4, dpi=100)
        voltLayout.addWidget(self.v1)
        voltLayout.addWidget(self.v2)
        self.v1.set_title("SiPMT #1")
        self.v2.set_title("SiPMT #2")

        histLayout = QtGui.QHBoxLayout(self.main_widget)
        mainLayout.addLayout(histLayout)
        self.h1 = HistCanvas(self.main_widget, width=5, height=4, dpi=100)
        self.h2 = HistCanvas(self.main_widget, width=5, height=4, dpi=100)
        histLayout.addWidget(self.h1)
        histLayout.addWidget(self.h2)
        self.h1.set_title("SiPMT #1 Arrival Times")
        self.h2.set_title("SiPMT #2 Arrival Times")
        self.h1.set_xlabel("Time(ns)")
        self.h2.set_xlabel("Time(ns)")

        self.diff = DiffCanvas(self.main_widget, width=5, height=4, dpi=100)
        mainLayout.addWidget(self.diff)
        self.diff.set_title("Arrival time difference 100 events")
        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        self.thread = QtCore.QThread()
        self.zeromq_listener = ZmqReceiver()
        self.zeromq_listener.moveToThread(self.thread)
        self.thread.started.connect(self.zeromq_listener.loop)

        self.zeromq_listener.v1_update.connect(self.v1.update_figure)
        self.zeromq_listener.v2_update.connect(self.v2.update_figure)

        self.zeromq_listener.h1_update.connect(self.h1.update_figure)
        self.zeromq_listener.h2_update.connect(self.h2.update_figure)

        self.zeromq_listener.ar_update.connect(self.diff.update_figure)
        QtCore.QTimer.singleShot(0, self.thread.start)

        self.setWindowTitle("nTC Calibration Monitor")

qApp = QtGui.QApplication(sys.argv)
aw = ApplicationWindow()
aw.show()
sys.exit(qApp.exec_())
