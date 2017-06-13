#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <mtc/readoutd2/scrodnet.hpp>
#include "../lib/config.hpp"

using namespace std;
using mtc::net2::ScrodNet;

extern "C"
{
int getTrg(ScrodNet* net)
{
    try
    {
        return net->getTriggerMin();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setTrg(ScrodNet* net, int val)
{
    try
    {
        return net->setTriggerMin(val);
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getTrgMask(ScrodNet* net)
{
    try
    {
        return net->getTriggerMask();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setTrgMask(ScrodNet* net, int val)
{
    try
    {
        net->setTriggerMask(val);
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getTrgCount(ScrodNet* net)
{
    try
    {
        return net->getTriggerCount();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int trgSoft(ScrodNet* net)
{
    try
    {
        net->softTrigger();
        return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int trgVetoEnGet(ScrodNet* net)
{
    try
    {
        return net->getVetoEnabled();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int trgVetoEnSet(ScrodNet* net, bool on)
{
    try
    {
        net->enableVeto(on);
        return  0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int trgVetoClear(ScrodNet* net)
{
    try
    {
        net->clearVeto();
        return  0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int trgVeto(ScrodNet* net)
{
    try
    {
        net->getVeto();
        return  0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getTrgEn(ScrodNet* net)
{
    try
    {
        return net->getEnable();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setTrgEn(ScrodNet* net, int val)
{
    try
    {
        net->setEnable(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMinA(ScrodNet* net)
{
    try
    {
        return net->getMinA();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMinA(ScrodNet* net, int val)
{
    try
    {
        net->setMinA(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMaxA(ScrodNet* net)
{
    try
    {
        return net->getMaxA();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMaxA(ScrodNet* net, int val)
{
    try
    {
        net->setMaxA(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMinB(ScrodNet* net)
{
    try
    {
        return net->getMinB();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMinB(ScrodNet* net, int val)
{
    try
    {
        net->setMinB(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMaxB(ScrodNet* net)
{
    try
    {
        return net->getMaxB();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMaxB(ScrodNet* net, int val)
{
    try
    {
        net->setMaxB(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMinC(ScrodNet* net)
{
    try
    {
        return net->getMinC();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMinC(ScrodNet* net, int val)
{
    try
    {
        net->setMinC(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMaxC(ScrodNet* net)
{
    try
    {
        return net->getMaxC();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMaxC(ScrodNet* net, int val)
{
    try
    {
        net->setMaxC(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMinDelayAB(ScrodNet* net)
{
    try
    {
        return net->getMinDelayAB();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMinDelayAB(ScrodNet* net, int val)
{
    try
    {
        net->setMinDelayAB(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getMaxDelayAB(ScrodNet* net)
{
    try
    {
        return net->getMaxDelayAB();
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int setMaxDelayAB(ScrodNet* net, int val)
{
    try
    {
        net->setMaxDelayAB(val); return 0;
    }
    catch(std::runtime_error &e)
    {
        cout << e.what() << endl;
        return -1;
    }
}

int getPrescaleA(ScrodNet* net) {
    try { return net->getPrescaleA(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int setPrescaleA(ScrodNet* net, int val) {
    try { net->setPrescaleA(val); return 0; }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getPrescaleB(ScrodNet* net) {
    try { return net->getPrescaleB(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int setPrescaleB(ScrodNet* net, int val) {
    try { net->setPrescaleB(val); return 0;}
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getPrescaleC(ScrodNet* net) {
    try { return net->getPrescaleC(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int setPrescaleC(ScrodNet* net, int val) {
    try { net->setPrescaleC(val); return 0;}
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getPrescaleAB(ScrodNet* net) {
    try { return net->getPrescaleAB(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int setPrescaleAB(ScrodNet* net, int val) {
    try { net->setPrescaleAB(val); return 0;}
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getTrgLinkStatus(ScrodNet* net) {
    try { return net->getTrgLinkStatus(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getLiveTimeFraction(ScrodNet* net) {
    try { return net->getLiveTimeFraction(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getTriggerARate(ScrodNet* net) {
    try { return net->getTriggerARate(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getTriggerBRate(ScrodNet* net) {
    try { return net->getTriggerBRate(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getTriggerCRate(ScrodNet* net) {
    try { return net->getTriggerCRate(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}
int getTriggerABRate(ScrodNet* net) {
    try { return net->getTriggerABRate(); }
    catch(std::runtime_error &e) { cout << e.what() << endl; return -1;}
}

}
