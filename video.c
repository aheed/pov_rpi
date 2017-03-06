/*
$ gcc ldclient.c bmp.c leddata.c video.c -o video -lavformat -lavcodec -lavutil -lswscale -lm
$ ./video ~/Videos/WILDLIFE\ IN\ 4K\ \(ULTRA\ HD\).mp4
*/


#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
//#include <ffmpeg/swscale.h>
#include <libswscale/swscale.h>

#include <stdio.h>
#include <time.h>
#include "leddata.h"
#include "ldclient.h"
#include "povgeometry.h"

// compatibility with newer API
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
#define av_frame_alloc avcodec_alloc_frame
#define av_frame_free avcodec_free_frame
#endif

#define OUTPUT_WIDTH  512
#define OUTPUT_HEIGHT 512

//------------------------------------------
// POV stuff
#define NOF_SECTORS 120
#define NOF_LEDS 32
#define MAX_BRIGHTNESS 255 //64

#define SLEEP_PER_FRAME 10000000 // nanoseconds

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

static uint8_t leddata[NOF_SECTORS * NOF_LEDS * 4] = {0};

//-------------------------------------

void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame) {
  FILE *pFile;
  char szFilename[32];
  int  y;
  
  // Open file
  sprintf(szFilename, "frame%d.ppm", iFrame);
  pFile=fopen(szFilename, "wb");
  if(pFile==NULL)
    return;
  
  // Write header
  fprintf(pFile, "P6\n%d %d\n255\n", width, height);
  
  // Write pixel data
  for(y=0; y<height; y++)
    fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
  
  // Close file
  fclose(pFile);
}

int main(int argc, char *argv[]) {
  // Initalizing these to NULL prevents segfaults!
  AVFormatContext   *pFormatCtx = NULL;
  int               i, videoStream;
  AVCodecContext    *pCodecCtxOrig = NULL;
  AVCodecContext    *pCodecCtx = NULL;
  AVCodec           *pCodec = NULL;
  AVFrame           *pFrame = NULL;
  AVFrame           *pFrameRGB = NULL;
  AVPacket          packet;
  int               frameFinished;
  int               numBytes;
  uint8_t           *buffer = NULL;
  struct SwsContext *sws_ctx = NULL;
  int               connected = 0;
  struct timespec sleeper, dummy;

  struct BITMAPHEADER bmh = {0};

  if(argc < 2) {
    printf("Please provide a movie file\n");
    return -1;
  }
  // Register all formats and codecs
  av_register_all();
  
  // Open video file
  if(avformat_open_input(&pFormatCtx, argv[1], NULL, NULL)!=0)
    return -1; // Couldn't open file
  
  // Retrieve stream information
  if(avformat_find_stream_info(pFormatCtx, NULL)<0)
    return -1; // Couldn't find stream information
  
  // Dump information about file onto standard error
  av_dump_format(pFormatCtx, 0, argv[1], 0);
  
  // Find the first video stream
  videoStream=-1;
  for(i=0; i<pFormatCtx->nb_streams; i++)
    if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
      videoStream=i;
      break;
    }
  if(videoStream==-1)
    return -1; // Didn't find a video stream
  
  // Get a pointer to the codec context for the video stream
  pCodecCtxOrig=pFormatCtx->streams[videoStream]->codec;
  // Find the decoder for the video stream
  pCodec=avcodec_find_decoder(pCodecCtxOrig->codec_id);
  if(pCodec==NULL) {
    fprintf(stderr, "Unsupported codec!\n");
    return -1; // Codec not found
  }
  // Copy context
  pCodecCtx = avcodec_alloc_context3(pCodec);
  if(avcodec_copy_context(pCodecCtx, pCodecCtxOrig) != 0) {
    fprintf(stderr, "Couldn't copy codec context");
    return -1; // Error copying codec context
  }

  // Open codec
  if(avcodec_open2(pCodecCtx, pCodec, NULL)<0)
    return -1; // Could not open codec
  
  // Allocate video frame
  pFrame=av_frame_alloc();
  
  // Allocate an AVFrame structure
  pFrameRGB=av_frame_alloc();
  if(pFrameRGB==NULL)
    return -1;

  // Determine required buffer size and allocate buffer
  numBytes=avpicture_get_size(PIX_FMT_BGR24, OUTPUT_WIDTH,
			      OUTPUT_HEIGHT);
  buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));
  
  // Assign appropriate parts of buffer to image planes in pFrameRGB
  // Note that pFrameRGB is an AVFrame, but AVFrame is a superset
  // of AVPicture
  avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
		 OUTPUT_WIDTH, OUTPUT_HEIGHT);
  
  // initialize SWS context for software scaling
  sws_ctx = sws_getContext(pCodecCtx->width,
			   pCodecCtx->height,
			   pCodecCtx->pix_fmt,
			   OUTPUT_WIDTH,
			   OUTPUT_HEIGHT,
			   PIX_FMT_BGR24,
			   SWS_BILINEAR,
			   NULL,
			   NULL,
			   NULL
			   );

/*  //-----------------------------------
  if(LDconnect("127.0.0.1"))
  {
    return 1;
  }
 
  //-----------------------------------*/

  // Read frames and save first five frames to disk
  i=0;
//  while((av_read_frame(pFormatCtx, &packet)>=0) && (i < 500)) {
  while((av_read_frame(pFormatCtx, &packet)>=0)) {
    // Is this a packet from the video stream?
    if(packet.stream_index==videoStream) {
      // Decode video frame
      avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);
      
      // Did we get a video frame?
      if(frameFinished) {
	// Convert the image from its native format to RGB
	sws_scale(sws_ctx, (uint8_t const * const *)pFrame->data,
		  pFrame->linesize, 0, pCodecCtx->height,
		  pFrameRGB->data, pFrameRGB->linesize);

        // prepare LED data for POV
        if(i == 0)
        {
          bmh.Width = OUTPUT_WIDTH;
          bmh.Height = OUTPUT_HEIGHT;
          bmh.BitsPerPixel = 24;

          if(LDInitFromBmpData(pFrameRGB->data[0],
                  &bmh,
                  MAX_BRIGHTNESS,
                  NOF_SECTORS,
                  NOF_LEDS,
                  ledRadius,
                  0))
           {
             fprintf(stderr, "Failed to init LED data\n");
             return 1;
           }
        }
        
        LDgetLedDataFromBmpData3(pFrameRGB->data[0],
                             MAX_BRIGHTNESS,
                             leddata,
                             1,
                             0);
        /*
        LDgetLedDataFromBmpData(pFrameRGB->data[0],
                             MAX_BRIGHTNESS,
                             leddata,
                             1,
                             0);*/

        //Transmit LED data to POV server
        if(!connected)
        {
          LDconnect("127.0.0.1");
          connected = 1;
        }

        LDTransmit(leddata, POV_FRAME_SIZE);
        if(LDWaitforAck())
        {
          printf("got no ack\n");
          LDDisconnect();
          connected = 0;
        }

        ++i;

        if(SLEEP_PER_FRAME > 0)
        {
          // wait a while
          sleeper.tv_sec  = 0;
          sleeper.tv_nsec = SLEEP_PER_FRAME;
          nanosleep(&sleeper, &dummy);
          //sleep(2); //temp
        }
	
	// Save the frame to disk
        /*
	if(i<=5)
	  SaveFrame(pFrameRGB, OUTPUT_WIDTH, OUTPUT_HEIGHT, 
		    i);*/

      }
    }
    
    // Free the packet that was allocated by av_read_frame
    av_free_packet(&packet);
  }
  
  // Free the RGB image
  av_free(buffer);
  av_frame_free(&pFrameRGB);
  
  // Free the YUV frame
  av_frame_free(&pFrame);
  
  // Close the codecs
  avcodec_close(pCodecCtx);
  avcodec_close(pCodecCtxOrig);

  // Close the video file
  avformat_close_input(&pFormatCtx);
  
  return 0;
}
