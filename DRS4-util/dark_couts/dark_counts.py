import xml.etree.cElementTree
import matplotlib.pyplot as plt
import numpy as np
import sys

e = xml.etree.cElementTree.parse(sys.argv[1]).getroot()
minimums = []
for atype in e.findall('Event'):
    strs =  map(lambda x: x.text.split(","), atype.find('Board_2675').find('CHN2').findall('Data'))
    times =[]
    voltages = []
    map(lambda x: times.append(float(x[0])) or  voltages.append(float(x[1])), strs)
    min_index = np.argmin(voltages)
    mean = np.mean(voltages[0: min_index - 20 if min_index - 20 > 0 else 1] +  voltages[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages) - 1])
    minimums.append(mean - min(voltages))

    plt.xlabel('Time(ns)')
    plt.ylabel('ADC')
    plt.title('Signal before cut')
    plt.plot(times,voltages)
    plt.show()

    plt.xlabel('Time(ns)')
    plt.ylabel('ADC')
    plt.title('Signal after cut')
    plt.plot(times[0: min_index - 20 if min_index - 20 > 0 else 1] +  times[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages)], voltages[0: min_index - 20 if min_index - 20 > 0 else 1] +  voltages[min_index + 20 if min_index + 20 < len(voltages) else len(voltages) - 1 : len(voltages)])
    plt.show()


plt.xlabel('Hits')
plt.ylabel('ADC counts above Mean')
plt.title('Dark count measurement after 2.5 Voltage gain amp')
plt.hist(minimums, bins='auto')

plt.show()

