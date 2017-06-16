# -*- coding: utf-8 -*-

from ctypes import *
from numpy.ctypeslib import ndpointer

MAX_PACKET_LENGTH = 42

class readoutdPython:

    #loads the glue library. Creates the connection.
    def __init__(self):
        self.lib = cdll.LoadLibrary('/usr/local/mtc/python/libreadoutd_python2.so')
        self.net = self.lib.createConnection()
        self.lastLength = 0
        self.lib.liveCollectPacket.restype = ndpointer(dtype=c_uint32,shape=(MAX_PACKET_LENGTH,))

        if self.net == 0:
            exit(0)

    def __del__(self):
        self.lib.closeConnection(self.net);

    #Register manipulation
    def getReg(self, card, chan, addr):
        return self.lib.getReg(self.net, card, chan, addr);

    def setReg(self, card, chan, addr, val):
        return self.lib.setReg(self.net, card, chan, addr, val);

    #Get a list of cards.
    #Frst index is the card id, next 4 indexes is the status of the channel
    #Pattern repeats for all cards.
    #This is for compatibility with the old readout scripts.
    #in reality each channel has a unique id calculated as channel+card*4
    def getCards(self):
        ret = list();
        cardNum =  self.lib.cardNum(self.net);
        infoType = c_int*cardNum;
        cardInfo = infoType();
        cardInfo_ptr = pointer(cardInfo);
        self.lib.cardInfo(self.net, cardInfo_ptr)
        for i in range(0, cardNum):
            ret.append(int(cardInfo[i]));
        return ret

    #L1 trigger. Min number of scrods
    def getTrg(self):
        return self.lib.getTrg(self.net)

    def setTrg(self, val):
        return self.lib.setTrg(self.net, val)

    #Trigger mask. 12 bit number.
    def getTrgMask(self):
        return self.lib.getTrgMask(self.net)

    def setTrgMask(self, val):
        return self.lib.setTrgMask(self.net, val)

    #returns the trigger counter
    def getTrgCount(self):
        return self.lib.getTrgCount(self.net)

    #soft trigger via cajipci
    def trgSoft(self):
        return self.lib.trgSoft(self.net)

    #Trigger veto manipulation
    def trgVetoEnGet(self):
        return self.lib.trgVetoEnGet(self.net)

    #To turn on, pass a value of 1 or greater. To turn off, pass a value of 0.
    def trgVetoEnSet(self, on):
        return self.lib.trgVetoEnSet(self.net, on)

    def trgVetoClear(self):
        return self.lib.trgVetoClear(self.net)

    def trgVeto(self):
        return self.lib.trgVeto(self.net)

    #calibration stuff

    #fine calibration delay via the delay line. 10bits
    def getDelay(self):
       return self.lib.calDelayGet(self.net)

    def setDelay(self, on):
        return self.lib.calDelaySet(self.net, on)

    #Calibation mode
    def calEnGet(self):
        return self.lib.calEnGet(self.net)

    #on can be set to 0 to turn off, 1 to turn on
    def calEnSet(self, on):
        return self.lib.calEnSet(self.net, on)

    #Number of the sst cycles between the pulse and the trigger
    def calTrgDelayGet(self):
        return self.lib.calTrgDelayGet(self.net)

    def calTrgDelaySet(self, delay):
        return self.lib.calTrgDelaySet(self.net, delay)

    #Coarse phase of the calibation pulse. 0-5.
    def calCoarseDelayGet(self):
        return self.lib.calCoarseDelayGet(self.net)

    def calCoarseDelaySet(self, delay):
        return self.lib.calCoarseDelaySet(self.net, delay)

    #read events to file
    def collectEvents(self, num, fname, timeout = -1):
        cfn = c_char_p(fname);
        return self.lib.collectEvents(num,cfn, timeout)

    #rotate the server data file;
    def rotateStorageFile(self):
        return self.lib.rotateStorageFile(self.net)

    # New trigger accessors
    # Trig Enable is 8-bits, upper 4 reserved,
    #                        lower 4 are mapped MSB to LSB as: AB, C, B, A
    def getTrgEn(self):
        return self.lib.getTrgEn(self.net)
    def setTrgEn(self, val):
        return self.lib.setTrgEn(self.net, val)
    # Number of hits to set off A:
    # if (minA < nHits < maxA) then A has been seen
    def getMinA(self):
        return self.lib.getMinA(self.net)
    def setMinA(self, val):
        return self.lib.setMinA(self.net, val)
    def getMaxA(self):
        return self.lib.getMaxA(self.net)
    def setMaxA(self, val):
        return self.lib.setMaxA(self.net, val)
    # As above for trigger B
    def getMinB(self):
        return self.lib.getMinB(self.net)
    def setMinB(self, val):
        return self.lib.setMinB(self.net, val)
    def getMaxB(self):
        return self.lib.getMaxB(self.net)
    def setMaxB(self, val):
        return self.lib.setMaxB(self.net, val)
    # As above for trigger C
    def getMinC(self):
        return self.lib.getMinC(self.net)
    def setMinC(self, val):
        return self.lib.setMinC(self.net, val)
    def getMaxC(self):
        return self.lib.getMaxC(self.net)
    def setMaxC(self, val):
        return self.lib.setMaxC(self.net, val)
    # Minimum allowed delay between an A and a B to issue an AB
    def getMinDelayAB(self):
        return self.lib.getMinDelayAB(self.net)
    def setMinDelayAB(self, val):
        return self.lib.setMinDelayAB(self.net, val)
    # Maxmimum delay between an A and a B before timing out on an AB
    # Delay is in 21.33 MHz clock cycles.
    def getMaxDelayAB(self):
        return self.lib.getMaxDelayAB(self.net)
    def setMaxDelayAB(self, val):
        return self.lib.setMaxDelayAB(self.net, val)
    # Prescaler settings
    # These are 8-bit values that define the number of events to skip between
    # issuing the respective triggers
    def getPrescaleA(self):
        return self.lib.getPrescaleA(self.net)
    def setPrescaleA(self, val):
        return self.lib.setPrescaleA(self.net, val)
    def getPrescaleB(self):
        return self.lib.getPrescaleB(self.net)
    def setPrescaleB(self, val):
        return self.lib.setPrescaleB(self.net, val)
    def getPrescaleC(self):
        return self.lib.getPrescaleC(self.net)
    def setPrescaleC(self, val):
        return self.lib.setPrescaleC(self.net, val)
    def getPrescaleAB(self):
        return self.lib.getPrescaleAB(self.net)
    def setPrescaleAB(self, val):
        return self.lib.setPrescaleAB(self.net, val)
    # Trigger link status, read only (11 bits, 1 - synced, 0 - not synced)
    def getTrgLinkStatus(self):
        return self.lib.getTrgLinkStatus(self.net)
    # Get live time fraction, read only (8 bits, live % = value/256*100)
    def getLiveTimeFraction(self):
        return self.lib.getLiveTimeFraction(self.net)
    # Get trigger rates, 16-bits, number of triggers / 10 s
    def getTriggerARate(self):
        return self.lib.getTriggerARate(self.net)
    def getTriggerBRate(self):
        return self.lib.getTriggerBRate(self.net)
    def getTriggerCRate(self):
        return self.lib.getTriggerCRate(self.net)
    def getTriggerABRate(self):
        return self.lib.getTriggerABRate(self.net)

    # Added methods to allow online data collection through python API
    # These should be used for monitoring only, they are not at all guaranteed
    # to keep up with data bandwidth.
    def openMonitor(self,selectFlag,scrod = 0, channelStart = 0, channelStop = 0):
        self.collectNet = self.lib.openMonitor(selectFlag,
                                               c_uint16(scrod),
                                               c_uint8(channelStart),
                                               c_uint8(channelStop))
        return        

    def liveCollectPacket(self, timeout = -1):
        length = c_int32()
        data = self.lib.liveCollectPacket(self.collectNet,timeout,byref(length))
        self.lastLength = length.value
        if self.lastLength > 0:
            return data
        else:
            return None

    def liveCollectLastLength(self):
        return self.lastLength

	def advanceMotor(self, steps):
		return self.lib.advanceMotor(self.net, steps)

#Create a dictionary to link command line input to the actual functions more quickly than if statements?



