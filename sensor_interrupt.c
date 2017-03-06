/*
To build:
gcc sensor_interrupt.c -lwiringPi -lrt
 */
 
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>

#define MIN_REV_TIME 1000000 //nsec

volatile int eventCounter = 0;
volatile int oldeventCounter = 0;

// -------------------------------------------------------------------------
void myInterrupt(void) {
   eventCounter++;
}


// -------------------------------------------------------------------------
int main (void)
{
  int start_time, current_time, elapsed_time1, elapsed_time2, elapsed_time3, tmp;
  int rev_start_time = 0;
  int time_since_rev_start;
  int last_rev_time = 0;
  int loops_last_rev = 0;
  struct timespec resolution;

  printf ("Raspberry Pi interrupt handling with wiringPi\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;

 
  pinMode (6,  OUTPUT) ;         // aka BCM_GPIO pin 17
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (0,  OUTPUT) ; //pin 11
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
  if ( wiringPiISR (5, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
//  if ( wiringPiISR (5, INT_EDGE_BOTH, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  digitalWrite (4, 1) ;       // On
 
  for (;;)
  {

    clock_gettime(CLOCK_MONOTONIC, &resolution);
    current_time = resolution.tv_nsec;

    time_since_rev_start = current_time - rev_start_time;
    if(time_since_rev_start < 0)
    {
      time_since_rev_start = time_since_rev_start + 1000000000; //in case clock_gettime wraps around
    }

    if(oldeventCounter != eventCounter)
    {
      if(time_since_rev_start > MIN_REV_TIME)
      {
        rev_start_time = current_time;
        last_rev_time = time_since_rev_start;
        printf("lap time:%d loops:%d\n", time_since_rev_start, loops_last_rev);
        loops_last_rev = 0;
      }

      //printf ("cnt:%d light:%d\n", eventCounter, digitalRead (5)) ;
      oldeventCounter = eventCounter;
    }


    //-------------------------------------------------------
    // Update LED(s) depending on estimated current display angle
    
    // LED on if less than 10% of the rev is completed
    digitalWrite (0, (last_rev_time / time_since_rev_start) > 10);

    
    //-------------------------------------------------------
    
    digitalWrite (6, digitalRead (5)) ;

//    delay (1) ;               // mS  TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    loops_last_rev++;
  }
  return 0 ;
}

