/*
Test: open a bitmap file and convert the contents to a Dotstar LED data buffer

$ gcc animator.c bmp.c leddata.c pictest.c -o pictest -lm -lpthread
$ ./pictest

*/

#include <stdio.h>
#include <unistd.h>
#include "leddata.h"
#include "animator.h"


#define NOF_LEDS     32
#define NOF_SECTORS 120

// millimeters or whatever
/*static const int ledRadius[NOF_LEDS] =
  {50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230 ,236};*/

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

static char leddata[NOF_SECTORS * NOF_LEDS * 4] = {0};

int main( int argc, char* args[] )
{
  int i;

  if(LDInitFromBmp("/home/anders/src/sim/test_photo.bmp",
                   128,
                   NOF_SECTORS,
                   NOF_LEDS,
                   ledRadius))
  {
    printf("failed!\n");
    return 1;
  }

  printf("success!\n");

/*  printf("&leddata:0x%X\n", (int)leddata);
  printf("&leddata:0x%X\n", (int)&(leddata[0]));
  printf("leddata[0]:0x%X\n", (int)(leddata[0]));
  printf("&leddata[1]:0x%X\n", (int)&(leddata[1]));
  printf("&leddata[max]:0x%X\n", (int)&(leddata[NOF_SECTORS * NOF_LEDS * 4-1]));*/

  LDgetLedData(NOF_SECTORS,
               NOF_LEDS,
               ledRadius,
               0,  // x offset
               0,  // y offset
               leddata);

  LDgetLedData2(-40,  // x offset
                -10,  // y offset
                leddata);

  StartAnimation(leddata);

  sleep(1);

  printf("leddata[0]:0x%X\n", (int)(leddata[0]));
  printf("leddata[1]:0x%X\n", (int)(leddata[1]));
  printf("leddata[2]:0x%X\n", (int)(leddata[2]));
  printf("leddata[3]:0x%X\n", (int)(leddata[3]));
 
  for(i=0; i<sizeof(leddata); i=i+4)
  {
    int tmp = (leddata[i] << 24) | (leddata[i+1] << 16) | (leddata[i+2] << 8) | leddata[i+3];
    printf("0x%X ", tmp);
  }
  printf("\n");

  

  LDRelease();

  return 0;    
}

