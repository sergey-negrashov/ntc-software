from moto import moto
from time import sleep
from subprocess import call
m = moto()
position = 0
m.moveUp(-4000*105)
count  = 0
while position < 4000*100:
    count+=1
#    call(["/usr/local/bin/drs_ntc", "motordata.glen", "%d" % position])
    m.moveUp(4000)
    position += 4000

