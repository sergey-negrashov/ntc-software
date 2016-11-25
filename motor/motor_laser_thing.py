from moto import moto
from time import sleep
from subprocess import call
m = moto()
position = 0
#m.zero()
#m.moveUp(-4000*1)
count  = 0
while position < 360000:
    count+=1
    if (not count == 20) or (not count == 19):
        call(["/usr/local/bin/drs_ntc", "motordata.glen", "%d" % position])
    m.moveUp(4000)
    position += 4000
    print count
    #sleep(5)

