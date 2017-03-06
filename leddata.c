#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <malloc.h>
#include <string.h>
#include "bmp.h"
#include "leddata.h"

#define Q_ASSERT(x) assert(x)
#define MYPI (3.14159265)

#define LED_DATA_SIZE 4 //bytes per LED

typedef struct LDSectorLedAreaColor
{
  int blue;
  int green;
  int red;
  int nof_pixels;
} LDSectorLedAreaColor;

typedef struct LDcache
{
  // BMP cache
  BITMAPHEADER bmh;
  char *pBuf;

  // geometry cache
  int nofSectors;
  int nofLeds;
  int *xCoords; //bmp pixel x coordinate for each sector/led combo
  int *yCoords; //bmp pixel y coordinate for each sector/led combo

  //raw led data value for each bmp pixel
  int *pCache;

  //gamma correction lookup table
  unsigned char gammaLUT[256];

  int blackValue;

  //pixel to sector/led map
  int *pPixelToLedMap;

  // Temporary storage for averaging color in each sector/led combo
  struct LDSectorLedAreaColor* pSectorLedAvg;
} LDCache;


static LDCache theLDCache = {
  {0},
  NULL,
  0,
  0,
  NULL,
  NULL,
  NULL,
  {0},
  0
};


//////////////////////////////////////////////////////////////
//
static void initBlack()
{
  unsigned char* pCacheChar;

  // Save int value for Black color in correct byte order
  pCacheChar = (unsigned char*)&theLDCache.blackValue;
  *pCacheChar++ = 0xFF; // header 
  *pCacheChar++ = 0x00; // blue
  *pCacheChar++ = 0x00; // green
  *pCacheChar++ = 0x00; // red
}

//////////////////////////////////////////////////////////////
//
void ConvertPicCoordsToBmpCoords(unsigned int maxxin, unsigned int maxyin,
                                 unsigned int xin, unsigned int yin,
                                 const BITMAPHEADER * const pHeader,
                                 unsigned int *pxout, unsigned int *pyout)
{
  Q_ASSERT(xin <= maxxin);
  Q_ASSERT(yin <= maxyin);

  *pxout = xin * (pHeader->Width - 1)/ maxxin;
  *pyout = yin * (pHeader->Height - 1) / maxyin;
}

//////////////////////////////////////////////////////////////
//
void ConvertPicCoordsToBmpCoordsNoBoundsCheck(
                             int maxxin, int maxyin,
                             int xin, int yin,
                             const BITMAPHEADER * const pHeader,
                             int *pxout, int *pyout)
{
  *pxout = xin * (pHeader->Width - 1)/ maxxin;
  *pyout = yin * (pHeader->Height - 1) / maxyin;
}


///////////////////////////////////////////////////////////////
//
void LDsetLedDataBlack(const int nofSectors,
                       const int nofLeds,
                       char * const pLeddataOut)
{
  int sector, led;
  int *pLedDataInt = (int*)pLeddataOut;

  initBlack();

  for(sector=0; sector<nofSectors; sector++)
  {
    for(led=0; led<nofLeds; led++)
    {
      *pLedDataInt++ = theLDCache.blackValue;
    }
  }

  
}

///////////////////////////////////////////////////////////////
//
void LDsetLed(const int nofLeds,
              const int sector,
              const int led,
              const unsigned char blue,
              const unsigned char green,
              const unsigned char red,
              char * const pLeddataOut)
{
  char *pLedData = pLeddataOut;

  pLedData += (sector * nofLeds + led) * 4;
  
  *pLedData++ = 0xFF; //header
  *pLedData++ = blue;
  *pLedData++ = green;
  *pLedData++ = red;

}

///////////////////////////////////////////////////////////////
//
int LDInitFromBmpData(char * const pBmpBuf,
                  const BITMAPHEADER * pBmpHeader,
                  const unsigned char brightness,
                  const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int gamma)
{
  int i, x, y, pixelvalue;
  unsigned char* pCacheChar;

  int sector, led;
  int aLen;
  int startAng;
  double startAngRad;
  int maxx, maxy;
  int xin, yin;
  int bmpx;
  int bmpy;
  int* pXCoord;
  int* pYCoord;

  Q_ASSERT(brightness > 0);


  theLDCache.pBuf = pBmpBuf;     // copy only the pointer
  theLDCache.bmh  = *pBmpHeader; // copy the whole struct

  /////////////////////////////////////////////////////////////////////////
  // Calculate gamma correction table, makes mid-range colors look 'right'

  for(i=0; i<256; i++)
  {
    theLDCache.gammaLUT[i] = (int)(pow((float)(i) / 255.0, 2.7) * 255.0 + 0.5);
//    printf("%d\n", gammaLUT[i]); //TEMP!!!
  }

  /////////////////////////////////////////////////////////////////////////
  // Create cache with precalculated led data values for each bmp pixel
  theLDCache.pCache = 
    (int*)malloc(theLDCache.bmh.Width *
                 theLDCache.bmh.Height * 
                 sizeof(int));
  if(!theLDCache.pCache)
  {
      //failed to allocate memory
      LDRelease();
      return 2;
  }

  // use a temporary char pointer to make sure the byte order is correct
  pCacheChar = (unsigned char*)theLDCache.pCache;

  for(x=0; x<theLDCache.bmh.Width; x++)
  {
    for(y=0; y<theLDCache.bmh.Height; y++)
    {
      pixelvalue = GetPixel(&theLDCache.bmh, theLDCache.pBuf, x, y);

      *pCacheChar++ = 0xFF; // header
      if(gamma)
      {
        *pCacheChar++ = (theLDCache.gammaLUT[(pixelvalue & 0x00FF0000) >> 16]  * brightness) / 255;  //blue
        *pCacheChar++ = (theLDCache.gammaLUT[(pixelvalue & 0x0000FF00) >>  8]  * brightness) / 255;  //green
        *pCacheChar++ = (theLDCache.gammaLUT[pixelvalue & 0x000000FF] * brightness) / 255;         //red
      }
      else
      {
        *pCacheChar++ = (((pixelvalue & 0x00FF0000) >> 16)  * brightness) / 255;  //blue
        *pCacheChar++ = (((pixelvalue & 0x0000FF00) >>  8)  * brightness) / 255;  //green
        *pCacheChar++ = ((pixelvalue & 0x000000FF) * brightness) / 255;         //red
      }
//      printf("0x%08X ", pixelvalue);
    }
  }

  // Save int value for Black color in correct byte order
  pCacheChar = (unsigned char*)&theLDCache.blackValue;
  *pCacheChar++ = 0xFF; // header 
  *pCacheChar++ = 0x00; // blue
  *pCacheChar++ = 0x00; // green
  *pCacheChar++ = 0x00; // red

  //////////////////////////////////////////////////////////////////////
  // Create geometry cache
  theLDCache.nofSectors = nofSectors;
  theLDCache.nofLeds = nofLeds;

  theLDCache.xCoords = (int*)malloc(nofSectors * nofLeds * sizeof(int));
  if(!theLDCache.xCoords)
  {
      //failed to allocate memory
      LDRelease();
      return 3;
  }

  theLDCache.yCoords = (int*)malloc(nofSectors * nofLeds * sizeof(int));
  if(!theLDCache.yCoords)
  {
      //failed to allocate memory
      LDRelease();
      return 3;
  }

  aLen = 5760 / nofSectors;

  // Set scale so the whole circular display fits in the square picture.
  // This means the corners of the picture are not visible
  // unless an offset is applied later.
  maxx = 2 * ledRadiusArray[nofLeds-1];
  maxy = 2 * ledRadiusArray[nofLeds-1];

  for(sector=0; sector<nofSectors; sector++)
  {
    startAng = aLen * sector + aLen/2;
    startAngRad = ((double)startAng / 5760) * 2 * MYPI;

    for(i=0; i<nofLeds; i++)
    {
        // caluculate coords with origo in center of POV display.
        // Use LED radius units (mm or whatever).
        x = cos((double)startAngRad) * ledRadiusArray[i];
        y = -sin((double)startAngRad) * ledRadiusArray[i];

        // convert to coords with origo in top left corner of image
        xin = x + maxx / 2;
        yin = y + maxy / 2;

        pXCoord = theLDCache.xCoords + sector * nofLeds + i;
        pYCoord = theLDCache.yCoords + sector * nofLeds + i;

        // convert to bmp file pixel coords
        // Note: coordinates could be outside the picture
        // (negative or >max)
        ConvertPicCoordsToBmpCoordsNoBoundsCheck(maxx, maxy, xin, yin, &theLDCache.bmh, &bmpx, &bmpy);
        *pXCoord = bmpx;
        *pYCoord = bmpy;          
    }
  }

  ///////////////////////////////////////////////////////////
  // Allocate and populate bmp pixel-to-sector/led map
  theLDCache.pPixelToLedMap =
    (int*)malloc(theLDCache.bmh.Width *
                 theLDCache.bmh.Height * 
                 sizeof(int));
  if(!theLDCache.pPixelToLedMap)
  {
      //failed to allocate memory
      LDRelease();
      return 3;
  }

  //Populate

  //assume even distance between leds and a square bmp image
  int sectorHeight = ledRadiusArray[1] - ledRadiusArray[0];
  
  //convert to height in bmp pixels
  sectorHeight = (sectorHeight * (theLDCache.bmh.Height /2)) / ledRadiusArray[nofLeds-1];

  for(x=0; x<theLDCache.bmh.Width; x++)
  {
    for(y=0; y<theLDCache.bmh.Height; y++)
    {
      ////Calculate sector/led combo

      // Calculate LED
      int squareDistanceFromOrigo = (x-(theLDCache.bmh.Width/2)) * (x-(theLDCache.bmh.Width/2)) +
                                    (y-(theLDCache.bmh.Height/2)) * (y-(theLDCache.bmh.Height/2));
      for(led=-1; led<(nofLeds-1);)
      {
        int ledRadiusInPixels = (ledRadiusArray[led+1] * (theLDCache.bmh.Height /2)) / ledRadiusArray[nofLeds-1];
        int lowbound = ledRadiusInPixels - sectorHeight/2;
        if(squareDistanceFromOrigo < (lowbound * lowbound))
        {
          break;
        }
        led++;
      }
      
      if(led == (nofLeds - 1))
      {
        int ledRadiusInPixels = theLDCache.bmh.Height /2;
        int hibound = ledRadiusInPixels + sectorHeight/2;
        if(squareDistanceFromOrigo > (hibound * hibound))
        {
          // outside outermost LED
          led++;
        }
      }

      //printf("%d", led);
      
      if((led >= nofLeds) || (led < 0))
      {
        //the pixel is outside the outermost LED or inside innermost
        theLDCache.pPixelToLedMap[x * theLDCache.bmh.Height + y] = -1; //magic value
      }
      else
      {
        // Calculate sector
        double angle = acos((x - (int)theLDCache.bmh.Width/2) /
                            sqrt(squareDistanceFromOrigo));

/*        if(led == 10) printf("%d %f %f %f ", x - theLDCache.bmh.Width/2,
                            sqrt(squareDistanceFromOrigo),
                             (x - (int)theLDCache.bmh.Width/2) /
                             sqrt(squareDistanceFromOrigo),
                             angle);*/
        if(y > (theLDCache.bmh.Height/2))
        {
          // lower half
          angle = 2 * M_PI - angle;
        }
//        if(led == 10) printf("%f\n", angle);
        sector = (int)(angle * nofSectors/ (2 * M_PI));
        if(sector > nofSectors)
        {
          sector = 0;
        }

        // Cache the result
        theLDCache.pPixelToLedMap[x * theLDCache.bmh.Height + y] = 
          sector * nofLeds + led;
      }

      //printf("%d ", theLDCache.pPixelToLedMap[x * theLDCache.bmh.Height + y]);
    }
    //printf("\n");
  }

  //////////////////////////////////////////////////////////////////////////
  // Allocate reusable storage for averaging color in each sector/led combo.
  // To be populated when extracting LED data.
  theLDCache.pSectorLedAvg = (struct LDSectorLedAreaColor*)
    malloc(nofSectors * nofLeds * sizeof(struct LDSectorLedAreaColor));
  if(!theLDCache.pSectorLedAvg)
  {
      //failed to allocate memory
      LDRelease();
      return 4;
  }

  return 0;
}

///////////////////////////////////////////////////////////////
//
int LDInitFromBmp(const char * const pszFileName,
                  const unsigned char brightness,
                  const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int gamma)
{

  //////////////////////
  // Open bmp file
  if(OpenBmp(pszFileName, &theLDCache.pBuf, &theLDCache.bmh))
  {
    //failed to open bmp file
    printf("Failed to open bmp file %s\n", pszFileName);
    theLDCache.pBuf = NULL;
    return 1;
  }

  return LDInitFromBmpData(theLDCache.pBuf,
                    &theLDCache.bmh,
                    brightness,
                    nofSectors,
                    nofLeds,
                    ledRadiusArray,
                    gamma);
}

///////////////////////////////////////////////////////////////
// Uses averaging over each sector/led combo
void LDgetLedDataFromBmpData3(const char * const pBmpBuf,
                             const unsigned char brightness,
                             char * const pLeddataOut,
                             const int yflip,
                             const int gamma)
{
  unsigned char* pLed;

  int sector, led;
  int x, y, yflipped; //bmp coordinates
  int pixelvalue;
  int sectorLedIndex;
  int ledvalue;
  struct LDSectorLedAreaColor *pLedAvg;

  Q_ASSERT(theLDCache.pPixelToLedMap);
  Q_ASSERT(theLDCache.pSectorLedAvg);

  memset(theLDCache.pSectorLedAvg, 0,
           theLDCache.nofSectors * theLDCache.nofLeds * sizeof(theLDCache.pSectorLedAvg[0]));

  // Accumulate color values
  for(x=0; x<theLDCache.bmh.Width; x++)
  {
    for(y=0; y<theLDCache.bmh.Height; y++)
    {
      if(yflip)
      {
        yflipped = theLDCache.bmh.Height - y - 1;
      }
      else
      {
        yflipped = y;
      }

      sectorLedIndex = theLDCache.pPixelToLedMap[x * theLDCache.bmh.Height + yflipped];

      //printf("sectorLedIndex:%d\n", sectorLedIndex);

      if(sectorLedIndex != -1) // check magic value
      {
        Q_ASSERT(sectorLedIndex >= 0);
        Q_ASSERT(sectorLedIndex < (theLDCache.nofSectors * theLDCache.nofLeds));

        pixelvalue = GetPixel(&theLDCache.bmh, pBmpBuf, x, y);
        theLDCache.pSectorLedAvg[sectorLedIndex].blue  += (pixelvalue & 0x00FF0000) >> 16;
        theLDCache.pSectorLedAvg[sectorLedIndex].green += (pixelvalue & 0x0000FF00) >> 8;
        theLDCache.pSectorLedAvg[sectorLedIndex].red   += (pixelvalue & 0x000000FF);
        theLDCache.pSectorLedAvg[sectorLedIndex].nof_pixels++;
      }
    }

  }

  // Calculate averages
  pLed = pLeddataOut;
  pLedAvg = theLDCache.pSectorLedAvg;

  int bluevalue, greenvalue, redvalue;

  for(sectorLedIndex = 0; sectorLedIndex < (theLDCache.nofSectors * theLDCache.nofLeds); sectorLedIndex++)
  {
    *pLed++ = 0xFF; //Header

    /*if( (sectorLedIndex / theLDCache.nofLeds) == 10)
    {
      printf("%d ", pLedAvg->nof_pixels);
      if( sectorLedIndex == 353)
      {
        printf("\n");
      }
    }*/
    
    if(pLedAvg->nof_pixels > 0)
    {
      bluevalue  = pLedAvg->blue  / pLedAvg->nof_pixels;
      greenvalue = pLedAvg->green / pLedAvg->nof_pixels;
      redvalue   = pLedAvg->red   / pLedAvg->nof_pixels;

      Q_ASSERT(bluevalue < 256);
      Q_ASSERT(greenvalue < 256);
      Q_ASSERT(redvalue < 256);
    
      // Gamma correction and brightness adjustment
      if(gamma)
      {
        *pLed++ = (theLDCache.gammaLUT[bluevalue] * brightness) / 255;
        *pLed++ = (theLDCache.gammaLUT[greenvalue] * brightness) / 255;
        *pLed++ = (theLDCache.gammaLUT[redvalue] * brightness) / 255;
      }
      else
      {
        *pLed++ = (bluevalue * brightness) / 255;
        *pLed++ = (greenvalue * brightness) / 255;
        *pLed++ = (redvalue * brightness) / 255;
      }
    }
    else
    {
      // black: set all 3 color components to zero
      *pLed++ = 0;
      *pLed++ = 0;
      *pLed++ = 0;
      
//      printf("no pixels! %d\n", sectorLedIndex);
    }

    pLedAvg++;
  }


}

///////////////////////////////////////////////////////////////
//
void LDgetLedDataFromBmpData(const char * const pBmpBuf,
                             const unsigned char brightness,
                             char * const pLeddataOut,
                             const int yflip,
                             const int gamma)
{
  unsigned char* pChar;

  int sector, led;
  int bmpx, bmpy;
  int pixelvalue;

  Q_ASSERT(theLDCache.xCoords);
  Q_ASSERT(theLDCache.yCoords);


  pChar = pLeddataOut;

  for(sector=0; sector<theLDCache.nofSectors; sector++)
  {
    for(led=0; led<theLDCache.nofLeds; led++)
    {
      bmpx = *(theLDCache.xCoords + sector * theLDCache.nofLeds + led);
      bmpy = *(theLDCache.yCoords + sector * theLDCache.nofLeds + led);
      if(yflip)
      {
        bmpy = theLDCache.bmh.Height - bmpy - 1;
      }

      pixelvalue = GetPixel(&theLDCache.bmh, pBmpBuf, bmpx, bmpy);

      *pChar++ = 0xFF; // header
      if(gamma)
      {
        *pChar++ = (theLDCache.gammaLUT[(pixelvalue & 0x00FF0000) >> 16]  * brightness) / 255;  //blue
        *pChar++ = (theLDCache.gammaLUT[(pixelvalue & 0x0000FF00) >>  8]  * brightness) / 255;  //green
        *pChar++ = (theLDCache.gammaLUT[pixelvalue & 0x000000FF] * brightness) / 255;         //red
      }
      else
      {
        *pChar++ = (((pixelvalue & 0x00FF0000) >> 16)  * brightness) / 255;  //blue
        *pChar++ = (((pixelvalue & 0x0000FF00) >>  8)  * brightness) / 255;  //green
        *pChar++ = ((pixelvalue & 0x000000FF) * brightness) / 255;         //red
      }
    }
  }

}

///////////////////////////////////////////////////////////////
//
void LDgetLedData2(const int xbmpoffset,  //bmp pixels
                   const int ybmpoffset,  //bmp pixels
                   char * const pLeddataOut)
{
  int sector, led;
  int *pLedDataInt;
  int bmpx, bmpy;

  Q_ASSERT(theLDCache.pCache);
  Q_ASSERT(theLDCache.xCoords);
  Q_ASSERT(theLDCache.yCoords);

  pLedDataInt = (int*)pLeddataOut;

  for(sector=0; sector<theLDCache.nofSectors; sector++)
  {
    for(led=0; led<theLDCache.nofLeds; led++)
    {
      bmpx = *(theLDCache.xCoords + sector * theLDCache.nofLeds + led) 
             + xbmpoffset;
      bmpy = *(theLDCache.yCoords + sector * theLDCache.nofLeds + led) 
             + ybmpoffset;

      if((bmpx <= theLDCache.bmh.Width) && (bmpy <= theLDCache.bmh.Height)
          && (bmpx >= 0) && (bmpy >= 0))
      {
        *pLedDataInt = *(theLDCache.pCache + bmpx * theLDCache.bmh.Height + bmpy);
      }
      else
      {
        *pLedDataInt = theLDCache.blackValue; //all black
        //printf("ooops! s:%d l:%d x:%d y:%d offset:%d\n", sector, led, bmpx, bmpy, (sector * theLDCache.nofLeds + led)*4); ///TEMP!!!!!!!!!!!!!!!!!!!!!!!
      }

      pLedDataInt++;
    }
  }
}


///////////////////////////////////////////////////////////////
//
void LDgetLedData(const int nofSectors,
                  const int nofLeds,
                  const int *ledRadiusArray,
                  const int xoffset,
                  const int yoffset,
                  char * const pLeddataOut)
{
  int sector;
  int i;
  int aLen;
  int startAng;
  double startAngRad;
  int x;
  int y;
  int maxx, maxy;
  int xin, yin;
  unsigned int bmpx;
  unsigned int bmpy;
  int ledstate;
  int ledoffset;
//  const int sectordatasize = nofLeds * LED_DATA_SIZE; //bytes
  int *pLedDataInt;

  Q_ASSERT(theLDCache.pBuf);
  Q_ASSERT(theLDCache.pCache);


  //////////////////////
  // Calculate led data
  aLen = 5760 / nofSectors;

    // Set scale so the whole square picture fits in the circular display
    maxx = 2 * ledRadiusArray[nofLeds-1];
    maxy = 2 * ledRadiusArray[nofLeds-1];

    pLedDataInt = (int*)pLeddataOut;

    for(sector=0; sector<nofSectors; sector++)
    {
        startAng = aLen * sector + aLen/2;
        startAngRad = ((double)startAng / 5760) * 2 * MYPI;

        for(i=0; i<nofLeds; i++)
        {
            // caluculate coords with origo in center of POV display.
            // Use LED radius units (mm or whatever).
            x = cos((double)startAngRad) * ledRadiusArray[i]  - xoffset;
            y = -sin((double)startAngRad) * ledRadiusArray[i] + yoffset;

            // convert to coords with origo in top left corner of image
            xin = x + maxx / 2; // + anim_offset_x;
            yin = y + maxy / 2; // + anim_offset_y;

            // convert to bmp file pixel coords and get color
            if((xin <= maxx) && (yin <= maxy) && (xin > 0) && (yin > 0))
            {
              ConvertPicCoordsToBmpCoords(maxx, maxy, xin, yin, &theLDCache.bmh, &bmpx, &bmpy);
              *pLedDataInt = *(theLDCache.pCache + bmpx * theLDCache.bmh.Height + bmpy);
//              *pLedDataInt = *pCache;
              //ledstate = GetPixel(&bmh, pBuf, bmpx, bmpy);
            }
            else
            {
              *pLedDataInt = theLDCache.blackValue; //all black
               //printf("ooops again! s:%d l:%d x:%d y:%d offset:%d\n", sector, i, xin, yin, (sector * theLDCache.nofLeds + i)*4); ///TEMP!!!!!!!!!!!!!!!!!!!!!!!
              //ledstate = 0;  //old slower implementation
            }
            
//            printf("p0x%X:", (int)pLedDataInt);
//            printf("0x%X ", *pLedDataInt);

            pLedDataInt++;

            /*
            //convert color value to Dotstar format
            ledoffset = (sector * sectordatasize) + (i * LED_DATA_SIZE);
            pLeddataOut[ledoffset] = 0xFF; //header
            pLeddataOut[ledoffset + 1] = 
              gammaLUT[(((ledstate & 0x00FF0000) >> 16) * brightness) / 255];  //blue
            pLeddataOut[ledoffset + 2] =
              gammaLUT[(((ledstate & 0x0000FF00) >> 8)  * brightness) / 255];  //green
            pLeddataOut[ledoffset + 3] =
              gammaLUT[((ledstate & 0x000000FF)  * brightness) / 255];         //red
            */
          
        }
    }

}


///////////////////////////////////////////////////////
//
void LDRelease()
{
  printf("%s: cleaning up\n", __FILE__);

  if(theLDCache.pCache)
  {
    free(theLDCache.pCache);
    theLDCache.pCache = NULL;
  }

  if(theLDCache.xCoords)
  {
    free(theLDCache.xCoords);
    theLDCache.xCoords = NULL;
  }

  if(theLDCache.yCoords)
  {
    free(theLDCache.yCoords);
    theLDCache.yCoords = NULL;
  }
}

///////////////////////////////////////////////////////
//
void LDReleaseBmpData()
{
  printf("%s: cleaning up BMP data\n", __FILE__);

  if(theLDCache.pBuf)
  {
    ReleaseBmp(&theLDCache.pBuf);
    theLDCache.pBuf = NULL;
  }
}

