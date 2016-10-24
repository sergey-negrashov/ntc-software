import xml.etree.cElementTree
import matplotlib.pyplot as plt
import numpy as np
import sys
from scipy.signal import fftconvolve, lfilter, firwin


fir_coef = [-0.0031969360962557777,
        -0.005086159316412403,
        -0.008098989848820957,
        -0.010974258716589618,
        -0.012939462173545403,
        -0.012810228564375277,
        -0.009590665294909258,
        -0.0022964344511770965,
        0.009435822168123784,
        0.025513163299239153,
        0.044970226575298006,
        0.06629653734109198,
        0.08735399524106294,
        0.1058781009237327,
        0.11965514824783034,
        0.12701225676137842,
        0.12701225676137842,
        0.11965514824783034,
        0.1058781009237327,
        0.08735399524106294,
        0.06629653734109198,
        0.044970226575298006,
        0.025513163299239153,
        0.009435822168123784,
        -0.0022964344511770965,
        -0.009590665294909258,
        -0.012810228564375277,
        -0.012939462173545403,
        -0.010974258716589618,
        -0.008098989848820957,
        -0.005086159316412403,
        -0.0031969360962557777
        ]

e = xml.etree.cElementTree.parse(sys.argv[1]).getroot()
minimums = []
for atype in e.findall('Event'):
    strs =  map(lambda x: x.text.split(","), atype.find('Board_2675').find('CHN1').findall('Data'))
    times =[]
    voltages = []
    map(lambda x: times.append(float(x[0])) or voltages.append(float(x[1])), strs)
    voltages = lfilter(fir_coef, 1.0, voltages).tolist()
    #print voltages
    min_index = np.argmax(voltages)

    mean = np.mean(voltages[0: min_index - 20 if min_index - 20 > 0 else 1] +  voltages[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages) - 1])
    minimums.append(max(voltages) - mean)

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
plt.xlabel('ADC counts above Mean')
plt.title('Dark count measurement')
plt.hist(minimums, bins='auto')

plt.show()

