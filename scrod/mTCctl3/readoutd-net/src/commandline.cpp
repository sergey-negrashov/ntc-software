#include "../lib/commandline.hpp"
#include "../lib/storage.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>

using namespace std;
using mtc::net2::CardInfo;
using mtc::net2::ScrodNet;

namespace Color {
enum Code {
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_BLUE     = 34,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_DEFAULT  = 49
};
class Modifier {
    Code code;
public:
    Modifier(Code pCode) : code(pCode) {}
    friend std::ostream&
    operator<<(std::ostream& os, const Modifier& mod) {
        return os << "\033[" << mod.code << "m";
    }
};
}

CommandLine::CommandLine(std::string ip)
{
    net_ = boost::shared_ptr<mtc::net2::ScrodNet>(new mtc::net2::ScrodNet(10000, ip));

    chan_ = 0;
    ready_ = false;
    base_ = HEX;
    ip_ = ip;
#ifdef READOUT_DEBUG
    ready_ = true;
#endif
}


void CommandLine::mainLoop()
{
    char* input = NULL;

    for(;;)
    {
        if(input != NULL)
        {
            free(input);
            input = NULL;
        }
        input = readline("cpci>");

        if (!input)
            continue;

        string in(input);
        boost::algorithm::trim(in);
        if(in.length() == 0 || in.at(0) == '#')
            continue;
        add_history(input);
        if(processCommand(in))
        {
            Color::Modifier green(Color::FG_GREEN);
            Color::Modifier def(Color::FG_DEFAULT);
            cout << green << "[OK]" << def << endl;
        }
    }

}

bool CommandLine::processCommand(std::string in)
{
    Color::Modifier green(Color::FG_GREEN);
    Color::Modifier red(Color::FG_RED);
    Color::Modifier def(Color::FG_DEFAULT);
    vector <string> args;
    char* end;
    boost::algorithm::split(args, in, boost::algorithm::is_any_of(" \t"), boost::algorithm::token_compress_on);
    if(args.size()  == 0)
        return false;
    in = args[0];
    if(in == "quit")
    {
        exit(0);
    }
    if(in == "list")
    {
        std::vector<mtc::net2::CardInfo> info = net_->getCardInfo();
        for(int i = 0; i< info.size(); i++)
        {
            cout << "ID: " << (uint)info[i].id << endl;
            cout << "\tChannel Status:" << bitset<4>(info[i].chan) << endl;
            cout << "\tRead: " << (uint)info[i].ioOut << " Bytes" << endl;
            cout << "\tWritten: " << (uint)info[i].ioIn << " Bytes" << endl;

        }
        return true;
    }
    if(in == "card")
    {
        if(args.size() < 2)
        {
            cout << red << "Card command requires a numerical argument " << def << endl;
            return false;
        }

        card_ = strtoul(args[1].c_str(), &end, 0);
        if (*end)
        {
            cout << red << "Card command requires a numerical argument " << def << endl;
            return false;
        }
        std::vector<mtc::net2::CardInfo> info = net_->getCardInfo();
        for(size_t i = 0 ; i < info.size(); i++)
        {
            if(info.at(i).id == card_)
            {
                ready_ = true;
                return true;
            }
        }
        cout << red << "No card with id " << card_ << def << endl;
        return false;
    }
    else if(in == "channel")
    {
        if(!ready_)
        {
            cout << red << "Select card first with a card command" << def << endl;
            return false;
        }
        if(args.size() <2)
        {
            cout << red << "Channel command requires a numerical argument " << def << endl;
            return false;
        }
        chan_ = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Channel command requires a numerical argument " << def << endl;
            chan_ = 0;
            return false;
        }
        if(chan_ > 3)
        {
            cout << red << "Channel command requires an argument between 0 and 3 " << def << endl;
            chan_  = 0;
            return false;
        }
        return true;
    }
    else if(in == "set")
    {
        int reg;
        int data;
        if(!ready_)
        {
            cout << red << "Select card first with a card command" << def << endl;
            return false;
        }
        if(args.size() < 3)
        {
            cout << red << "Set command requires two numerical arguments: register and data" << def << endl;
            return false;
        }
        reg = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Set command requires two numerical arguments: register and data" << def << endl;
            return false;
        }
        data = strtoul(args[2].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Set command requires two numerical arguments: register and data" << def << endl;
            return false;
        }
        try
        {
            net_->setReg(card_, chan_, reg, data);
        }
        catch(std::runtime_error &e)
        {
            cout << red << e.what() << def << endl;
            return false;
        }
        return true;
    }
    else if(in == "get")
    {
        int reg;
        if(!ready_)
        {
            cout << red << "Select card first with a card command" << def << endl;

            return false;
        }
        if(args.size() <2)
        {
            cout << red << "Get command requires a numerical argument (register) " << def << endl;
            return false;
        }
        reg = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout << red <<  "Get command requires a numerical argument (register) " << def << endl;
            return false;
        }

        uint16_t ret;
        try
        {
            ret = net_->getReg(card_, chan_, reg);
        }
        catch(std::runtime_error &e)
        {
            cout << red << e.what() << def << endl;
            return false;
        }

        switch(base_)
        {
        case DEC:
            cout << dec;
            cout << ret << dec<< endl;
            break;
        case HEX:
            cout << hex;
            cout << "0x"<< ret << dec<< endl;
            break;
        case BIN:
            cout << "0b"<< bitset<16>(ret) << endl;
        }
        return true;
    }
    else if(in == "poll")
    {
        int reg;
        int interval;
        int num;
        if(!ready_)
        {
            cout << red << "Select card first with a card command" << def << endl;
            return false;
        }
        if(args.size() < 4)
        {
            cout << red << "Poll command requires three numerical arguments: register, interval(ms), number-of-polls " << def << endl;
            return false;
        }
        reg = strtoul(args[1].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Poll command requires three numerical arguments: register, interval(ms), number-of-polls " << def << endl;
            return false;
        }
        interval = strtoul(args[2].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Poll command requires three numerical arguments: register, interval(ms), number-of-polls " << def << endl;
            return false;
        }
        num = strtoul(args[3].c_str(), &end, 0);
        if(*end)
        {
            cout << red << "Poll command requires three numerical arguments: register, interval(ms), number-of-polls " << def << endl;
            return false;
        }
        for(int i = 0; i< num; i++)
        {
            uint16_t ret;
            try
            {
                ret = net_->getReg(card_, chan_, reg);
            }
            catch(std::runtime_error &e)
            {
                cout << red << e.what() << def << endl;
                return false;
            }

            switch(base_)
            {
            case DEC:
                cout << dec;
                cout << ret << dec<< endl;
                break;
            case HEX:
                cout << hex;
                cout << "0x"<< ret << dec<< endl;
                break;
            case BIN:
                cout << "0b"<< bitset<16>(ret) << endl;
            }
            boost::this_thread::sleep(boost::posix_time::milliseconds(interval));
        }
        return true;
    }
    else if(in == "open")
    {
        if(args.size() <2)
        {
            cout << red <<  "Open requires a filename." << def << endl;
            return false;
        }

        string filename = args[1];

        if(filename == "now")
        {
            time_t rawtime;
            struct tm * timeinfo;
            char timebuffer [160];
            timebuffer[0] = '\0';
            time ( &rawtime );
            timeinfo = localtime ( &rawtime );
            strftime (timebuffer,80,"readout_%Y-%m-%d-%H:%M.%S.dat",timeinfo);
            filename = timebuffer;
        }

        if(boost::filesystem::exists( filename ))
        {
            cout << red <<  "Path taken. Pick a new File" << def << endl;
            return false;
        }
        Storage s(ip_);
        if(s.startRawStorageTo(filename))
            return true;
        cout << red <<  "Could not open file for storage. Pick a new Fifd_le" << def << endl;
        return false;
    }
    else if(in == "trg")
    {
        if(args.size() < 2)
        {
            uint16_t ret;
            try
            {
                ret = net_->getTriggerMin();
            }
            catch(std::runtime_error &e)
            {
                cout << red << e.what() << def << endl;
                return false;
            }
            cout << "Trg Min: " << green << ret  << def << endl;
        }
        else
        {
            int newMin;
            int oldMin;
            newMin = strtoul(args[1].c_str(), &end, 0);
            if(*end)
            {
                cout << red <<  "Trg command requires a numerical argument (minScrods) " << def << endl;
                return false;
            }
            try
            {
                oldMin = net_->setTriggerMin(newMin);
            }
            catch(std::runtime_error &e)
            {
                cout << red << e.what() << def << endl;
                return false;
            }
            cout << "Old Trg Min: " << green << oldMin << def << " New Trg Min: " << green << newMin  << def << endl;
        }
        return true;
    }
    else if(in == "delay")
    {
        if(args.size() < 2)
        {
            uint16_t ret;
            try
            {
                ret = net_->getDelay();
            }
            catch(std::runtime_error &e)
            {
                cout << red << e.what() << def << endl;
                return false;
            }
            cout << "Delay Min: " << green << ret  << def << endl;
        }
        else
        {
            int newDelay;
            int setDelay;
            newDelay = strtoul(args[1].c_str(), &end, 0);
            if(*end)
            {
                cout << red <<  "delay command requires a numerical argument (minScrods) " << def << endl;
                return false;
            }
            try
            {
                net_->setDelay(newDelay);
                setDelay = net_->getDelay();
            }
            catch(std::runtime_error &e)
            {
                cout << red << e.what() << def << endl;
                return false;
            }
            cout << "Set Delay: " << green << setDelay << def << ". Requested Delay " << green << newDelay  << def << endl;
        }
        return true;
    }

    else if(in == "run")
    {
        if(args.size() < 2)
        {
            cout << red <<  "Run requires a filename." << def << endl;
            return false;
        }

        string filename = args[1];
        if(!boost::filesystem::exists( filename ))
        {
            cout << red <<  "File does not exist. Pick a new File" << def << endl;
            return false;
        }

        string sLine = "";
        ifstream infile;
        infile.open(filename.c_str());
        if(!infile.is_open())
        {
            cout << red <<  "Could not open command file " << filename << def << endl;
            return false;
        }

        int i = 0;
        while (getline(infile, sLine))
        {
            i++;
            boost::algorithm::trim(sLine);
            cout << i << ": " << sLine << "\t\t";
            if(sLine.length() == 0 || sLine[0] == '#')
            {
                cout << endl;
                continue;
            }
            if(!processCommand(sLine))
            {
                cout << "Error in script line " << i << ". Continue(Y/N)";
                char ok;
                cin >> ok;
                if(ok == 'Y' || ok == 'y')
                    continue;
                else
                    return false;
                cout << endl;
            }
            else
                cout << green << "[OK]" << def << endl;
        }
        infile.close();
        return true;
    }
    else if(in == "sleep")
    {
        int ammount;
        if(args.size() < 2)
        {
            ammount = 1;
        }
        else
        {
            ammount = strtoul(args[1].c_str(), &end, 0);
            if(*end)
            {
                cout << red <<  "Sleep command requires an numeric argument(milliseconds)" << def << endl;
                return false;
            }
        }
        boost::this_thread::sleep(boost::posix_time::milliseconds(ammount));
        return true;
    }
    else if(in == "hex")
    {
        base_ = HEX;
        return true;
    }
    else if(in == "dec")
    {
        base_ = DEC;
        return true;
    }
    else if(in== "bin")
    {
        base_ = BIN;
        return true;
    }
    else if(in == "help")
    {
        cout << "list : Lists cards" << endl;
        cout << "card : Selects card" << endl;
        cout << "channel : Selects channel" << endl;
        cout << "set : sets a register" << endl;
        cout << "get : gets a register" << endl;
        cout << "poll : polls a register at a specified interval" << endl;
        cout << "help : this message" << endl;
        cout << "run : execute commands from file" << endl;
        cout << "sleep : sleep for a specified numbed of milliseconds" << endl;
        cout << "net : start to start network control, info for number of connections" << endl;
        cout << "dec : print get values in decimal" << endl;
        cout << "hex : print get values in hex" << endl;
        cout << "bin : print get values in binary" << endl;
        return true;
    }

    else
    {
        cout << red << "Unknown command " << in << def<< endl;
        return false;
    }
}
