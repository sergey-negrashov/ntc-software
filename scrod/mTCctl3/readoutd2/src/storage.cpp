#include "../lib/storage.hpp"
#include "../lib/settings.hpp"
#include <boost/filesystem.hpp>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <boost/array.hpp>
#include <sys/uio.h>
#include <boost/foreach.hpp>
#include "../lib/packets.hpp"
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string>
#include <time.h>
#include <vector>
#include <sys/select.h>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"

using std::cout;
using std::endl;
using std::string;
using std::vector;

std::vector<int> Storage::getRunAndExperimentNumbers() {
    vector<int> runStuff; //stores experiment number, run number, returns them to main function
    fstream runNumberFile;
    runNumberFile.open(
            "/usr/local/etc/mtc/experimentAndRunNumbers.txt");              //file, in this folder, where this info

    if (!runNumberFile) {
        cout << "ERROR: Could not open experimentAndRunNumbers.txt" << endl;
        exit(0);
    }


    if (runNumberFile) {
        string word;
        while (runNumberFile >> word) {
            const char firstChar = word.at(0);
            char ignChar = '#';
            if (firstChar == ignChar)    //skip the word if it's a header word, tagged by #
            {
                continue;
            } else {
                int value = atoi(word.c_str()); //convert numbers from string to int
                runStuff.push_back(value);
            }

        }

        //runNumberFile << "Writing this to a file.\n";
        int incrementedRunNum = runStuff[1] + 1;
        runStuff[1] = incrementedRunNum;    //increment run number by 1 for this new run. Experiment left unchanged unless altered by user.
        runNumberFile.close();

        ofstream runFile;            //Reopen file to overwrite with new run number
        runFile.open("/usr/local/etc/mtc/experimentAndRunNumbers.txt",
                     std::fstream::trunc);    //file stored in same place as settings.set

        runFile << "#Exp #Run" << endl << runStuff[0] << " " << runStuff[1];
        runFile.close();        //close file when done

        return runStuff;
    } else {
        cout << "could not open file.";
    }

}

static uint64_t makeTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (uint64_t) 1000000 + tv.tv_usec;
}

std::string makeDate(void) {
    time_t now;
    char dateString[25];
    dateString[0] = '\0';
    now = time(NULL);
    strftime(dateString, 25, "%Y_%m_%d",
             localtime(&now));            //changed to use abbreviated month name for clarity when reading filename
    return std::string(
            dateString);                                    //day left first for now for easier sorting of folders
}

std::string makeTime(void) {
    time_t now;
    char dateString[25];
    dateString[0] = '\0';
    now = time(NULL);
    strftime(dateString, 25, "%H_%M_%S", localtime(&now));
    return std::string(dateString);
}

Storage::Storage(StorageQueuePtr q) {
    oFile = -1;
    timeFile = -1;
    rotateStorageFile();
    lastEvent = 0;
    this->q = q;
    numClients = 0;
}

void Storage::renameStorageFile(string prefix) {
    storageFilenamePrefix = prefix;
}

void Storage::rotateStorageFile() {
    using namespace boost::filesystem;
    boost::mutex::scoped_lock scopedLock(lock_);

    MtcState *state = MtcState::Instance();
    if (oFile != -1)
        ::close(oFile);
    if (timeFile != -1)
        ::close(timeFile);
    string rootStorageDirString = state->getValue(STORAGE_PATH);
    boost::filesystem::path storageRootDir(rootStorageDirString);

    if (exists(storageRootDir)) {
        if (!is_directory(storageRootDir))
            throw std::runtime_error("Storage directory missconfigured");
    } else {
        if (!create_directory(storageRootDir))
            throw std::runtime_error("Storage directory missconfigured");
    }


    string storageDirString = rootStorageDirString + makeDate();
    boost::filesystem::path storageDir(storageDirString);
    if (exists(storageDir)) {
        if (!is_directory(storageDir))
            throw std::runtime_error("Storage directory missconfigured");
    } else {
        if (!create_directory(storageDir))
            throw std::runtime_error("Could not create storage directory " + storageDir.string());
    }

    vector<int> runNumbers = Storage::getRunAndExperimentNumbers();        //gets experiment and run number from file
    //std::string experimentNum = std::to_string(runNumbers[0]);
    //std::string runNum = std::to_string(runNumbers[1]);

    std::stringstream expNum;    //preparing experiment number string with leading 0's
    expNum << setw(4) << setfill('0') << runNumbers[0];
    string experimentNum = expNum.str();
    std::stringstream rnNum;    //preparing run number string with leading 0's
    rnNum << setw(4) << setfill('0') << runNumbers[1];
    string runNum = rnNum.str();
    std::string file_name_string = "exp_" + experimentNum + "_run_" + runNum;
    //storageFilenamePrefix = "data";
    //storageFileString = storageDirString + "/" + makeDate() + "_" + "atTime" + "_" + makeTime() + "_" + storageFilenamePrefix + ".dat";
    //storageTimeString = storageDirString + "/" + makeDate() + "_" + "atTime" + "_" + makeTime() + "_" + storageFilenamePrefix + ".tme";


    storageFileString = storageDirString + "/" + file_name_string + ".dat";
    storageTimeString = storageDirString + "/" + file_name_string + ".tme";

    oFile = ::open(storageFileString.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
    timeFile = ::open(storageTimeString.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
    lastEvent = -1;
}

void Storage::mainLoop() {
    try {
        while (true) {
            bool ok = true;
            //Grab the next event
            uint32_t *packet = q->pop_timeout(1000, ok);

            //check the bail flag
            if (ok == false || packet == NULL) {
                //good place for housekeeping.
                boost::this_thread::interruption_point();
                continue;
            }

            boost::mutex::scoped_lock scopedLock(lock_);

            //Save the event timestamp;
            if (packet[2] == EVENT_HEADER_WORD) {
                int64_t eventNumber = packet[5];
                if (eventNumber > lastEvent) {
                    lastEvent = eventNumber;
                    if (timeFile > 0) {
                        uint64_t timeStamp = makeTimeStamp();
                        string eventText = boost::lexical_cast<std::string>(eventNumber) + " : " +
                                           boost::lexical_cast<std::string>(timeStamp) + "\n";
                        ::write(timeFile, eventText.c_str(), eventText.length());
                    }
                }
            }

            //Write packet localy
            int size = packet[1] + 2;
            ::write(oFile, packet, size * sizeof(uint32_t));
            //Send waveform packet over the network to everyone who requested it

            fd_set writable;
            timeval timeout;
            for (auto it = clients.begin(); it != clients.end(); ++it) {
                int fd = it->fd;
                if (packet[2] == EVENT_WORD) {
                    //Do we need to send this packet to this client?
                    if (it->tokens.size() != 0) {
                        uint scrodId = packet[3] >> 16;
                        uint chanId = packet[7] >> 9;
                        uint chan = chanId & 0x7;
                        uint row = (chanId >> 3) & 0x3;
                        uint col = (chanId >> 5) & 0x3;

                        if (it->tokens.find(scrodId) == it->tokens.end()) {
                            continue;
                        }

                        uint32_t r = it->tokens.at(scrodId)[row];
                        uint8_t chip = 0xFF & (r >> (col * 8));
                        if (!(chip & (1 << chan))) {
                            continue;
                        }
                    }
                }
                FD_ZERO(&writable);
                FD_SET(fd, &writable);
                timeout.tv_sec = 0;
                timeout.tv_usec = 1000;
                if (select(fd + 1, NULL, &writable, NULL, &timeout) > 0) {
                    ::write(fd, packet, size * sizeof(uint32_t));
                }
            }
            delete[] packet;
        }
    }
    catch (boost::thread_interrupted &e) {
        //We are done here.
    }
    cout << "Storage: Done" << endl;
}


void Storage::addClent(int fd, std::map<uint16_t, boost::array<uint32_t, 4> > tokens) {

    boost::mutex::scoped_lock scopedLock(lock_);
    Client c;
    c.fd = fd;
    c.tokens = tokens;
    clients.push_back(c);
    numClients++;
}

bool Storage::removeClient(int fd) {
    boost::mutex::scoped_lock scopedLock(lock_);
    for (auto it = clients.begin(); it != clients.end(); it++) {
        if (it->fd == fd) {
            it = clients.erase(it);
            numClients--;
            return true;
        }
    }
    return false;
}

void Storage::start() {
    if (thr.joinable()) {
        throw std::runtime_error("Storage thread: attempting to start but already running");
    }
    thr = boost::thread(&Storage::mainLoop, this);
}

void Storage::stop() {
    thr.interrupt();
    thr.join();
}
