#include "lib/monitor.hpp"
#include "lib/settings.hpp"
#include "lib/common.hpp"
#include "lib/trigger.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <map>

using namespace std;

MtcHealthMonitor* MtcHealthMonitor::inst = NULL;

MtcHealthMonitor* MtcHealthMonitor::Instance()
{
    if(!inst)
        inst = new MtcHealthMonitor();
    return inst;
}

MtcHealthMonitor::MtcHealthMonitor()
{
    MtcState *state = MtcState::Instance();
}

MtcHealth MtcHealthMonitor::getState()
{
    boost::unique_lock<boost::mutex> scoped_lock(string);
    MtcHealth h;

    Trigger* t = Trigger::Instance();

    //New trigger related variables / states
    h.trgEn = t->getEnable();
    h.trgAMin = t->getMinA();
    h.trgAMax = t->getMaxA();
    h.trgBMin = t->getMinB();
    h.trgBMax = t->getMaxB();
    h.trgCMin = t->getMinC();
    h.trgCMax = t->getMaxC();

    h.minDelayAB = t->getMinDelayAB();
    h.maxDelayAB = t->getMaxDelayAB();

    h.prescaleA = t->getPrescaleA();
    h.prescaleB = t->getPrescaleB();
    h.prescaleC = t->getPrescaleC();
    h.prescaleAB = t->getPrescaleAB();

    h.trgARate = float(t->getTrgARate())/10.0;
    h.trgBRate = float(t->getTrgBRate())/10.0;
    h.trgCRate = float(t->getTrgCRate())/10.0;
    h.trgABRate = float(t->getTrgABRate())/10.0;

    h.trgLinkStatus = t->getTrgLinkStatus();
    h.liveTimeFraction = float(t->getLiveTime())/255.0;


    return h;
}
