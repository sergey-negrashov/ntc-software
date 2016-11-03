import numpy as np
import matplotlib.pyplot as plt
import sys
import os
from scipy.signal import fftconvolve, lfilter, firwin

fir_coef = [0.005836310228843312,
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


def get_rid_of_random_god_damn_spikes(voltages_in):
    voltages = voltages_in[:]
    for i in range(1, len(voltages) - 1):
        if voltages_in[i] - voltages_in[i - 1] > 5 and voltages_in[i + 1] - voltages_in[i] < 5:
            voltages[i] = voltages[i - 1] 
    return voltages

def pulse_50_percent_time(voltages, times):
    max_index = voltages.index(max(voltages))
    max_value = max(voltages)
    mean = np.mean(voltages[0: max_index - 20 if max_index - 20 > 0 else 1] +  voltages[max_index + 20 if max_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages) - 1])
    target = (max_value)/2.0
    for index in range(max_index - 40, max_index)[::-1]:
        if voltages[index] >= target and voltages[index - 1] <= target:
            #print [times[index - 1], times[index], np.interp(target, [voltages[index - 1], voltages[index]], [times[index - 1], times[index]]), target]
            return np.interp(target, [voltages[index - 1], voltages[index]], [times[index - 1], times[index]])
    return -1
    
    


actual_times = np.linspace(0.0, 204.8, num=1024)

pos = []
dif_time = []

for f in os.listdir("."):
    difference = []
    data = open(f, "rb")
    times1 =[]
    voltages1 = []
    
    times2 =[]
    voltages2 = []
    data.readline()
    
    for line in data:
        if line[0] == '#':            
            voltages1 = get_rid_of_random_god_damn_spikes(voltages1)
            voltages1 = np.interp(actual_times, times1, voltages1)
            voltages1 = lfilter(fir_coef, 1.0, voltages1).tolist()
            
            voltages1 = voltages1[100:]
            times1 = actual_times[100:]

            voltages2 = get_rid_of_random_god_damn_spikes(voltages2)
            voltages2 = np.interp(actual_times, times2, voltages2)
            voltages2 = lfilter(fir_coef, 1.0, voltages2).tolist()

            voltages2 = voltages2[100:]
            times2 = actual_times[100:]

            
            tdr1 =  pulse_50_percent_time(voltages1, times1)
            tdr2 =  pulse_50_percent_time(voltages2, times2)    
            if not tdr1 == -1 and not tdr2 == -1:
                difference.append(tdr2 - tdr1)

            #plt.plot(times1, voltages1, "r--")
            #plt.axvline(tdr1, hold = "False")
            #plt.show()
            times1 = []
            times2 = []
            voltages1 = []
            voltages2 = []
            continue
        parts = line.split()
        times1.append(float(parts[0]))
        times2.append(float(parts[2]))
        voltages1.append(float(parts[1]))
        voltages2.append(float(parts[3]))
    pos.append(float(f[25:-4])/4000.0)
    dif_time.append(np.mean(difference))
    print float(f[25:-4])/4000.0
    
plt.ylabel('Time difference between pulses(ns)')
plt.xlabel('Motor position(cm)')
plt.title('50% TDR time difference 1000 Events per point')
plt.plot(pos,dif_time,"r*")
plt.show()
