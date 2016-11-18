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
#include <string.h>
#include <stdlib.h>

#include "strlcpy.h"
#include "DRS.h"
#include <iostream>

#include <ctime>

using namespace std;

/*------------------------------------------------------------------*/

int main(int argc, char **argv) {

    if (argc != 3) return -1;
    int nBoards;
    DRS *drs;
    DRSBoard *b;
    float time_array[4][1024];
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
    b->EnableTrigger(1,0);
    b->SetTranspMode(0);
    b->SetTriggerLevel(-0.01);            // 0.05 V
    b->SetTriggerPolarity(false);        // positive edge
    b->SetTriggerSource(1<<0);

    b->EnableTcal(0);

    b->SetTriggerDelayNs(50);             // zero ns trigger delay


    std::time_t now = std::time(NULL);
    std::tm *ptm = std::localtime(&now);
    char buffer[32];
    string name = argv[1];
    f = fopen(name.c_str(), "a");
    if (f == NULL) {
        perror("ERROR: Cannot open file \"data.txt\"");
        return 1;
    }

    for (int event = 0; event < 1000; event++) {

        /* start board (activate domino wave) */
        b->StartDomino();

        /* wait for trigger */
        printf("Waiting for trigger...");

        fflush(stdout);
        while (b->IsBusy());

        /* read all waveforms */
        b->TransferWaves(0, 8);

        /* read time (X) array of first channel in ns */
        b->GetTime(0, 0, b->GetTriggerCell(0), time_array[0]);

        /* decode waveform (Y) array of first channel in mV */
        b->GetWave(0, 0, wave_array[0]);

        /* read time (X) array of second channel in ns
         Note: On the evaluation board input #1 is connected to channel 0 and 1 of
         the DRS chip, input #2 is connected to channel 2 and 3 and so on. So to
         get the input #2 we have to read DRS channel #2, not #1. */
        b->GetTime(0, 2, b->GetTriggerCell(0), time_array[1]);

        /* decode waveform (Y) array of second channel in mV */
        b->GetWave(0, 2, wave_array[1]);

        b->GetTime(0, 4, b->GetTriggerCell(0), time_array[2]);
        /* decode waveform (Y) array of second channel in mV */
        b->GetWave(0, 4, wave_array[2]);

        b->GetTime(0, 6, b->GetTriggerCell(0), time_array[3]);
        /* decode waveform (Y) array of second channel in mV */
        b->GetWave(0, 6, wave_array[3]);

        for (int chan = 0; chan< 4; chan++) {
            //Event num
            fprintf(f, "%d ", event);
            //Print the board id
            fprintf(f, "%d ", board_id);
            //print channel
            fprintf(f, "%d ", chan);
            fprintf(f, "%d ", b->GetTriggerCell(0));
            fprintf(f, "%s ", argv[2]);
            fprintf(f, "%d ", 0);
            for (int cell = 0; cell < 1024; cell++)
                fprintf(f, "%7.1f ", wave_array[chan][cell]);
            fprintf(f,"\n");
        }
        /* print some progress indication */
        printf("\033[2J\033[1;1H");
        printf("Event #%d read successfully\n", event);
    }

    fclose(f);

    /* delete DRS object -> close USB connection */
    delete drs;
}
