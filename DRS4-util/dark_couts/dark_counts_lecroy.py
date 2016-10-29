import numpy as np
import matplotlib.pyplot as plt
import sys
import os
from scipy.signal import fftconvolve, lfilter, firwin


fir_coef = [-0.00406900049466597,
        0.0014901583160827512,
        0.0022278714276290482,
        0.0034863421930641913,
        0.005262349013769549,
        0.007526937975321521,
        0.010265872128229686,
        0.013462614002518912,
        0.017080624262474767,
        0.021064542544973485,
        0.025343688456984637,
        0.029828825716452217,
        0.034413765259578025,
        0.03898213878353851,
        0.04341086645098334,
        0.047573306814647984,
        0.0513458497439482,
        0.05461325635763496,
        0.05727219604594146,
        0.059236523087357246,
        0.06044200599455753,
        0.060848468662696956,
        0.06044200599455753,
        0.059236523087357246,
        0.05727219604594146,
        0.05461325635763496,
        0.0513458497439482,
        0.047573306814647984,
        0.04341086645098334,
        0.03898213878353851,
        0.034413765259578025,
        0.029828825716452217,
        0.025343688456984637,
        0.021064542544973485,
        0.017080624262474767,
        0.013462614002518912,
        0.010265872128229686,
        0.007526937975321521,
        0.005262349013769549,
        0.0034863421930641913,
        0.0022278714276290482,
        0.0014901583160827512,
        -0.00406900049466597,
        ]

minimums = []
for f in os.listdir("."):
    print f
    data = open(f, "rb")
    times =[]
    voltages = []
    data.readline()
    data.readline()
    data.readline()
    data.readline()
    data.readline()
    for line in data:
        parts = line.split(",")
        times.append(float(parts[0]))
        voltages.append(float(parts[1]))
    voltages = lfilter(fir_coef, 1.0, voltages).tolist()
    #print voltages
    min_index = np.argmin(voltages)

    mean = np.mean(voltages[0: min_index - 20 if min_index - 20 > 0 else 1] +  voltages[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages) - 1])
    minimums.append(mean - min(voltages))

    #plt.xlabel('Time(ns)')
    #plt.ylabel('ADC')
    #plt.title('Signal before cut')
    #plt.plot(times,voltages)
    #plt.show()

    #plt.xlabel('Time(ns)')
    #plt.ylabel('ADC')
    #plt.title('Signal after cut')
    #plt.plot(times[0: min_index - 20 if min_index - 20 > 0 else 1] +  times[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages)], voltages[0: min_index - 20 if min_index - 20 > 0 else 1] +  voltages[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages)])
    #plt.show()


plt.ylabel('Hits')
plt.xlabel('Voltage bellow Mean.(mV)')
plt.title('Dark count measurement with 28 db')
plt.hist(minimums, bins='auto')

plt.show()

