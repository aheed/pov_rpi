/*
 * blink.c:
 *      blinks the first LED
 *      Gordon Henderson, projects@drogon.net
 */
 
#include <stdio.h>
#include <wiringPi.h>
 
int main (void)
{
  printf ("Raspberry Pi blink\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;
 
  pinMode (6,  OUTPUT) ;         // aka BCM_GPIO pin 17
  pinMode (4,  OUTPUT) ; //pin 16
  pinMode (5, INPUT) ;  //pin 18

  digitalWrite (4, 1) ;       // On
 
  for (;;)
  {
    digitalWrite (6, digitalRead (5)) ;
//    digitalWrite (6, 1) ;
    delay (1) ;               // mS
//    digitalWrite (6, 0) ;       // Off
//    delay (50) ;
  }
  return 0 ;
}

