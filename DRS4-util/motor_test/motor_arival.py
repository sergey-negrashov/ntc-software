import numpy as np
import matplotlib.pyplot as plt
import sys
import os
import math
from scipy.signal import fftconvolve, lfilter, firwin
from multiprocessing import Pool, TimeoutError, Lock, Manager

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
        if math.fabs(voltages_in[i] - voltages_in[i - 1]) > 2 and math.fabs(voltages_in[i + 1] - voltages_in[i]) > 2:
            voltages[i] = voltages[i - 1]
    return voltages

def pulse_50_percent_time(voltages, times):
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
    
def process_file(f):
    try:
        actual_times = np.linspace(0.0, 204.8, num=1024)
        difference = []
        maxes1=[]
        maxes2=[]
        data = open(f, "rb")
        times1 =[]
        voltages1 = []
        
        times2 =[]
        voltages2 = []
        data.readline()
        
        for line in data:
            if line[0] == '#':
                uv1 = voltages1[100:]
                ut1 = times1[100:]
                uv2 = voltages2[100:]
                ut2 = times2[100:]
                
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

                
                tdr1,max1 =  pulse_50_percent_time(voltages1, times1)
                tdr2,max2 =  pulse_50_percent_time(voltages2, times2)    
                if not tdr1 == -1 and not tdr2 == -1 and math.fabs(tdr2 - tdr1) < 10:
                    difference.append(tdr2 - tdr1)
                    maxes1.append(max1)
                    maxes2.append(max2)
                if not tdr1 == -1 and not tdr2 == -1 and math.fabs(tdr2 - tdr1) > 50:
                    line1, line2 = plt.plot(times2, voltages2, ut2, uv2);
                    plt.title("Pathology")
                    plt.xlabel("Time(ns)")
                    plt.ylabel("Voltage(V)")
                    line1.set_label("filtered")
                    line2.set_label("unfiltered")
                    plt.legend()
                    plt.show()
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
        plt.hist(difference, 100, range=[-5, 80])
        plt.ylabel('Hits')
        plt.xlabel('Time Difference')
        plt.title('Distribution for position ' +  str(int(f[25:-4])/4000) + 'cm')
        #plt.savefig('../gif/pos' + str(int(f[25:-4])/4000) + '.png')
        plt.close()
        #dif_time.append(np.mean(difference))
        #pos.append(float(f[25:-4])/4000.0)
        #ampl1.append(np.mean(maxes1))
        #ampl2.append(np.mean(maxes2))
        return float(f[25:-4])/4000.0, np.mean(difference), np.mean(maxes1), np.mean(maxes2), np.std(difference), np.std(maxes1),np.std(maxes2)
    except Exception as a:
        print a

files = sorted(os.listdir("."))
pool = Pool(processes=1) 
multiple_results = map(lambda f: pool.apply_async(process_file, (f,)), files)
results =  [res.get() for res in multiple_results]
pos = []
dif_time = []
ampl1 = []
ampl2 = []
std_diff = []
std_amp1 = []
std_amp2 = []

for result in results:
    pos.append(result[0])
    dif_time.append(result[1])
    ampl1.append(result[2])
    ampl2.append(result[3])
    std_diff.append(result[4])
    std_amp1.append(result[5])
    std_amp2.append(result[6])

exit(0)

plt.ylabel('Time Difference Between Pulses(ns)')
plt.xlabel('Motor position(cm)')
plt.title('50% TDR time difference 1000 Events per point')
plt.plot(pos,dif_time,"r")
plt.savefig('../difference.png')
plt.close()
plt.ylabel('Standard Deviation of Time Difference (ns)')
plt.xlabel('Motor position(cm)')
plt.title('Standard Deviation of time difference 1000 Events per point')
plt.plot(pos,std_diff,"r")
plt.savefig('../difference_std.png')
plt.close()
plt.ylabel('Amplitudes(mV)')
plt.xlabel('Motor position(cm)')
plt.title('Average amplitudes 1000 Events per point')
plt.plot(pos, ampl1, "r", pos, ampl2, "b")
plt.savefig('../ampl.png')
plt.close()
plt.ylabel('Standard Deviation Amplitudes(mV)')
plt.xlabel('Motor position(cm)')
plt.title('Standard Deviation of amplitudes 1000 Events per point')
plt.plot(pos, std_amp1, "r", pos, std_amp2, "b")
plt.savefig('../ampl_std.png')
plt.close()
