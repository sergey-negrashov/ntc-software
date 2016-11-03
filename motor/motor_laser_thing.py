from moto import moto
from time import sleep
from subprocess import call
m = moto()
position = 0
#m.zero()
m.moveUp(-1000000)
count  = 0
while position < 120000 :
    count+=1
    if (not count == 3) or (not count == 3):
        call(["/usr/local/bin/drs_ntc", "%d" % position])
    m.moveUp(4000)
    position += 4000
    print count
    #sleep(2)

