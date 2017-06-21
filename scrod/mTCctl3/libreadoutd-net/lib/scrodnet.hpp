#ifndef SCROD_NET_HPP
#define SCROD_NET_HPP

#include <string>
#include <vector>
#include <stdexcept>
#include <stdint.h>
#include <mtc/data/util.hpp>


namespace mtc
{
namespace net2
{
    struct CardInfo
    {
        ///Card ID. floor(ScrodId/4);
        uint8_t id;
        /// 4 bit channel status. 1 bit per channel.
        uint8_t chan;
        ///IO statistics.
        uint32_t ioIn;
        uint32_t ioOut;
    };

    /**
     * @brief The ScrodNet class This class is used for scrod register access, and remote data acquisition.
     * std::runtime_error gets thrown around everywhere. try/catch it.
     */
    class ScrodNet
    {
    public:

        /**
         * @brief ScrodNet Default constructor.
         * @param port  Port readoutd2 is accepting connections on.
         * @param ip IP address of the readoutd2.
         */
        ScrodNet(int port, std::string ip) throw(std::runtime_error);
        ~ScrodNet();

        /**
         * @brief getCardInfo Returns info on all configured scrods.
         * @return vector of card information object with potentially 4 scrods per card.
         */
        std::vector<CardInfo> getCardInfo();

        /**
         * @brief getReg Get scrod register.
         * @param card  Card ID (ScrodId/4).
         * @param channel Card channel ScrodId%4
         * @param address Scrod register map address.
         * @return Register value.
         */
        uint16_t getReg(uint16_t card, uint16_t channel, uint16_t address) throw (std::runtime_error);

        /**
         * @brief setReg Sets SCROD register.
         * @param card  Card ID (ScrodId/4).
         * @param channel Card channel ScrodId%4.
         * @param address Scrod register map address.
         * @param data Scrod register value.
         */
        void setReg(uint8_t card, uint8_t chan, uint16_t address, uint16_t data) throw (std::runtime_error);

        /**
         * @brief setTriggerMask Sets trigger enable for the 12 channels.
         * @param mask 12 bit value. 1 bit per channel.
         */
        void setTriggerMask(uint16_t mask);
        uint16_t getTriggerMask();

        /**
         * @brief setTriggerMask Sets l2 trigger.
         * @param mask Number of Scrods required for trigger. 1-15.
         */
        uint8_t setTriggerMin(uint8_t min);
        uint8_t getTriggerMin();

        /**
         * @brief getTriggerCount get the trigger count.
         * @return 8 bit trigger count.
         */
        uint getTriggerCount();

        /**
         * @brief softTrigger Soft triggers the scrods.
         */
        void softTrigger();

        ///Enable flow control veto.
        void enableVeto(bool on);
        ///Check if the veto is enabled.
        bool getVetoEnabled();
        ///Get veto status.
        bool getVeto();
        ///Clear veto.
        void clearVeto();

        ///Set the fine calibration signal delay.
        void setDelay(uint16_t delay);
        ///Get fine calibration system delay.
        uint16_t getDelay();
        ///Enable calibration mode.
        void enableCal(bool on);
        ///Get calibration mode status.
        bool getCal();

        ///Delay between the calibration pulse and trigger.
        void setTriggerDelay(uint8_t delay);
        uint8_t getTriggerDelay();

        ///Set the coarse delay for the calibration signal.
        void setCoarseDelay(uint8_t delay);
        ///Get coarse delay between the calibration signal and trigger.
        uint8_t getCoarseDelay();

        ///Rotate the local storage file in readoutd.
        void rotateStorage();

        // New trigger accessors
        uint8_t getEnable();
        void setEnable(uint8_t enable);
        uint16_t getMinA();
        void setMinA(uint16_t val);
        uint16_t getMaxA();
        void setMaxA(uint16_t val);
        uint16_t getMinB();
        void setMinB(uint16_t val);
        uint16_t getMaxB();
        void setMaxB(uint16_t val);
        uint16_t getMinDelayAB();
        void setMinDelayAB(uint16_t val);
        uint16_t getMaxDelayAB();
        void setMaxDelayAB(uint16_t val);
        uint16_t getMinC();
        void setMinC(uint16_t val);
        uint16_t getMaxC();
        void setMaxC(uint16_t val);
        uint8_t getPrescaleA();
        void setPrescaleA(uint8_t val);
        uint8_t getPrescaleB();
        void setPrescaleB(uint8_t val);
        uint8_t getPrescaleC();
        void setPrescaleC(uint8_t val);
        uint8_t getPrescaleAB();
        void setPrescaleAB(uint8_t val);
        uint16_t getTrgLinkStatus();
        uint8_t getLiveTimeFraction();
        uint16_t getTriggerARate();
        uint16_t getTriggerBRate();
        uint16_t getTriggerCRate();
        uint16_t getTriggerABRate();

        uint32_t advanceMotor(int steps);

        /**
         * @brief upgrade Upgrade to data connection. Once this is called this object can only be used for data acquisition.
         * @param channels vector of channels you would like to recieve. If it's empty all data will be transmited.
         */
        void upgrade(std::vector<mtc::data::GlobalChannel> channels = std::vector<mtc::data::GlobalChannel>()) throw (std::runtime_error);


        /**
         * @brief readPacket read a data packet.
         * @param usecTimeout -1 timeout means block till data. 0 timeout return right away. otherwise timeout in us.
         * @return pointer to the data packer. You MUST deal with this memory allocation yourself. IE you need to delete[] it once your done.
         */
        uint32_t* readPacket(uint32_t usecTimeout = -1) throw (std::runtime_error);

        /**
         * @brief isSocketReady check if data is available.
         * @return True on success. False on failier.
         */
        bool isSocketReady();

        /**
         * @brief getSocket Returns the data socket for you to play with. Connection must be upgraded.
         * @return data socket.
         */
        int getSocket();

        ///Last error for the register transaction, as defined by SCORD communication protocol API. 0 means timeout.
        int lastError;
    private:
        int udpSock_;
        int sock_;
        int port_;
        std::string ip_;
        bool upgraded_;
    };
}
}

#endif
