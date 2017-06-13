#include "lib/settings.hpp"
#include "lib/common.hpp"
#include "lib/udplistner.hpp"
#include "lib/registerinterface.hpp"
#include "lib/trigger.hpp"
#include "lib/monitor.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <stdio.h>
#include <bitset>
#include "lib/common.hpp"
#include <iostream>
#include <newt.h>

using namespace std;


std::vector <UdpDataListner*> listners;
Storage *s;

std::vector<int> activeScrods;
std::vector <int> lastPackets;
std::vector <float> lastMB;
StorageQueuePtr storeQ;
int uiRows, uiCols, oldUiRows, oldUiCols;
bool bailOut;

static void handler(int s)
{
    bailOut = true;
}

void updateGui()
{
    MtcState * state = MtcState::Instance();
    Trigger* t = Trigger::Instance();


    /* determine current terminal window size */
    newtGetScreenSize(&uiCols, &uiRows);
    // newtCls();
    string blanks;
    for(int i = 0; i< uiCols; i++)
    {
        blanks += " ";
    }
    for(int i = 0; i< listners.size() + 8; i++)
    {
        newtDrawRootText(0,i, blanks.c_str());
    }
        newtDrawRootText((uiCols - strlen("Interfaces"))/2 , 0, "Interfaces");
        newtDrawRootText(0,1, "If");
        newtDrawRootText(8,1, "Name");
        newtDrawRootText(20,1, "P/s");
        newtDrawRootText(32,1, "MB/s");
        newtDrawRootText(44,1, "MB");
        newtDrawRootText(56,1, "Packets");
        newtDrawRootText(68,1, "Errors");
        newtDrawRootText((uiCols - strlen("Triggering"))/2 , listners.size() + 2, "Triggering");
        newtDrawRootText(0,listners.size() + 3, "L2:");
        newtDrawRootText(0,listners.size() + 4, "Mask:");
        newtDrawRootText(0,listners.size() + 5, "Trg #:");
        newtDrawRootText(0,listners.size() + 6, "Veto On:");

        newtDrawRootText(44,listners.size() + 3, "F Delay:");
        newtDrawRootText(44,listners.size() + 4, "C Delay");
        newtDrawRootText(44,listners.size() + 5, "Cal Mode: ");
        newtDrawRootText(44,listners.size() + 6, "Veto Needed:");

        newtDrawRootText((uiCols - strlen("Network"))/2 , listners.size() + 7, "Network");
        newtDrawRootText(0 , listners.size() + 8, "Storage:");
        newtDrawRootText(44, listners.size() + 8, "Queue depth:");
        newtDrawRootText((uiCols - strlen("Storage"))/2 , listners.size() + 9, "Storage");
        newtDrawRootText(0, listners.size() + 10, "Storage Path: ");
        oldUiRows = uiRows;
        oldUiCols = uiCols;

    for(int i = 0; i< listners.size(); i++)
    {
        UdpDataListner* l = listners[i];
        string delim = l->delim;
        string pt = state->getValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_PACKETS);
        string bt = state->getValue(LISTNER_PREFACE + delim + LISTNER_TOTAL_BYTES);
        string bp = state->getValue(LISTNER_PREFACE + delim + LISTNER_BAD_PACKETS);

        int packets = boost::lexical_cast<int>(pt);
        int bytes = boost::lexical_cast<float>(bt);
        int prate = packets - lastPackets[i];
        float brate = bytes - lastMB[i];
        lastPackets[i] = packets;
        lastMB[i] = bytes;
        newtDrawRootText(0,i + 2, l->delim.c_str());
        newtDrawRootText(8,i + 2, l->interName.c_str());
        newtDrawRootText(20,i + 2, boost::lexical_cast<string>(prate).c_str());
        newtDrawRootText(32,i + 2, boost::lexical_cast<string>(brate).c_str());;
        newtDrawRootText(44,i + 2, bt.c_str());
        newtDrawRootText(56,i + 2, pt.c_str());
        newtDrawRootText(68,i + 2, bp.c_str());
    }

    newtDrawRootText(12,listners.size() + 3, boost::lexical_cast<string>((int)t->getTriggerMin()).c_str());
    std::bitset<12> trgMask = std::bitset<12>(t->getTriggerMask());
    newtDrawRootText(12,listners.size() + 4, trgMask.to_string().c_str());
    newtDrawRootText(12,listners.size() + 5, "   ");
    newtDrawRootText(12,listners.size() + 5, boost::lexical_cast<string>((int)t->getTriggerCount()).c_str());
    if(t->getVetoEnabled())
        newtDrawRootText(12,listners.size() + 6, "ON ");
    else
        newtDrawRootText(12,listners.size() + 6, "OFF");

    newtDrawRootText(58,listners.size() + 3, boost::lexical_cast<string>((int)t->getDelay()).c_str() );
    newtDrawRootText(58,listners.size() + 4, boost::lexical_cast<string>((int)t->getCoarseDelay()).c_str() );
    if(t->getCal())
        newtDrawRootText(58,listners.size() + 5, "YES");
    else
        newtDrawRootText(58,listners.size() + 5, "NO ");

    if(t->getVeto())
        newtDrawRootText(58,listners.size() + 6, "YES");
    else
        newtDrawRootText(58,listners.size() + 6, "NO ");



    newtDrawRootText(12,listners.size() + 8, boost::lexical_cast<string>(s->numClients).c_str());
    newtDrawRootText(58, listners.size() + 8, "            ");
    newtDrawRootText(58, listners.size() + 8, boost::lexical_cast<string>((storeQ->size())).c_str());
    newtDrawRootText(14, listners.size() + 10, s->storageFileString.c_str());
    newtResizeScreen(1);
}

void guiMainLoop()
{
    try
    {
        uiRows = uiCols = oldUiCols = oldUiRows = 0;
        newtInit();
        newtCls();
        newtPushHelpLine("");

        while(true)
        {
            boost::this_thread::sleep(boost::posix_time::seconds(1));
            updateGui();
        }
    }
    catch(boost::thread_interrupted &e)
    {
        //We have been interupted from the main thread;
        newtFinished();
    }
}



int main(int argc, char** argv)
{
    std::string pathToConfigFile = PATH_TO_CONFIGURATION_FILE;
    if(argc == 2)
        pathToConfigFile = argv[1];

    bailOut = false;
    signal(SIGPIPE, SIG_IGN);
    signal(SIGTERM, handler);

    MtcState::initialize(pathToConfigFile);
    MtcHealthMonitor* monitor = MtcHealthMonitor::Instance();
    MtcState * state = MtcState::Instance();
    std::string scrodString = state->getValue(ACTIVE_SCRODS);

    std::string storeQSizeString = state->getValue(STORAGE_QUEUE_SIZE);
    int storeQSize = boost::lexical_cast<int>(storeQSizeString);

    storeQ = StorageQueuePtr(new StorageQueue(storeQSize));

    std::vector<std::string> scrodStrings;
    boost::split(scrodStrings, scrodString, boost::is_any_of(","));

    for(auto i = scrodStrings.begin(); i != scrodStrings.end(); i++)
    {
        int iface = boost::lexical_cast<int>(boost::trim_copy(*i));
        activeScrods.push_back(iface);
        UdpDataListner *l = new UdpDataListner(iface, storeQ);
        l->start();
        listners.push_back(l);
        lastPackets.push_back(0);
        lastMB.push_back(0);
        cout << "Started readout thread " << iface << endl;
    }

    s = new Storage(storeQ);
    s->start();
    cout << "Started storage thread" << endl;

    RegisterInterface r(activeScrods, s);
    r.start();
    cout << "Started register thread" << endl;


    boost::thread guiThread(&guiMainLoop);

    while(bailOut == false)
    {
        pause();
        sleep(1);
    }

    guiThread.interrupt();
    guiThread.join();

    r.stop();
    for(auto i = listners.begin(); i != listners.end(); i++)
    {
        (*i)->stop();
        //delete (*i);
    }
    s->stop();
    delete s;
    return 0;
}
