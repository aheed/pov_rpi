POV - Persistence Of Vision using a Raspberry Pi on a scrap HDD motor

Hardware:
Raspberry Pi, model B revision 000d
  pi@hikaru ~ $ uname -a
  Linux hikaru 4.1.19+ #858 Tue Mar 15 15:52:03 GMT 2016 armv6l GNU/Linux
Power bank, Canyon2400mAh
Wifi USB dongle, TP-Link (Realtek) (r8188eu driver)
Scrap HDD
Ljusgivare (Ljus till spänning), TSL250R 



SW:
To build and run GPIO program using WiringPi:
$ gcc -o blink blink.c -lwiringPi
$ sudo ./blink

To build and run interrupt based GPIO program using WiringPi:
$ gcc sensor_interrupt.c -lwiringPi -lrt
$ sudo ./a.out



Todo:
Introduce minimum rev time to avoid interrupt flicker*
Avg rev time to stabilize image*
Make module (.h/.c) to read bmp and convert to Dotstar format
  -copy code from sim -> renderarea.cpp*
  -get gamma correction from Adafruit python example*
  -Reduce brightness (before or after gamma correction? before!)*
  -test on Gulliver*
  -Run on Hikaru to initialize led data (skip this and use TCP server?)*
Measure for correct led radius values*
Implement animation in separate thread*
Add TCP server to accept raw led data (Dotstar format) from remote client
Implement client to do animations and stuff (fading, rotation, sequences...)
Adjust height of counterweights, rebalance*
Change: first gamma correction, then brightness adjustment*
create make file(s)
Try Realtime priority for picture stability (nice -20 does not help at all)*
  -Reducing number of sectors does not help at all
  -longer averaging time does not help at all
  -why is it not stable???
    Observation: slower rotation does help
  -averaging rev time and rev start time helps a bit*
Make ssh key for easier login to rpi
put pov geometry defines in a common include file*
Antialiasing for images including movie frames:
  - cache coordinates and weight factors for a number of source pixels for each led.
    Use gaussian or similar algo. Ideal is to use more pixels for outermost leds.
    1) Find out which pixels are covered by the sector/led combo
    2) calculate weight based on distance to center of sector/led combo
  - First antialiasing, then gamma correction, then brightness adjustment
  - cache sector/led combo (offset) for each pixel. Use this to make every pixel
    covered by sector/led count. Use average of pixel colors as led color value.
    One extra element in cache for pixels not covered by any sector/led? Write only
    -create cache in leddata init function
    -for each pixel: add to average
    -for each sector/led combo: calculate average and assign corresponding leddata


Idea:
For more efficient animations:
  Buffer picture (read from bmp) in converted format*
    -4 bytes per pixel, header byte 0xFF inserted*
    -Brightness adjusted*
    -Gamma corrected colors*
    -cache x,y coordinates for each sector-led combo. Only apply offset each animation step.*
    -use bmp pixel offset for animation => no need for coord conversion*
    (-Close bmp file before init function ends. No need to keep it open.)
    -Make sure the integer type used is 4 bytes

Video:
  leddata.h/.c: 
    -Alternative LDInitFromBmp: "bmp" info and data buffer should be provided by caller, only the pointer cached*
    (-Add LDUpdateImage, assumes cache already exists. Only updates image contents.)
    -Add alternative function for extracting led data (caller supplies bmp data)*
    -Add support for bmp buffer allocation/deallocation managed by caller.*
       Move bmp data deallocation to a separate method.*

Problem:
Innermost led looks wrong, at least in simulator*
  led values seem wrapped: led0 has last leds value. led1 has led0s value. led2 has led1s value. Etc.
    Led data pointer has skipped 4 bytes. Why??? Happens in transmission??? No!
      Server always reports failure to receive frame once, then seems to work. why? Second frame, Normal!
       Hmmmmm, not during video transmission....still the problem is there in sim even for video.
      Varför kommer det skitdata (som ej börjar på FF) på offset 7676 då man kör ldservertest och picclient med test_photo.bmp???????????????????
        Fel i byteordning. Fixat. 
  Find out if its a led data problem or a simulator problem.
    Simulator problem: setpen must be called after setcolor. Fixed.
simulator: background should be black*
simulator: window size should be bigger*


