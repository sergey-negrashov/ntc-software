#ifndef SETTINGS_HPP
#define SETTINGS_HPP
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/stream_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>
#include <map>

class MtcState
{
public:
    static MtcState* Instance();
    static void initialize(std::string);
    std::string getValue(std::string);
    void setValue(std::string, std::string);
    std::vector<std::string> keys();
private:
    MtcState(std::string fname);
    MtcState(){}
    MtcState(MtcState const&){}
    MtcState& operator=(MtcState const&){}
    static MtcState* inst;


    std::map<std::string, std::string> kv;
    boost::mutex mutex;

    boost::thread thr;
    void webService();
};

#endif // SETTINGS_HPP
