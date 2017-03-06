#include <stdio.h>
#include <pthread.h>
#include "leddata.h"
#include "animator.h"

static char * pLeddata = NULL;
static pthread_t worker;


void *worker_entry(void *param)
{
  struct timespec sleeper, dummy;
  int animx = 0;
  int animy = 0;

  int animstepx = 12;
  int animstepy = 7;
  const int animmax = 100;

  printf("entering new thread\n");

  while(1)
  {
    //printf("working...\n");

    //animate
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
        
    LDgetLedData2(animx,
                  animy,
                  pLeddata);

    // wait a while
    sleeper.tv_sec  = 0;
    sleeper.tv_nsec = 100000000;
    nanosleep(&sleeper, &dummy);
  }
}

void StartAnimation(char * const pLeddataOut)
{
  pLeddata = pLeddataOut;

  if(pthread_create(&worker,
                    NULL,          //attributes
                    worker_entry,
                    NULL))         //parameter
  {
    fprintf(stderr, "Error creating thread\n");
  }
}



