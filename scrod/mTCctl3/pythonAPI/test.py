from readoutdpython import readoutdPython
import time

py = readoutdPython();
#py.trgVetoClear()

#for i in range(0,31):
#	py.trgSoft()

print py.getCards()

exit(0);


#py.calEnSet(0);
#py.setTrgMask(0b111111111111)
#py.trgVetoEnSet(1);
#print py.trgVetoEnGet();
#py.calEnSet(1);
#py.setDelay(700)
#py.calTrgDelaySet(1);
#py.calCoarseDelaySet(0);
##py.setDelay(0x0)
for i in range(0,4):
	for j in range(0,4):
		py.setReg(i,j,171, 1)
		py.setReg(i,j,163, 30)
		py.setReg(i,j,164, 26)

py.calEnSet(1);
for j in range(0,6):
	py.calCoarseDelaySet(j);
	for i in range(0, 86):	
		py.setDelay(i*10);
		time.sleep(0.5);
		py.collectEvents(100, str(i*10) + "." + str(j) + ".dat")

py.calEnSet(0);	
