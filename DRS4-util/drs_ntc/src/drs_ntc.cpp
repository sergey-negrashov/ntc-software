/********************************************************************\

  Name:         drs_exam.cpp
  Created by:   Stefan Ritt

  Contents:     Simple example application to read out a DRS4
                evaluation board

  $Id: drs_exam.cpp 21308 2014-04-11 14:50:16Z ritt $

\********************************************************************/

#include <math.h>

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
#include "ZmqPub.hpp"
#include "ProtoSerializer.hpp"
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
    int moveMotor = 1;
    int triggerType = 0; // 0 - laser trigger, 1 - software trigger, 2 - OR of 1/2, 3 - AND of 1/2
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
            if(items[0] == "MoveMotor")
                moveMotor = std::stoi(items[1]);
            if(items[0] == "TriggerType")
                triggerType = std::stoi(items[1]);

        }
        settingsFile.close();
    }

    cout << "Steps per Turn: " << stepsPerTurn << endl;
    cout << "Events per Turn " << eventsPerStep << endl;
    cout << "Random " << (randomSteps ? "No": "Yes") << endl;
    cout << "Pause after move " << pauseAfterStep << "ms" << endl;
    cout << "Moving motor? " << moveMotor << endl;
    cout << "TriggerType: " << triggerType << endl;
    int actualEventNumber = 1;
    std::srand(std::time(0));
    Motor m;
    int nBoards;
    DRS *drs;
    DRSBoard *b;
    //float time_array[4][1024];
    float wave_array[4][1024];
    FILE *f;

    /* do initial scan */
    drs = new DRS();

    /* show any found board(s) */
    for (int i = 0; i < drs->GetNumberOfBoards(); i++) {
        b = drs->GetBoard(i);
        printf("Found DRS4 evaluation board, serial #%d, firmware revision %d\n",
               b->GetBoardSerialNumber(), b->GetFirmwareVersion());
    }

    /* exit if no board found */
    nBoards = drs->GetNumberOfBoards();
    if (nBoards == 0) {
        printf("No DRS4 evaluation board found\n");
        return 0;
    }

    /* continue working with first board only */
    b = drs->GetBoard(0);

    //Get board id
    int board_id = b->GetBoardSerialNumber();

    newtInit();


    /* initialize board */
    b->Init();

    /* set sampling frequency */
    b->SetFrequency(5, true);

    /* enable transparent mode needed for analog trigger */
    b->SetTranspMode(1);

    /* set input range to -0.5V ... +0.5V */
    b->SetInputRange(0);

    /* use following line to set range to 0..1V */
    //b->SetInputRange(0.5);

    /* use following line to set the internal 100 MHz clock connected to all channels  */
    b->EnableTcal(0);

    // Trigger types
    // 0 - laser/external
    // 1 - dark (software)
    // 2 - OR of channels 1 and 2, standard thresholds set below
    // 3 - AND of channels 1 and 2, stanard thresholds set below

    switch(triggerType) {
       case(0):
          b->EnableTrigger(0, 0);           // enable hardware trigger
          b->SetTriggerSource(1<<4);        // set external trigger as source
          b->SetTriggerDelayNs(150);             // ns trigger delay
          break; 
       case(1):
          b->EnableTrigger(0,0);
          break;
       case(2):
          b->SetTranspMode(0);
          b->EnableTrigger(1, 0);           // enable hardware trigger
//          b->SetIndividualTriggerLevel(0,0.010);  //Standard setup - Serge
//          b->SetIndividualTriggerLevel(1,0.01);   //               - Serge
          b->SetIndividualTriggerLevel(0,-0.01);  //Tweaking triggers - Kurtis
          b->SetIndividualTriggerLevel(1,-0.007); //                  - Kurtis
          b->SetTriggerPolarity(false);        // positive edge //Standard setup
          b->SetTriggerSource(1<<1 | 1<<0);    // ch 1 OR ch 2
          b->SetTriggerDelayNs(50);             // ns trigger delay
          break;
       case(3):
          b->SetTranspMode(0);
          b->EnableTrigger(1, 0);           // enable hardware trigger
          b->SetIndividualTriggerLevel(0,-0.010);
          b->SetIndividualTriggerLevel(1,-0.007);
          b->SetTriggerPolarity(false);        // positive edge
          b->SetTriggerSource(1<<8 | 1<<9);    // ch 1 AND ch 2
          b->SetTriggerDelayNs(50);             // ns trigger delay
          break;
       others:
          cout << "ERROR: No valid trigger type set!" << endl;
          return 1;
    }

    string name = "/dev/null";
    if(argc > 1)
        name = argv[1];

    f = fopen(name.c_str(), "a");

    if (f == NULL) {
        perror(("ERROR: Cannot open file \""+ name +"\"").c_str());
        return 1;
    }

    ZmqPub net("tcp://127.0.0.1:5530");
    newtCls();
    newtDrawRootText(1, 1, "Reseting motor" );
    newtRefresh();

    running = 1;
    auto w  = std::thread(waiter);
    if (moveMotor) {
       m.moveUp(-105*4000); //Move to near the wall (SiPM #1)
//    m.moveUp(288000);  //Do the top first, then this if you want to move to the end point of the scan first
// If you do the above, adjust motorPosition accordingly.
    }
    int motorPosition = 0;
    while(running) {
        newtCls();
        ProtoMotorPosition p(motorPosition);
        newtDrawRootText(1, 1, "Acquiring           ");
        newtDrawRootText(1, 2, ("Reading " + std::to_string(eventsPerStep) + " events             ").c_str());
        newtDrawRootText(1, 4, ("Laser Position " + std::to_string(motorPosition / 4000) +"                  ").c_str());
        for (int event = 0; event < eventsPerStep; event++) {
            if (triggerType == 0) {
               b->EnableTrigger(1, 0);           // enable hardware trigger
               b->SetTriggerSource(1 << 4);        // set external trigger as source
            }
            newtDrawRootText(1, 3, ("Event: " + to_string(event) + "      ").c_str());
            /* start board (activate domino wave) */
            b->StartDomino();

            if (triggerType == 1) {
               b->SoftTrigger();
            }

            while (b->IsBusy()) std::this_thread::sleep_for(std::chrono::milliseconds(1));

            /* read all waveforms */
            b->TransferWaves(0, 8);

            /* read time (X) array of first channel in ns */
            // b->GetTime(0, 0, b->GetTriggerCell(0), time_array[0]);

            /* decode waveform (Y) array of first channel in mV */
            b->GetWave(0, 0, wave_array[0]);

            /* read time (X) array of second channel in ns
             Note: On the evaluation board input #1 is connected to channel 0 and 1 of
             the DRS chip, input #2 is connected to channel 2 and 3 and so on. So to
             get the input #2 we have to read DRS channel #2, not #1. */
            //b->GetTime(0, 2, b->GetTriggerCell(0), time_array[1]);

            /* decode waveform (Y) array of second channel in mV */
            b->GetWave(0, 2, wave_array[1]);

            //b->GetTime(0, 4, b->GetTriggerCell(0), time_array[2]);
            /* decode waveform (Y) array of second channel in mV */
            b->GetWave(0, 4, wave_array[2]);

            //b->GetTime(0, 6, b->GetTriggerCell(0), time_array[3]);
            /* decode waveform (Y) array of second channel in mV */


            b->GetWave(0, 6, wave_array[3]);
            p.addEvent(b, actualEventNumber);
            for (int chan = 0; chan < 4; chan++) {
                //Event num
                fprintf(f, "%d ", actualEventNumber);
                //Print the board id
                fprintf(f, "%d ", board_id);
                //print channel
                fprintf(f, "%d ", chan);
                fprintf(f, "%d ", b->GetTriggerCell(0));
                fprintf(f, "%d ", motorPosition);
                fprintf(f, "%d ", 0);
                fprintf(f, "%.1f ", b->GetTemperature());
                for (int cell = 0; cell < 1024; cell++)
                    fprintf(f, "%.1f ", wave_array[chan][cell]);
                fprintf(f, "\n");
            }
            actualEventNumber++;
            newtRefresh();
        }
        net.sendData(p);
        int newPos;
        if (randomSteps)
            newPos = std::rand()%(stepsPerTurn);
        else
            newPos = stepsPerTurn;
        if (moveMotor) {
           motorPosition += newPos;
        }

        if(motorPosition > 76*4000+2000){
        //if(motorPosition > 29*4000+2000){
            break;
            motorPosition = 0;
            newPos = -1*motorPosition;
        }

        newtDrawRootText(1, 1, ("Moving to " + std::to_string(motorPosition)).c_str());
        newtRefresh();
        if (moveMotor) {
           m.moveUp(newPos);
        }
        this_thread::sleep_for(std::chrono::milliseconds(pauseAfterStep));
    }
    fclose(f);
    delete drs;
    newtFinished();
    if (moveMotor) {
       m.moveUp(-105*4000); //Return motor to near the wall (SiPM #1)
    }
}
