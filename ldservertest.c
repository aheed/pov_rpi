/*
$ gcc ldservertest.c ldserver.c -o ldservertest -lpthread

*/

#include <stdio.h>
#include <time.h>
#include "povgeometry.h"
#include "ldserver.h"

static unsigned char myframebuf[POV_FRAME_SIZE] = {0};


int main( int argc, char* args[] )
{
  struct timespec sleeper, dummy;
  int frames = 0;
  int tmp;
  int ledoffset;

  if(LDListen(myframebuf, POV_FRAME_SIZE))
  {
    return 1;
  }

  while(1)
  {
    // wait a while
    sleeper.tv_sec  = 0;
    sleeper.tv_nsec = 10000000;
    nanosleep(&sleeper, &dummy);

    tmp = LDGetReceivedFrames();
    if(tmp != frames)
    {
      frames = tmp;
      printf("Got %d frames\n", frames);
      ledoffset = 0;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset += 4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset += 4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);

      ledoffset = 119*4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset = 120*4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset = 121*4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);

      ledoffset = (POV_FRAME_SIZE/8 - 1)*4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset = POV_FRAME_SIZE-8;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset = POV_FRAME_SIZE-4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);
      ledoffset = 33*32*4;
      printf("....ledoffset:%d\n", ledoffset);
      printf("0x%X 0x%X 0x%X 0x%X\n\n", 
             myframebuf[ledoffset + 0],
             myframebuf[ledoffset + 1],
             myframebuf[ledoffset + 2],
             myframebuf[ledoffset + 3]);

    }
  }
}


