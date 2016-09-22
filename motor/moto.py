import serial

class moto:
    def __init__(self):
        self.ser = serial.Serial('/dev/ttyUSB0')
    def moveUp(self, numSteps):
        cmd = 'C E I1M' + str(numSteps) + ',R'
        print cmd
        self.ser.write(cmd)
    def zero(self):
        self.ser.write('C E I1M-0, R')
    

a = moto()
a.zero()
