#include "lib/settings.hpp"
#include "lib/monitor.hpp"
#include "lib/common.hpp"
#include "lib/trigger.hpp"

#include <iostream>
#include <fstream>
#include "lib/web++.hpp"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

using namespace std;

MtcState* MtcState::inst = NULL;

void xmlResponce(WPP::Request* req, WPP::Response* res)
{
    MtcState * state = MtcState::Instance();

    MtcHealthMonitor * m = MtcHealthMonitor::Instance();
    MtcHealth h = m->getState();
    res->body << "<mTCState>\n";
    res->body << "\t<health>\n";

	//Triggering
    res->body << "\t\t<trig>\n";

    res->body << "\t\t\t<triggers>\n";

    res->body << "\t\t\t\t<trgEn>";
    res->body << boost::lexical_cast<string>(h.trgEn);
    res->body << "</trgEn>\n";

    res->body << "\t\t\t\t<trgAMin>";
    res->body << boost::lexical_cast<string>(h.trgAMin);
    res->body << "</trgAMin>\n";
    res->body << "\t\t\t\t<trgAMax>";
    res->body << boost::lexical_cast<string>(h.trgAMax);
    res->body << "</trgAMax>\n";
    res->body << "\t\t\t\t<trgBMin>";
    res->body << boost::lexical_cast<string>(h.trgBMin);
    res->body << "</trgBMin>\n";
    res->body << "\t\t\t\t<trgBMax>";
    res->body << boost::lexical_cast<string>(h.trgBMax);
    res->body << "</trgBMax>\n";
    res->body << "\t\t\t\t<trgCMin>";
    res->body << boost::lexical_cast<string>(h.trgCMin);
    res->body << "</trgCMin>\n";
    res->body << "\t\t\t\t<trgCMax>";
    res->body << boost::lexical_cast<string>(h.trgCMax);
    res->body << "</trgCMax>\n";

    res->body << "\t\t\t</triggers>\n";

    res->body << "\t\t\t<prescalers>\n";
	res->body << "\t\t\t\t<prescaleA>";
    res->body << boost::lexical_cast<string>(h.prescaleA);
    res->body << "</prescaleA>\n";
	res->body << "\t\t\t\t<prescaleB>";
    res->body << boost::lexical_cast<string>(h.prescaleB);
    res->body << "</prescaleB>\n";
	res->body << "\t\t\t\t<prescaleC>";
    res->body << boost::lexical_cast<string>(h.prescaleC);
    res->body << "</prescaleC>\n";
	res->body << "\t\t\t\t<prescaleAB>";
    res->body << boost::lexical_cast<string>(h.prescaleAB);
    res->body << "</prescaleAB>\n";
    res->body << "\t\t\t</prescalers>\n";

    res->body << "\t\t\t<rates>\n";
	res->body << "\t\t\t\t<trgARate>";
    res->body << boost::lexical_cast<string>(h.trgARate);
    res->body << "</trgARate>\n";
	res->body << "\t\t\t\t<trgBRate>";
    res->body << boost::lexical_cast<string>(h.trgBRate);
    res->body << "</trgBRate>\n";
	res->body << "\t\t\t\t<trgCRate>";
    res->body << boost::lexical_cast<string>(h.trgCRate);
    res->body << "</trgCRate>\n";
	res->body << "\t\t\t\t<trgABRate>";
    res->body << boost::lexical_cast<string>(h.trgABRate);
    res->body << "</trgABRate>\n";
    res->body << "\t\t\t</rates>\n";

    res->body << "\t\t\t<status>\n";
	res->body << "\t\t\t\t<minDelayAB>";
    res->body << boost::lexical_cast<string>(h.minDelayAB);
    res->body << "</minDelayAB>\n";
	res->body << "\t\t\t\t<maxDelayAB>";
    res->body << boost::lexical_cast<string>(h.maxDelayAB);
    res->body << "</maxDelayAB>\n";
	res->body << "\t\t\t\t<trgLinkStatus>";
    res->body << boost::lexical_cast<string>(h.trgLinkStatus);
    res->body << "</trgLinkStatus>\n";
	res->body << "\t\t\t\t<liveTimeFraction>";
    res->body << boost::lexical_cast<string>(h.liveTimeFraction);
    res->body << "</liveTimeFraction>\n";
    res->body << "\t\t\t</status>\n";

    res->body << "\t\t</trig>\n";

	//Readout
    res->body << "\t</health>\n";
    auto keys = state->keys();
    res->body << "\t<readoutd2>\n";
    for(int i = 0; i < keys.size(); i++)
    {
        res->body << "\t\t<" << keys[i] << "> "
                  << state->getValue(keys[i])
                  << " </" << keys[i] << ">\n";
    }
    res->body << "\t</readoutd2>\n";
    res->body << "</mTCState>\n";
}

void webResponse(WPP::Request* req, WPP::Response* res)
{

    MtcHealthMonitor * m = MtcHealthMonitor::Instance();
    MtcHealth h = m->getState();
    res->body << "<head><META HTTP-EQUIV=\"refresh\" CONTENT=\"5\"></head>\n";

    res->body << "<center><h3>Flow:</h3></center>\n";


    res->body << "<center><h3>Trigger Configuration:</h3></center>\n";

    res->body << "<table  style=\"width:100%;border: 1px solid black\">\n";
    res->body << "<td> Trigger </td><td> Status </td><td> Min Reqd. Hits </td><td> Max Reqd. Hits </td><td> Prescale </td><td> Min Delay </td><td> Max Delay </td>";
    res->body << "<tr><td colspan=\"7\"><hr/></tr>";
    res->body << "<tr>\n"
                  <<" <td> Type A </td> "
                  <<" <td> " << (h.trgEn & 0x1 ? "Enabled" : "Disabled") << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgAMin) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgAMax) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.prescaleA) << "</td>"
                  <<" <td> --- </td> "
                  <<" <td> --- </td> "
                  << "</tr>\n";
    res->body << "<tr>\n"
                  <<" <td> Type B </td> "
                  <<" <td> " << (h.trgEn & 0x2 ? "Enabled" : "Disabled") << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgBMin) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgBMax) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.prescaleB) << "</td>"
                  <<" <td> --- </td> "
                  <<" <td> --- </td> "
                  << "</tr>\n";
    res->body << "<tr>\n"
                  <<" <td> Type C </td> "
                  <<" <td> " << (h.trgEn & 0x4 ? "Enabled" : "Disabled") << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgCMin) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.trgCMax) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.prescaleC) << "</td>"
                  <<" <td> --- </td> "
                  <<" <td> --- </td> "
                  << "</tr>\n";
    res->body << "<tr>\n"
                  <<" <td> Type AB </td> "
                  <<" <td> " << (h.trgEn & 0x8 ? "Enabled" : "Disabled") << "</td>"
                  <<" <td> --- </td>"
                  <<" <td> --- </td>"
                  <<" <td> " << boost::lexical_cast<string>(h.prescaleAB) << "</td>"
                  <<" <td> " << boost::lexical_cast<string>(h.minDelayAB) << "</td> "
                  <<" <td> " << boost::lexical_cast<string>(h.maxDelayAB) << "</td> "
                  << "</tr>\n";
    res->body     << "</table>";

    res->body << "<center><h3>Trigger Status:</h3></center>\n";
    res->body << "<table  style=\"width:100%;border: 1px solid black\">\n";
    res->body << "<td> </td><td> Type A </td><td> Type B </td><td> Type C </td><td> Type AB </td>";
    res->body << "<tr><td></td><td colspan=\"4\"><hr/></tr>";
    res->body << "<tr>\n"
                  <<" <td> Rates (Hz) </td> "
                  //<<" <td> " << boost::lexical_cast<string>(h.trgARate) << "</td>"
                  //<<" <td> " << boost::lexical_cast<string>(h.trgBRate) << "</td>"
                  //<<" <td> " << boost::lexical_cast<string>(h.trgCRate) << "</td>"
                  //<<" <td> " << boost::lexical_cast<string>(h.trgABRate) << "</td>"
                  <<" <td> " << boost::str(boost::format("%.1f") % h.trgARate) << "</td>"
                  <<" <td> " << boost::str(boost::format("%.1f") % h.trgBRate) << "</td>"
                  <<" <td> " << boost::str(boost::format("%.1f") % h.trgCRate) << "</td>"
                  <<" <td> " << boost::str(boost::format("%.1f") % h.trgABRate) << "</td>"
                  << "</tr>\n";
    res->body << "<tr><td colspan=\"5\"><hr/></tr>";
    res->body << "<td>  </td><td> Link Status </td><td>  </td><td> Live Time(%) </td><td> </td>";
    res->body << "<tr><td></td><td colspan=\"4\"><hr/></tr>";
    res->body << "<tr>\n"
                  <<" <td> </td> ";
                  if (h.trgLinkStatus == 0xFFF) 
                      res->body <<" <td> ALL UP </td> ";
                  else 
                      res->body <<" <td> " <<  hex << "0x" << h.trgLinkStatus << " </td> ";
                  float liveTimeFraction = h.liveTimeFraction*100.0;
        res->body <<" <td> </td>"
                  <<" <td> " <<  boost::str(boost::format("%.1f") % liveTimeFraction) << "</td>"
                  << "</tr>\n";
    res->body << "</table>";

    res->body << "<center><h3>Readoutd2 internals:</h3></center>\n";
    MtcState * state = MtcState::Instance();
    auto keys = state->keys();

    res->body << "<table  style=\"width:100%;border: 1px solid black\">\n";
    for(int i = 0; i < keys.size(); i++)
    {
        res->body << "<tr>\n";
        res->body << "<td>" << keys[i] << "</td><td> " << state->getValue(keys[i]) << "</td>\n";
        res->body << "</tr>\n";
    }
    res->body << "</table>";
}


MtcState* MtcState::Instance()
{
    if(!inst)
        return NULL;
    return inst;
}

void MtcState::initialize(std::string fname)
{
    inst = new MtcState(fname);
}

MtcState::MtcState(string fname)
{
    ifstream fin(fname);
    for (string line; getline(fin, line, '\n'); )
    {
        if(line.size() < 3 || line.at(0) == '#') continue;
        std::vector<std::string> s;
        boost::split(s, line, boost::is_any_of(":"));
        if(s.size() != 2) continue;
        boost::trim(s[0]);
        boost::trim(s[1]);
        kv[s[0]] = s[1];
    }
    thr = boost::thread(&MtcState::webService, this);
}

string MtcState::getValue(string key)
{
    boost::unique_lock<boost::mutex> scoped_lock(mutex);
    if(kv.find(key) == kv.end())
        throw std::runtime_error("tried to get value " + key + " but it is not set.");
    return kv[key];
}

void MtcState::setValue(string key, string val)
{
    boost::unique_lock<boost::mutex> scoped_lock(mutex);
    kv[key] = val;
}

std::vector<std::string> MtcState::keys()
{
    vector<string> ret;
    for(auto it = kv.begin(); it != kv.end(); ++it)
    {
        ret.push_back(it->first);
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}


void MtcState::webService()
{
    try
    {
        std::string storagePath = kv[STORAGE_PATH];

        int port = boost::lexical_cast<int>(kv[STATE_PORT]);
        WPP::Server server;
        server.get("/api", &xmlResponce);
        server.get("/", &webResponse);
        server.all("/data", storagePath);
        server.start(port);

    }
    catch(WPP::Exception e)
    {
        std::cerr << "State: " << e.what() << std::endl;
    }
}
