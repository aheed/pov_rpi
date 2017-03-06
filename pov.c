/* 
Persistence Of Vision project
  Raspberry Pi
  Adafruit Dotstar strip, HW SPI
  Interrupt driven rev synch, wiringPi

$ gcc bmp.c leddata.c ldserver.c pov.c -o pov -lwiringPi -lrt -lm -lpthread
$ sudo ./pov

*/


#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <time.h>
#include <wiringPi.h>
#include "leddata.h"
#include "povgeometry.h"
#include "ldserver.h"

#define NOF_SECTORS 120
#define NOF_LEDS 32


#define SPI_BITRATE 8000000

#define SECTOR_DATA_SIZE (sizeof(leddata) / NOF_SECTORS) //bytes per sector
#define LED_DATA_SIZE 4 //bytes per LED

//255 is max but battery and rPi have power limitations
#define MAX_BRIGHTNESS 64 //128

#define SLEEP_PER_LOOP 10000 //500000 // nanoseconds

#define MIN_REV_TIME 1000000 //nsec

#define REV_TIME_AVG_COEFFICIENT 10 //2
#define REV_TIME_CORRECTION_COEFFICIENT 2


volatile int eventCounter = 0;
volatile int oldeventCounter = 0;

//millimeters x10
static const int ledRadius[NOF_LEDS] =
{
300,
371,
442,
513,
584,
655,
726,
797,
868,
939,
1010,
1081,
1152,
1223,
1294,
1365,
1435,
1506,
1577,
1648,
1719,
1790,
1861,
1932,
2003,
2074,
2145,
2216,
2287,
2358,
2429,
2500
};

/*volatile*/ static uint8_t leddata[NOF_SECTORS * NOF_LEDS * LED_DATA_SIZE] = {0};



// -------------------------------------------------------------------------
void myInterrupt(void) {
   eventCounter++;
}


// -------------------------------------------------------------------------

int main( int argc, char* args[] )
{
  int i;
  int      fd;         // File descriptor if using hardware SPI
  struct timespec sleeper, dummy;
  int led, sector;
  long long rev_start_time = 0;
  long long time_since_rev_start;
  long long last_rev_time = 0;
  long long avg_rev_time = 0;
  long long current_time = 0;
  long long rev_start_time_calc = 0;
  long long time_since_rev_start_calc = 0;
  long long start_time_diff = 0;
  long long max_start_time_diff = 0;

  struct timespec resolution;
  unsigned int current_sector = 93; //current orientation of LED strip
  unsigned int last_sector = 93;


  int animx = 0;
  int animy = 0;

  int animstepx = 7;
  int animstepy = 3;
  const int animmax = 100;

  struct spi_ioc_transfer xfer[3] = {
  { .tx_buf        = 0, // Header (zeros)
    .rx_buf        = 0,
    .len           = 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE},
  { .rx_buf        = 0, // Color payload
    .len           = NOF_LEDS * 4,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE},
  { .tx_buf        = 0, // Footer (zeros)
    .rx_buf        = 0,
    .len = (NOF_LEDS + 15) / 16,
    .delay_usecs   = 0,
    .bits_per_word = 8,
    .cs_change     = 0,
    .speed_hz = SPI_BITRATE}
  };


  /////////////////////////////////////////
  // initialize LED data.

/*  for(sector=0; sector < NOF_SECTORS; sector++)
  {
    for(led=0; led < NOF_LEDS; led++)
    {
      int ledoffset = (sector * SECTOR_DATA_SIZE) + (led * LED_DATA_SIZE);
      leddata[ledoffset + 0] = 0xFF;            //must be 0xFF

      leddata[ledoffset + 1] = 
        (MAX_BRIGHTNESS * sector) / NOF_SECTORS;   //blue

      leddata[ledoffset + 2] =                 
        MAX_BRIGHTNESS - 
          (MAX_BRIGHTNESS * sector) / NOF_SECTORS; //green

      leddata[ledoffset + 3] =                     //red
//          (MAX_BRIGHTNESS * led) / NOF_LEDS;
        0;

      if((led >= 10) && (led <= 20) && (sector >= 40) && (sector <= 60))
      {
         leddata[ledoffset + 1] = 0;
         leddata[ledoffset + 2] = 0;
         leddata[ledoffset + 3] = MAX_BRIGHTNESS;
      }

//      printf("0x%X 0x%X 0x%X 0x%X\n", 
//       leddata[ledoffset + 0],
//       leddata[ledoffset + 1],
//       leddata[ledoffset + 2],
//       leddata[ledoffset + 3]);
    }
  }*/

  // Get led data from bmp file
  if(LDInitFromBmp("/media/networkshare/julius_src/repo/pov_rpi/test_photo.bmp",
//  if(LDInitFromBmp("/media/networkshare/public/src/pov_rpi/test_photo.bmp",
//  if(LDInitFromBmp("/media/networkshare/public/src/pov_rpi/sandra3.bmp",  
                   MAX_BRIGHTNESS,
                   NOF_SECTORS,
                   NOF_LEDS,
                   ledRadius,
                   1))
  {
    printf("failed to get led data from bmp file!\n");
    return 1;
  }

  LDgetLedData(   NOF_SECTORS,
                  NOF_LEDS,
                  ledRadius,
                  animx,
                  animy,
                  leddata);

  printf("successfully read bmp data!\n");

  ///////////////////////////////////////7
  // Init SPI  
  if((fd = open("/dev/spidev0.0", O_RDWR)) < 0) {
    printf("Can't open /dev/spidev0.0 (try 'sudo')\n");
    return 2;
  }
 
  uint8_t mode = SPI_MODE_0 | SPI_NO_CS;
  ioctl(fd, SPI_IOC_WR_MODE, &mode);
  // The actual data rate may be less than requested.
  // Hardware SPI speed is a function of the system core
  // frequency and the smallest power-of-two prescaler
  // that will not exceed the requested rate.
  // e.g. 8 MHz request: 250 MHz / 32 = 7.8125 MHz.
  ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, SPI_BITRATE);

  printf("\n\n");


  /////////////////////////////////////////////////////////
  // Set up interrupt routine for rev synch with WiringPi

  if (wiringPiSetup () == -1)
    return 3 ;
  
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 to generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (5, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  // Start animation thread
//  StartAnimation(leddata);

  if(LDListen(leddata, POV_FRAME_SIZE))
  {
    fprintf (stderr, "Failed to setup server\n");
    return 1;
  }

  while(1)
  {
    ///////////////////////////////////////////
    // What time is it?
    clock_gettime(CLOCK_MONOTONIC, &resolution);
    current_time = resolution.tv_nsec;

    time_since_rev_start = current_time - rev_start_time;
    if(time_since_rev_start < 0)
    {
      time_since_rev_start = time_since_rev_start + 1000000000; //in case clock_gettime wraps around
    }


    //-----------------------------------------------
    time_since_rev_start_calc = current_time - rev_start_time_calc;
    if(time_since_rev_start_calc < 0)
    {
      time_since_rev_start_calc += 1000000000; //in case clock_gettime wraps around
    }

    while((avg_rev_time != 0) && (time_since_rev_start_calc > avg_rev_time))
    {
      // By now a new rev should have started.
      time_since_rev_start_calc = time_since_rev_start_calc - avg_rev_time;

      rev_start_time_calc += avg_rev_time;
      if(rev_start_time_calc > 1000000000)
      {
        rev_start_time_calc -= 1000000000; //wrap
      }

    }
    //---------------------------------------------------

    ///////////////////////////////////////////
    // Handle any received rev synch interrupt
    if(oldeventCounter != eventCounter)
    {
      if(time_since_rev_start > MIN_REV_TIME)
      {
        rev_start_time = current_time;


        if( ((last_rev_time - time_since_rev_start) < (last_rev_time / 10)) &&
            ((time_since_rev_start - last_rev_time) < (last_rev_time / 10)) )
        {
          // Rev time differs little from last rev:
          // It is believable so let the last rev contribute to avg
          avg_rev_time = (avg_rev_time * (REV_TIME_AVG_COEFFICIENT-1) + time_since_rev_start) / REV_TIME_AVG_COEFFICIENT;
        }

        last_rev_time = time_since_rev_start;
        

        
        // Adjust calculated rev_start_time     
        start_time_diff = rev_start_time - rev_start_time_calc;

        // Handle clock wrap
        if(start_time_diff < -(1000000000 / 2))
        {
           start_time_diff += 1000000000;
        }

        if(start_time_diff > (1000000000 / 2))
        {
           start_time_diff -= 1000000000;
        }

        //Handle rev wrap
        if(start_time_diff < -(avg_rev_time)/2)
        {
          start_time_diff += avg_rev_time;
        }
        
        if(start_time_diff > (avg_rev_time)/2)
        {
          start_time_diff -= avg_rev_time;
        }

        rev_start_time_calc += start_time_diff / REV_TIME_CORRECTION_COEFFICIENT;

        // Handle clock wrap again
        if(rev_start_time_calc > 1000000000)
        {  
          rev_start_time_calc -= 1000000000;
        }

        if(rev_start_time_calc < -1000000000)
        {  
          rev_start_time_calc += 1000000000;
        }
        


        /*
        //animation
        animx += animstepx;
        animy += animstepy;
        if((animx > animmax) || (animx < -animmax))
        {
          animstepx *= (-1);
        }

        if((animy > animmax) || (animy < -animmax))
        {
          animstepy *= (-1);
        }
        
        // old implementation
        // refresh led data with updated animation offset
        // LDgetLedData(   NOF_SECTORS,
        //          NOF_LEDS,
        //          ledRadius,
        //          animx,
        //          animy,
        //          leddata);
        //
        LDgetLedData2(animx,
                      animy,
                      leddata);*/
      }

      oldeventCounter = eventCounter;
    }

    

    ///////////////////////////////////////////
    // Update sector index
    if(avg_rev_time != 0)
    {
//      current_sector = (NOF_SECTORS * time_since_rev_start) / avg_rev_time;
      current_sector = (NOF_SECTORS * time_since_rev_start_calc) / avg_rev_time;
      while(current_sector >= NOF_SECTORS)
      {
        current_sector -= NOF_SECTORS;
      }
 
      if(current_sector != last_sector)
      {        
        //printf("sector:%u\n", current_sector);
        if( ((current_sector - last_sector) > 1) && (current_sector != 0))
        {
          // a sector was skipped
          //printf("sector:%u last:%u\n", current_sector, last_sector);
        }
        last_sector = current_sector;

        ////////////////////////////////////////
        // Transmit LED data on SPI
        xfer[1].tx_buf   = (unsigned long)&(leddata[current_sector * SECTOR_DATA_SIZE]);
        (void)ioctl(fd, SPI_IOC_MESSAGE(3), xfer);

        /*//sleep 75% of expected time until next sector
        sleeper.tv_sec  = 0;
        sleeper.tv_nsec = (avg_rev_time * 2) / (NOF_SECTORS * 4);
        nanosleep(&sleeper, &dummy);*/
      }
      else
      {
        // wait a while
        sleeper.tv_sec  = 0;
        sleeper.tv_nsec = SLEEP_PER_LOOP;
        nanosleep(&sleeper, &dummy);
      }
    }
    else
    {
      // wait a while
      sleeper.tv_sec  = 0;
      sleeper.tv_nsec = SLEEP_PER_LOOP;
      nanosleep(&sleeper, &dummy);
    }
  }
  

  /*
  // Cleanup
  if(fd) {
   close(fd);
   fd = -1;
  }
  */

  return 0;
}

