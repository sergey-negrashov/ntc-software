#ifndef CONFIG_HPP
#define CONFIG_HPP

static char *ENV_IP = (char*)"READOUTD_IP";
static char *ENV_PORT = (char*)"READOUTD_PORT";

static char* DEFAULT_IP = (char*)"127.0.0.1";
static char* DEFAULT_PORT = (char*)"10000";

namespace mtc
{
namespace net
{
class ScrodNet;
}
}

extern "C"
{
mtc::net2::ScrodNet *createConnection();
}
#endif // CONFIG_HPP
