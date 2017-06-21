from readoutdpython import readoutdPython
from time import sleep
py = readoutdPython();
py.advanceMotor(-100000)
sleep(60)
for i in range (0, 100):
	print(py.advanceMotor(1000))
	sleep(10)
