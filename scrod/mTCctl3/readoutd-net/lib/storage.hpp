#ifndef STORAGE_HPP
#define STORAGE_HPP
#include <mtc/readoutd2/scrodnet.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

class Storage
{
public:
    Storage(std::string ip);
    bool startRawStorageTo(std::string path);

private:
    void rawStorageWait();
    void rawStorageUpdate();
    void drawStatic();
    std::string ip_;
    std::string path_;
    std::string storageType_;
    bool done_;
    int events_;
    int packets_;
    int lastEvent_;
    int checksumErrors_;
};

#endif // STORAGE_HPP
