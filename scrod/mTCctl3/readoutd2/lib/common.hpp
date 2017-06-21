#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>
#include <stdint.h>
#include <map>

#include "coredataqueue.hpp"

using std::string;

//Configuration parsing

static const string REGISTER_PREFACE = "register.";

static const string LISTNER_PREFACE = "data.";

static const string LISTNER_STATE = ".running";

static const string LISTNER_CONNECTED = ".connected";

static const string LISTNER_TOTAL_PACKETS = ".packets";

static const string LISTNER_TOTAL_BYTES = ".MB";

static const string LISTNER_RATE_PACKETS = ".pps";

static const string LISTNER_RATE_BYTES = ".MBps";

static const string LISTNER_BAD_PACKETS = ".errors";

static const string SETTING_PORT = "port";

static const string SETTING_IP_LOCAL = ".local";

static const string SETTING_IP_REMOTE = ".remote";

static const string SETTING_INTERFACE = ".interface";

static const string SETTING_PORT_CLIENT = "client.port";

static const string STATE_PORT = "state.port";

static const string ACTIVE_SCRODS = "scrods";

static const string STORAGE_QUEUE_SIZE = "storage.scrodEventBuffer";

static const string STORAGE_PATH = "storage.path";

static const string MONITOR_PREFACE = "monitor.";

static const string MONITOR_MPOD_IP = "mpodIp";

static const string MONITOR_PIP_IP = "pipIp";

static const string MONITOR_FLOW_IP = "flowIp";

//Check the cheksum
static const bool CHECK_PACKET_CHECKSUM = true;

//Path to the configuration
static const string PATH_TO_CONFIGURATION_FILE = "/usr/local/etc/mtc/settings.set";

typedef pland::DataQueue < uint32_t* > StorageQueue;

typedef boost::shared_ptr< StorageQueue > StorageQueuePtr;

struct MtcHealth
{
    //triggering stuff from Cajipci, grab functions from readoutd python C api
    unsigned int trgEn;
    unsigned int trgAMin;
    unsigned int trgAMax;
    unsigned int trgBMin;
    unsigned int trgBMax;
    unsigned int trgCMin;
    unsigned int trgCMax;

    unsigned int minDelayAB;
    unsigned int maxDelayAB;

    unsigned int prescaleA;
    unsigned int prescaleB;
    unsigned int prescaleC;
    unsigned int prescaleAB;

    float trgARate;
    float trgBRate;
    float trgCRate;
    float trgABRate;

    unsigned int trgLinkStatus;
    float liveTimeFraction;

};




#endif // COMMON_H
