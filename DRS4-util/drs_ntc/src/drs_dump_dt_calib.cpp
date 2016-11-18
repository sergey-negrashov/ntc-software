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

/*------------------------------------------------------------------*/

int main()
{
   int i, j, nBoards;
   DRS *drs;
   DRSBoard *b;
   float time_array[8][1024];
   float time_dt[8][1024];
   float wave_array[8][1024];
   FILE  *f;

   /* do initial scan */
   drs = new DRS();

   /* show any found board(s) */
   for (i=0 ; i<drs->GetNumberOfBoards() ; i++) {
      b = drs->GetBoard(i);
      printf("## Found DRS4 evaluation board, serial #%d, firmware revision %d\n", 
         b->GetBoardSerialNumber(), b->GetFirmwareVersion());
   }

   /* exit if no board found */
   nBoards = drs->GetNumberOfBoards();
   if (nBoards == 0) {
      printf("## No DRS4 evaluation board found\n");
      return 0;
   }

   /* continue working with first board only */
   b = drs->GetBoard(0);

   /* initialize board */
   b->Init();

   for (j=0 ; j<4 ; j++) {
      b->GetTimeCalibration(0, j*2, 0, time_dt[j], 0);
   }
   printf("#Sample\tdt_ch0\t\tdt_ch1\t\tdt_ch2\t\tdt_ch3\n");
   for (int samp = 0; samp < 1024; ++samp) {
      printf("%d\t%f\t%f\t%f\t%f\n",samp,time_dt[0][samp],time_dt[1][samp],time_dt[2][samp],time_dt[3][samp]);
   }

   /* delete DRS object -> close USB connection */
   delete drs;
}
