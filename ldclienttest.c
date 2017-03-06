/*
$ gcc ldclient.c povgeometry.c ldclienttest.c -o ldclienttest
$ ./ldclienttest <server ip>
*/



#include <stdio.h>
#include <time.h>
#include "leddata.h"
#include "ldclient.h"
#include "povgeometry.h"


#define MAX_BRIGHTNESS 255 //64

static uint8_t leddata[POV_FRAME_SIZE] = {0};


///////////////////////////////////////////////
//
int main(int argc, char *argv[]) {

  
  int i;
  
  if(argc < 2)
  {
    printf("usage:\n%s <server ip>\n", argv[0]);
    return -1;
  }

  //Init fake led data
  for(i=0; i<(POV_FRAME_SIZE); i++)
  {
    leddata[i] = i % 256;
    if(i == 666)
    {
      leddata[i] = 13;
    }
  }

  if(LDconnect(argv[1]))
  {
    printf("failed to connect to server\n");
    return 1;
  }

  if(LDTransmit(leddata, POV_FRAME_SIZE))
  {
    printf("Failed to transmit led data to server\n");
    return 3;
  }

  if(LDWaitforAck())
  {
    printf("got no ack\n");
  }

  LDDisconnect();  

  return 0;
}

