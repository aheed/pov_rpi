/*
Analog clock POV client

$ gcc ldclient.c leddata.c bmp.c povgeometry.c clock.c -o clock -lm
$ ./clock <server ip>
*/


#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include "leddata.h"
#include "ldclient.h"
#include "povgeometry.h"


//#define MAX_BRIGHTNESS 64

#define HOURS_LEDS 20
#define MINUTES_LEDS 30
#define SECONDS_LEDS 32
#define MARKER_LEDS 3

static uint8_t leddata[POV_FRAME_SIZE] = {0};


///////////////////////////////////////////////
//
int main(int argc, char *argv[]) {
  time_t rawtime;
  struct tm *timeinfo;
  int hours_sector, minutes_sector, seconds_sector;
  int i, j;

  if(argc < 2)
  {
    printf("usage:\n%s <server ip>\n", argv[0]);
    return -1;
  }


  while(1)
  {
    // set up led data according to current time of day

    time( &rawtime );
    timeinfo = localtime( &rawtime );
    printf("Current local time and date: %s", asctime(timeinfo));
      
    LDsetLedDataBlack(NOF_SECTORS, NOF_LEDS, leddata);

    for(i=0; i<12; i++)
    {
      for(j=(NOF_LEDS-MARKER_LEDS); j<NOF_LEDS; j++)
      { 
        LDsetLed(NOF_LEDS,
                (i * NOF_SECTORS) / 12,
                j,
                0,    //blue
                0x7F, //green
                0, //red
                leddata);
      }
    }
    #define MARKER_LEDS 3

    hours_sector = (NOF_SECTORS * ((timeinfo->tm_hour % 12) * 60 + timeinfo->tm_min) ) / (12 * 60);
    hours_sector = (NOF_SECTORS - hours_sector - 1 + NOF_SECTORS/4) % NOF_SECTORS;
    minutes_sector = (NOF_SECTORS * timeinfo->tm_min) / 60;
    minutes_sector = (NOF_SECTORS - minutes_sector - 1 + NOF_SECTORS/4) % NOF_SECTORS;
    seconds_sector = (NOF_SECTORS * timeinfo->tm_sec) / 60;
    seconds_sector = (NOF_SECTORS - seconds_sector - 1 + NOF_SECTORS/4) % NOF_SECTORS;


    for(i=0; i<HOURS_LEDS; i++)
    {
      LDsetLed(NOF_LEDS,
               hours_sector,
               i,
               0,    //blue
               0x7F, //green
               0x7F, //red
               leddata);
    }
    
    for(i=0; i<MINUTES_LEDS; i++)
    {
      LDsetLed(NOF_LEDS,
               minutes_sector,
               i,
               0,    //blue
               0x7F, //green
               0x7F, //red
               leddata);
    }

    for(i=0; i<SECONDS_LEDS; i++)
    {
      LDsetLed(NOF_LEDS,
               seconds_sector,
               i,
               0x7F, //blue
               0x7F, //green
               0x7F, //red
               leddata);
    }

    // Connect to server and transmit data
    if(LDconnect(argv[1]))
    {
      printf("failed to connect to server\n");
    }
    else
    {
      if(LDTransmit(leddata, POV_FRAME_SIZE))
      {
        printf("Failed to transmit led data to server\n");
      }
      else
      {
        if(LDWaitforAck())
        {
          printf("got no ack\n");
        }
      }

      LDDisconnect();
    }

    //wait 1 sec
    sleep(1);
  }


  return 0;
}

