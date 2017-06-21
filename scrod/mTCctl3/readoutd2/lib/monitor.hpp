#ifndef MONITOR_HPP
#define MONITOR_HPP

#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <string>
#include <map>

#include "lib/common.hpp"

class MtcHealthMonitor
{
public:
    static MtcHealthMonitor* Instance();
    MtcHealth getState();

private:
    MtcHealthMonitor();
    MtcHealthMonitor(MtcHealthMonitor const&){}
    MtcHealthMonitor& operator=(MtcHealthMonitor const&){}
    static MtcHealthMonitor* inst;
    boost::mutex mutex;
};

#endif // MONITOR_HPP
