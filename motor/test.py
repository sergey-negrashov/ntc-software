class moto:
    def moveUp(self, numSteps):
        cmd = 'C E I1M' + str(numSteps) + ',R'
        print(cmd)
    def zero(self):
        print('C E I1M-0, R')
    def absmove(self, pos):
        cmd = 'C E IA1M' + str(pos) + ',R'
        print(cmd)
    def paus(self, time):
        cmd = 'C E P' + str(time) + ',R'
        print(cmd)
    def loop(self, sec, steps, loops):
        cmd = 'C E P' + str(sec*10) + ',I1M' + str(steps) + ',L' + str(loops) + ',R'
        print(cmd)


a = moto()
a.moveUp(10000)

