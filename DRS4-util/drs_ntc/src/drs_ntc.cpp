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

using namespace std;

/*------------------------------------------------------------------*/

int main(int argc, char **argv) {

    Motor m;
    if (argc < 2) return -1;
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

    /* use following line to turn on the internal 100 MHz clock connected to all channels  */
    b->EnableTcal(1);
    /* use following lines to enable hardware trigger on CH1 at 50 mV positive edge */
    /*
    b->EnableTrigger(1,0);
    b->SetTranspMode(0);
    b->SetTriggerLevel(-0.01);            // 0.05 V
    b->SetTriggerPolarity(false);        // positive edge
    b->SetTriggerSource(1<<0);
    */

    /* use following lines to enable the external trigger */
    if (b->GetBoardType() == 8) {     // Evaluaiton Board V4
        b->EnableTrigger(1, 0);           // enable hardware trigger
        b->SetTriggerSource(1<<4);        // set external trigger as source
    } else {                          // Evaluation Board V3
        b->EnableTrigger(1, 0);           // lemo on, analog trigger off
    }

    b->EnableTcal(0);

    b->SetTriggerDelayNs(150);             // zero ns trigger delay

    string name = argv[1];
    f = fopen(name.c_str(), "a");

    if (f == NULL) {
        perror(("ERROR: Cannot open file \""+ name +"\"").c_str());
        return 1;
    }

    ZmqPub net("tcp://127.0.0.1:5530");
    newtCls();
    newtDrawRootText(1, 1, "Reseting motor" );
    m.moveUp(-4000*105);
    int motorPosition = 0;

    for(;motorPosition < 4000*75; motorPosition+=4000) {
        newtCls();
        ProtoMotorPosition p(motorPosition);
        newtDrawRootText(1, 1, "Reading 1000 events");
        newtDrawRootText(1, 3, ("Laser Position " + std::to_string(motorPosition / 4000)).c_str());
        for (int event = 0; event < 1000; event++) {
            b->EnableTrigger(1, 0);           // enable hardware trigger
            b->SetTriggerSource(1 << 4);        // set external trigger as source
            newtDrawRootText(1, 2, ("Event: " + to_string(event)).c_str());
            // enable hardware trigger
            b->SetTriggerSource(1 << 4);        // set external trigger as source
            /* start board (activate domino wave) */
            b->StartDomino();

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
            p.addEvent(b, event);
            for (int chan = 0; chan < 4; chan++) {
                //Event num
                fprintf(f, "%d ", event);
                //Print the board id
                fprintf(f, "%d ", board_id);
                //print channel
                fprintf(f, "%d ", chan);
                fprintf(f, "%d ", b->GetTriggerCell(0));
                fprintf(f, "%d ", motorPosition);
                fprintf(f, "%d ", 0);
                for (int cell = 0; cell < 1024; cell++)
                    fprintf(f, "%.1f ", wave_array[chan][cell]);
                fprintf(f, "\n");
            }
            newtRefresh();
        }
        m.moveUp(4000);
        net.sendData(p);
    }
    fclose(f);
    delete drs;
    newtFinished();
}
