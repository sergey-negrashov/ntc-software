/********************************************************************\

  Name:         drs_exam.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a DRS4
                evaluation board

  $Id: drs_exam.cpp 21308 2014-04-11 14:50:16Z ritt $

\********************************************************************/

#include <math.h>
#include <utility>

#ifdef _MSC_VER

#include <windows.h>

#elif defined(OS_LINUX)

#define O_BINARY 0

#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>

#define DIR_SEPARATOR '/'

#endif

#include <stdio.h>

#include "DRS.h"
#include <iostream>
#include "motor.hpp"
#include <chrono>
#include <thread>
#include <newt.h>
#include <csignal>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>



using namespace std;

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

// trim from start (copying)
static inline std::string ltrimmed(std::string s) {
    ltrim(s);
    return s;
}

// trim from end (copying)
static inline std::string rtrimmed(std::string s) {
    rtrim(s);
    return s;
}

// trim from both ends (copying)
static inline std::string trimmed(std::string s) {
    trim(s);
    return s;
}


void split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

/*------------------------------------------------------------------*/

namespace
{
    volatile std::sig_atomic_t running;
}

void waiter()
{
    newtWaitForKey();
    running = 0;
}

int main(int argc, char **argv) {
    int stepsPerTurn = 4000;
    int randomSteps = 0;
    int eventsPerStep = 100;
    int pauseAfterStep = 0;
    ifstream settingsFile("run_config.cfg");
    string line;
    if (settingsFile.is_open())
    {
        cout << "Loading configuration from file" << endl;
        while ( getline (settingsFile,line) )
        {
            auto items = split(line,':');
            if(items.size() != 2)
                continue;
            trim(items[0]);
            if(items[0] == "Steps")
                stepsPerTurn = std::stoi(items[1]);
            if(items[0] == "Random")
                randomSteps = std::stoi(items[1]);
            if(items[0] == "Events")
                eventsPerStep = std::stoi(items[1]);
            if(items[0] == "Pause")
                pauseAfterStep = std::stoi(items[1]);

        }
        settingsFile.close();
    }

    cout << "Steps per Turn: " << stepsPerTurn << endl;
    cout << "Events per Turn " << eventsPerStep << endl;
    cout << "Random " << (randomSteps ? "No": "Yes") << endl;
    cout << "Pause after move " << pauseAfterStep << "ms" << endl;
    int actualEventNumber = 1;
    std::srand(std::time(0));
    int nBoards;
    DRS *drs;
    DRSBoard *master;
    DRSBoard *slave;
    //float time_array[4][1024];
    float wave_array[4][1024];
    FILE *f;

    /* do initial scan */
    drs = new DRS();

    /* show any found board(s) */
    for (int i = 0; i < drs->GetNumberOfBoards(); i++) {
        master = drs->GetBoard(i);
        printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n",
               master->GetBoardSerialNumber(), master->GetFirmwareVersion());
    }

    /* exit if no board found */
    nBoards = drs->GetNumberOfBoards();
    if (nBoards < 2) {
        printf("No DRS4 evaluation board found\n");
        //return 0;
    }

    /* continue working with first board only */
    master = drs->GetBoard(0);
    slave = drs->GetBoard(1);

    //Get board id
    if(master->GetBoardSerialNumber() != 2675)
        std::swap(master, slave);

    int boardid_master = master->GetBoardSerialNumber();
    int boardid_slave = slave->GetBoardSerialNumber();

    newtInit();

    /* initialize board */
    master->Init();
    /* set sampling frequency */
    master->SetFrequency(5, true);
    /* enable transparent mode needed for analog trigger */
    master->SetTranspMode(1);
    /* set input range to -0.5V ... +0.5V */
    master->SetInputRange(0);

    /* use following line to set range to 0..1V */
    /* use following line to turn on the internal 100 MHz clock connected to all channels  */
    master->EnableTcal(1);
    /* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */

    /* use following lines to enable the external trigger */
    if (master->GetBoardType() == 8) {     // Evaluaiton Board V4
        master->EnableTrigger(1, 0);           // enable hardware trigger
        master->SetTriggerSource(1<<4);        // set external trigger as source
    } else {                          // Evaluation Board V3
        master->EnableTrigger(1, 0);           // lemo on, analog trigger off
    }

    master->EnableTcal(0);
    master->SetTriggerDelayNs(150);             // zero ns trigger delay

    /* initialize board */
    slave->Init();
    slave->SetRefclk(1); //external clock
    /* set sampling frequency */
    slave->SetFrequency(5, true);
    /* enable transparent mode needed for analog trigger */
    slave->SetTranspMode(1);
    /* set input range to -0.5V ... +0.5V */
    slave->SetInputRange(0);

    /* use following line to set range to 0..1V */
    /* use following line to turn on the internal 100 MHz clock connected to all channels  */
    slave->EnableTcal(1);

    //set ref clock to external
    slave->SetRefclk(1);

    /* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */

    /* use following lines to enable the external trigger */
    if (slave->GetBoardType() == 8) {     // Evaluaiton Board V4
        slave->EnableTrigger(1, 0);           // enable hardware trigger
        slave->SetTriggerSource(1<<4);        // set external trigger as source
    } else {                          // Evaluation Board V3
        slave->EnableTrigger(1, 0);           // lemo on, analog trigger off
    }

    slave->EnableTcal(0);

    slave->SetTriggerDelayNs(150);             // zero ns trigger delay



    string name = "/dev/null";
    if(argc > 1)
        name = argv[1];

    f = fopen(name.c_str(), "a");

    if (f == NULL) {
        perror(("ERROR: Cannot open file \""+ name +"\"").c_str());
        return 1;
    }

    newtCls();
    newtDrawRootText(1, 1, "Reseting motor" );
    newtRefresh();

    running = 1;
    auto w  = std::thread(waiter);
    //Motor m;
    //m.moveUp(-4000*105);
    int motorPosition = 0;
    while(running) {
        newtCls();
        newtDrawRootText(1, 1, "Acquiring           ");
        newtDrawRootText(1, 2, "Reading 100 events             ");
        newtDrawRootText(1, 4, ("Laser Position " + std::to_string(motorPosition / 4000) +"                  ").c_str());
        for (int event = 0; event < eventsPerStep; event++) {
            newtDrawRootText(1, 3, ("Event: " + to_string(event) + "      ").c_str());

            slave->EnableTrigger(1, 0);             // enable hardware trigger
            slave->SetTriggerSource(1 << 4);        // set external trigger as source

            // enable hardware trigger
            master->EnableTrigger(1, 0);           // enable hardware trigger
            master->SetTriggerSource(1 << 4);        // set external trigger as source
            /* start board (activate domino wave) */
            slave->StartDomino();                   /* start board (activate domino wave) */
            master->StartDomino();

            while (master->IsBusy()) std::this_thread::sleep_for(std::chrono::milliseconds(1));
            while (slave->IsBusy()) std::this_thread::sleep_for(std::chrono::milliseconds(1)); //this should not block since the triggeres are daisychained

            /* read all waveforms */
            master->TransferWaves(0, 8);

            /* read time (X) array of first channel in ns */
            // b->GetTime(0, 0, b->GetTriggerCell(0), time_array[0]);

            /* decode waveform (Y) array of first channel in mV */
            master->GetWave(0, 0, wave_array[0]);

            /* read time (X) array of second channel in ns
             Note: On the evaluation board input #1 is connected to channel 0 and 1 of
             the DRS chip, input #2 is connected to channel 2 and 3 and so on. So to
             get the input #2 we have to read DRS channel #2, not #1. */
            //b->GetTime(0, 2, b->GetTriggerCell(0), time_array[1]);

            /* decode waveform (Y) array of second channel in mV */
            master->GetWave(0, 2, wave_array[1]);

            //b->GetTime(0, 4, b->GetTriggerCell(0), time_array[2]);
            /* decode waveform (Y) array of second channel in mV */
            master->GetWave(0, 4, wave_array[2]);

            //b->GetTime(0, 6, b->GetTriggerCell(0), time_array[3]);
            /* decode waveform (Y) array of second channel in mV */


            master->GetWave(0, 6, wave_array[3]);
            for (int chan = 0; chan < 4; chan++) {
                //Event num
                fprintf(f, "%d ", actualEventNumber);
                //Print the board id
                fprintf(f, "%d ", boardid_master);
                //print channel
                fprintf(f, "%d ", chan);
                fprintf(f, "%d ", master->GetTriggerCell(0));
                fprintf(f, "%d ", motorPosition);
                fprintf(f, "%d ", 0);
                fprintf(f, "%.1f ", master->GetTemperature());
                for (int cell = 0; cell < 1024; cell++)
                    fprintf(f, "%.1f ", wave_array[chan][cell]);
                fprintf(f, "\n");
            }

            /* read all waveforms */
            slave->TransferWaves(0, 8);

            /* read time (X) array of first channel in ns */
            // b->GetTime(0, 0, b->GetTriggerCell(0), time_array[0]);

            /* decode waveform (Y) array of first channel in mV */
            slave->GetWave(0, 0, wave_array[0]);

            /* read time (X) array of second channel in ns
             Note: On the evaluation board input #1 is connected to channel 0 and 1 of
             the DRS chip, input #2 is connected to channel 2 and 3 and so on. So to
             get the input #2 we have to read DRS channel #2, not #1. */
            //b->GetTime(0, 2, b->GetTriggerCell(0), time_array[1]);

            /* decode waveform (Y) array of second channel in mV */
            slave->GetWave(0, 2, wave_array[1]);

            //b->GetTime(0, 4, b->GetTriggerCell(0), time_array[2]);
            /* decode waveform (Y) array of second channel in mV */
            slave->GetWave(0, 4, wave_array[2]);

            //b->GetTime(0, 6, b->GetTriggerCell(0), time_array[3]);
            /* decode waveform (Y) array of second channel in mV */


            slave->GetWave(0, 6, wave_array[3]);
            for (int chan = 0; chan < 4; chan++) {
                //Event num
                fprintf(f, "%d ", actualEventNumber);
                //Print the board id
                fprintf(f, "%d ", boardid_slave);
                //print channel
                fprintf(f, "%d ", chan);
                fprintf(f, "%d ", master->GetTriggerCell(0));
                fprintf(f, "%d ", motorPosition);
                fprintf(f, "%d ", 0);
                fprintf(f, "%.1f ", master->GetTemperature());
                for (int cell = 0; cell < 1024; cell++)
                    fprintf(f, "%.1f ", wave_array[chan][cell]);
                fprintf(f, "\n");
            }

            actualEventNumber++;


            newtRefresh();
        }
        int newPos;
        if (randomSteps)
            newPos = std::rand()%(stepsPerTurn);
        else
            newPos = stepsPerTurn;
        motorPosition += newPos;
        if(motorPosition > 72*4000){
            motorPosition = 0;
            newPos = -1*motorPosition;
        }
        newtDrawRootText(1, 1, ("Moving to " + std::to_string(motorPosition)).c_str());
        newtRefresh();
        //m.moveUp(newPos);
        this_thread::sleep_for(std::chrono::milliseconds(pauseAfterStep));
    }
    fclose(f);
    delete drs;
    newtFinished();
}
