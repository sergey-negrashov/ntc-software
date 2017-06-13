#include "../lib/storage.hpp"
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <newt.h>
#include <mtc/data/datapacket.hpp>
#include <mtc/data/util.hpp>
#include <mtc/readoutd2/scrodnet.hpp>

Storage::Storage(std::string ip)
{
    ip_ = ip;
}

void Storage::drawStatic()
{
    newtCls();
    int uiRows, uiCols;
    newtPushHelpLine("Help Line");

    /* determine current terminal window size */

    uiRows = uiCols = 0;
    newtGetScreenSize(&uiCols, &uiRows);

    /* draw standard help and string on root window */
    newtPushHelpLine(NULL);

    newtDrawRootText((uiCols - strlen("Storing data"))/2 , 0, "Storing data");

    newtDrawRootText(0,1, "Storage type");
    newtDrawRootText(15,1, ":");
    newtDrawRootText(20,1, storageType_.c_str());

    newtDrawRootText(0,2, "Storage path");
    newtDrawRootText(15,2, ":");
    newtDrawRootText(20,2, path_.c_str());

    newtDrawRootText(0,3, "Crate IP");
    newtDrawRootText(15,3, ":");
    newtDrawRootText(20,3, ip_.c_str());



    newtDrawRootText(0,4, "Packets");
    newtDrawRootText(15,4, ":");

    newtDrawRootText(0,5, "Events");
    newtDrawRootText(15,5, ":");

    newtDrawRootText(0,6, "Event #");
    newtDrawRootText(15,6, ":");

    newtDrawRootText(0,7, "Errors #");
    newtDrawRootText(15,7, ":");

    newtPushHelpLine("Press any key to stop");
    /* cleanup after getting a keystroke */


}

void Storage::rawStorageWait()
{
    newtInit();
    drawStatic();
    boost::thread *t = new boost::thread(&Storage::rawStorageUpdate, this);
    try
    {
        newtWaitForKey();
    }
    catch( boost::thread_interrupted &e)
    {
    }
    done_ = true;
    t->join();
}

void Storage::rawStorageUpdate()
{
    try
    {
        while(!done_)
        {
            std::string temp = boost::lexical_cast<std::string>(packets_);
            newtDrawRootText(20,4, temp.c_str());

            temp = boost::lexical_cast<std::string>(events_);
            newtDrawRootText(20,5, temp.c_str());

            temp = boost::lexical_cast<std::string>(lastEvent_);
            newtDrawRootText(20,6, temp.c_str());

            temp = boost::lexical_cast<std::string>(checksumErrors_);
            newtDrawRootText(20,7, temp.c_str());

            newtRefresh();
            boost::this_thread::sleep(boost::posix_time::millisec(500));
        }
    }
    catch( boost::thread_interrupted &e)
    {
    }
    newtFinished();
}

bool isSocketReady(int sock)
{
    bool             res;
    fd_set          sready;
    struct timeval  now;

    FD_ZERO(&sready);
    FD_SET((unsigned int)sock,&sready);
    now.tv_sec = 0;
    now.tv_usec = 0;
    res = select(sock+1,&sready,NULL,NULL,&now);
    if( FD_ISSET(sock,&sready) )
        return true;
    return false;
}

using namespace std;

bool Storage::startRawStorageTo(std::string path)
{

    path_= path;
    storageType_ = "Binary";
    done_ = false;
    events_ = 0;
    packets_ = 0;
    lastEvent_ = 0;
    checksumErrors_ = 0;

    int fd = ::open(path.c_str(), O_WRONLY | O_CREAT, 0644);
    if( fd < 0 )
    {
        return -1;
    }
    mtc::net2::ScrodNet net(10000, ip_);
    net.upgrade();
    int sock = net.getSocket();
    boost::thread *t = new boost::thread(&Storage::rawStorageWait, this);

    while(!done_)
    {
        try
        {
            uint32_t* data  = net.readPacket(2000000);
            if(data == NULL)
                continue;
            mtc::data::DataPacket p(data,false);
            delete[] data;
            if(p.type == p.EVENT_WORD)
            {
                int event = mtc::data::windowEventNumber(p);
                if(event > lastEvent_)
                {
                    lastEvent_ = event;
                    events_++;
                }
            }
            p.toFile(fd);
            packets_++;

        }
        catch(std::runtime_error &e)
        {
            checksumErrors_++;
        }
    }
    t->join();
    return 1;
}
