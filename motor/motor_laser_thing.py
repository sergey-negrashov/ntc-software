from moto import moto
from time import sleep
from subprocess import call
m = moto()
position = 0
m.zero()
count  = 0
while position < 120000 :
    count+=1
    call(["/usr/local/bin/drs_ntc", "%d" % position])
    m.moveUp(4000)
    position += 4000
    print count

