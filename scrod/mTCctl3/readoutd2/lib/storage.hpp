#ifndef STORAGE_HPP
#define STORAGE_HPP
#include "common.hpp"
#include <boost/thread.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/array.hpp>
#include <vector>

class Storage
{
public:
    Storage(StorageQueuePtr q);
    void addClent(int fd, std::map < uint16_t, boost::array<uint32_t,4> > tokens);
    std::vector<int> getRunAndExperimentNumbers();
    void renameStorageFile(string prefix);
    void rotateStorageFile();
    bool removeClient(int fd);
    int getLastEventNumber(){return lastEvent;}
    void start();
    void stop();
    int numClients;
    std::string storageFileString;
    std::string storageTimeString;
    std::string storageFilenamePrefix;          //added to make filenames more human readable
private:
    StorageQueuePtr q;
    void mainLoop();

    std::string path_;
    int oFile;
    boost::thread thr;
    boost::mutex lock_;
    bool running_;

    //Event Meta Data
    int metaFile_;

    //Event Time File
    int timeFile;
    int64_t lastEvent;

    struct Client
    {
        int fd;
        std::map < uint16_t, boost::array<uint32_t,4> > tokens;
    };
    std::list < Client > clients;

    //Storage directory
    std::string storageDir;
/*
    //File rotation
    void fileRotationLoop();
    boost::thread* rotationThread_;
    int sequence_;
    int period_;
*/

};

#endif // STORAGE_HPP
