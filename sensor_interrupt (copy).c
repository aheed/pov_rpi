/*
To build:
gcc sensor_interrupt.c -lwiringPi -lrt
 */
 
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <wiringPi.h>

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
  struct timespec resolution;

  printf ("Raspberry Pi interrupt handling with wiringPi\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;

 
  pinMode (6,  OUTPUT) ;         // aka BCM_GPIO pin 17
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  // set Pin 18 generate an interrupt on high-to-low transitions
  // and attach myInterrupt() to the interrupt
//  if ( wiringPiISR (5, INT_EDGE_FALLING, &myInterrupt) < 0 ) {
  if ( wiringPiISR (5, INT_EDGE_BOTH, &myInterrupt) < 0 ) {
      fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      return 1;
  }

  digitalWrite (4, 1) ;       // On
 
  for (;;)
  {

//    digitalWrite (6, eventCounter % 2) ;
    if(oldeventCounter != eventCounter)
    {
      clock_gettime(CLOCK_MONOTONIC, &resolution);
      start_time = resolution.tv_nsec;
  
      //--------------------------
      clock_gettime(CLOCK_MONOTONIC, &resolution);
      current_time = resolution.tv_nsec;

      elapsed_time1 = current_time - start_time;
      if(elapsed_time1 < 0) {
        elapsed_time1 = elapsed_time1 + 1000000000; //in case clock_gettime wraps around
      }
//      printf("%i\n", elapsed_time1);
      //---------------------------
      
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      tmp = tmp + 3;
      
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);

      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);

      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 1);
clock_gettime(CLOCK_MONOTONIC, &resolution);
      digitalWrite (6, 0);
clock_gettime(CLOCK_MONOTONIC, &resolution);



      //--------------------------
      clock_gettime(CLOCK_MONOTONIC, &resolution);
      current_time = resolution.tv_nsec;

      elapsed_time2 = current_time - start_time;
      if(elapsed_time2 < 0) {
        elapsed_time2 = elapsed_time2 + 1000000000; //in case clock_gettime wraps around
      }
//      printf("%i\n", elapsed_time2);
      //---------------------------


      printf ("cnt:%d light:%d\n", eventCounter, digitalRead (5)) ;
      oldeventCounter = eventCounter;

      //--------------------------
      clock_gettime(CLOCK_MONOTONIC, &resolution);
      current_time = resolution.tv_nsec;

      elapsed_time3 = current_time - start_time;
      if(elapsed_time3 < 0) {
        elapsed_time3 = elapsed_time3 + 1000000000; //in case clock_gettime wraps around
      }
      printf("%i\n%i\n%i\n\n", elapsed_time1, elapsed_time2, elapsed_time3);
      //---------------------------

     
    }
        digitalWrite (6, digitalRead (5)) ;
//    digitalWrite (6, digitalRead (5)) ;
//    digitalWrite (6, 1) ;
    delay (100) ;               // mS
//    digitalWrite (6, 0) ;       // Off
//    delay (50) ;
  }
  return 0 ;
}

