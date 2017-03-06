/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtGui>

#include "/home/anders/Public/src/pov_rpi/bmp.h"
#include "renderarea.h"
#include "/home/anders/Public/src/pov_rpi/povgeometry.h"
#include "/home/anders/Public/src/pov_rpi/ldserver.h"

#ifndef __cplusplus
#define __cplusplus
#endif

////////////////////////////////////////////////////////////
#define NOF_LEDS 32 //10 //4
#define NOF_SECTORS 120 //120 //60
#define MYPI 3.14159265

#define MULTI_COLOR

const int ledRadius[NOF_LEDS] =
#if NOF_LEDS == 4
//  {210, 290, 370, 450};
  {105, 145, 185, 225};
#elif NOF_LEDS == 10
//  {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
  {50, 70, 90, 110, 130, 150, 170, 190, 210, 230};
#elif NOF_LEDS == 20
//  {100, 140, 180, 220, 260, 300, 340, 380, 420, 460};
  {50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240};
#elif NOF_LEDS == 32
  {50, 56, 62, 68, 74, 80, 86, 92, 98, 104, 110, 116, 122, 128, 134, 140, 146, 152, 158, 164, 170, 176, 182, 188, 194, 200, 206, 212, 218, 224, 230 ,236};
#endif

//const int xc = 280;
//const int yc = 280;
const int xc = 500;
const int yc = 450;

static QPoint mypoints[NOF_LEDS * NOF_SECTORS];

const float povLedScale = 0.17;

/*
static char led_states[NOF_LEDS][NOF_SECTORS/8+1] =
{


//// Assuming NOF_SECTORS=60
// {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1, 0,0,0,0, zero...},
 {0x84, 0x21, 0x00, 0, 0, 0, 0, 0},

// {0,1,0,0, 0,0,1,0, 0,0,0,1, 0,0,0,0, 1,0,0,0, zero...},
 {0x42, 0x10, 0x80, 0, 0, 0, 0, 0},

// {0,0,1,0, 0,0,0,1, 0,0,0,0, 1,0,0,0, 0,1,0,0, zero...},
 {0x21, 0x08, 0x40, 0, 0, 0, 0, 0},

// {0,0,0,1, 0,0,0,0, 1,0,0,0, 0,1,0,0, 0,0,1,0, zero...},
 {0x10, 0x84, 0x20, 0, 0, 0, 0, 0}

};
*/


static char led_states[NOF_LEDS][NOF_SECTORS/8+1] =
#if NOF_LEDS == 4
{
{0x80, 0x22, 0x22, 0x08, 0x00, 0x1f, 0x1e, 0x30},
{0x20, 0x22, 0x22, 0x08, 0x00, 0x10, 0x12, 0x20},
{0x20, 0x22, 0x22, 0x00, 0x00, 0x11, 0x12, 0x20},
{0xc1, 0xe3, 0xe3, 0xc8, 0x00, 0x1f, 0x1e, 0x30}
};
#elif NOF_LEDS == 10
{
{0x80, 0x22, 0x22, 0x08, 0x00, 0x1f, 0x1e, 0x30},
{0x20, 0x22, 0x22, 0x08, 0x00, 0x10, 0x12, 0x20},
{0x20, 0x22, 0x22, 0x00, 0x00, 0x11, 0x12, 0x20},
{0xc1, 0xe3, 0xe3, 0xc8, 0x00, 0x1f, 0x1e, 0x30},
{0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
{0x80, 0x22, 0x22, 0x08, 0x00, 0x1f, 0x1e, 0x30},
{0x20, 0x22, 0x22, 0x08, 0x00, 0x10, 0x12, 0x20},
{0x20, 0x22, 0x22, 0x00, 0x00, 0x11, 0x12, 0x20},
{0xc1, 0xe3, 0xe3, 0xc8, 0x00, 0x1f, 0x1e, 0x30},
{0x0, 0x0, 0x0, 0x0, 0x00, 0x0, 0x0, 0x0}
};
#else  //elif NOF_LEDS == 20
{0};
#endif

#ifdef MULTI_COLOR
// assume 24 bit colors
static int color_states[NOF_SECTORS][NOF_LEDS] = {0};
#endif

#define ANIM_INTERVAL 100 //msec

int anim_offset_x = 0;
int anim_offset_y = 0;

int anim_dir_x = 12;
int anim_dir_y = 2;

BITMAPHEADER bmh;
char *pBuf = NULL;

///////////////////////////////////////////////////////////

// helper function
void savedata()
{
    FILE *outfile;
    int sector;
    int shiftbits;
    int i;
    int aLen;
    int startAng;
    int supersector;

    outfile = fopen("/home/anders/src/sim/spindat.txt", "wt");
    if(!outfile)
    {
        // printf("Failed to open file %s for writing\n", pszFile);
        return;
    }

    fprintf(outfile, "static char led_states[NOF_LEDS][NOF_SECTORS/8+1] =\n{\n");

    aLen = 5760 / NOF_SECTORS;

    for(i=0; i<NOF_LEDS; i++)
    {
        fprintf(outfile, "{");

        for(sector=0; sector<(NOF_SECTORS/8+1); sector++)
        {
            startAng = aLen * sector;

            shiftbits = 7 - sector % 8;  //[0,7]
            supersector = sector / 8; //[0, NOF_SECTORS/8]

            fprintf(outfile, "0x%02x", (unsigned char)led_states[i][sector]);

            if(sector < (NOF_SECTORS/8))
            {
                fprintf(outfile, ", ");
            }
        }

        fprintf(outfile, "}");
        if(i < (NOF_LEDS-1))
        {
            fprintf(outfile, ",");
        }
        fprintf(outfile, "\n");
    }

    fprintf(outfile, "};");

    fclose(outfile);
}

//////////////////////////////////////////////////////////////77
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

void SetLedStates()
{
    //////////////////////////////////////////////////////////////////


//    const char *bmpfilename = "/home/anders/src/sim/test.bmp";
    const char *bmpfilename = "/home/anders/Public/src/pov_rpi/sandra3.bmp";

    if(!pBuf)
    {
      if(OpenBmp(bmpfilename, &pBuf, &bmh))
      {
          //failed to open bmp file
          printf("Failed to openbmp file %s\n", bmpfilename);
          pBuf = NULL;
      }
    }


    //////////////////////////////////////////////////////////////////

//--------------------------------------------------
    int sector;
    int shiftbits;
    int i;
    int aLen;
    int startAng;
    double startAngRad;
    int supersector;
    int x;
    int y;
    int maxx, maxy;
    int xin, yin;
    unsigned int bmpx;
    unsigned int bmpy;
//    char ledstate;
    int ledstate;


    aLen = 5760 / NOF_SECTORS;

    maxx = 2 * ledRadius[NOF_LEDS-1];
    maxy = 2 * ledRadius[NOF_LEDS-1];

    for(sector=0; sector<NOF_SECTORS; sector++)
    {
        startAng = aLen * sector + aLen/2;

        shiftbits = 7 - sector % 8;  //[0,7]
        supersector = sector / 8; //[0, NOF_SECTORS/8]

        startAngRad = ((double)startAng / 5760) * 2 * MYPI;

        for(i=0; i<NOF_LEDS; i++)
        {
            x = xc + cos((double)startAngRad) * ledRadius[i];
            y = yc - sin((double)startAngRad) * ledRadius[i];

            mypoints[sector*NOF_LEDS + i].setX(x);
            mypoints[sector*NOF_LEDS + i].setY(y);

            xin = x - xc + ledRadius[NOF_LEDS-1] + anim_offset_x;
            yin = y - yc + ledRadius[NOF_LEDS-1] + anim_offset_y;

            if((xin <= maxx) && (yin <= maxy) && (xin > 0) && (yin > 0))
            {

//            ConvertPicCoordsToBmpCoords(xc + ledRadius[NOF_LEDS-1], yc + ledRadius[NOF_LEDS-1], x, y, &bmh, &bmpx, &bmpy);
              ConvertPicCoordsToBmpCoords(maxx, maxy, xin, yin, &bmh, &bmpx, &bmpy);

              if(pBuf)
              {

                //ledstate = (GetPixel(&bmh, pBuf, bmpx, bmpy) != 0);
                ledstate = GetPixel(&bmh, pBuf, bmpx, bmpy);
              }
              else
              {
                ledstate = (led_states[i][supersector] >> shiftbits) & 0x01;
              }
            }
            else
            {
              ledstate = 0;
            }

            if(!ledstate)
            {
                //reset bit
                led_states[i][supersector] &= ~(0x01 << shiftbits);
            }
            else
            {
                //set bit
                led_states[i][supersector] |= (0x01 << shiftbits);
            }
#ifdef MULTI_COLOR
            color_states[sector][i] = ledstate;
//            printf("ledstate:0x%X\n", ledstate);
#endif

        }
    }
//--------------------------------------------------
    /*
    if(pBuf)
    {
      ReleaseBmp(&pBuf);
      pBuf = NULL;
    }*/


}


//! [0]
RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    shape = Chord; //Polygon;
    antialiased = false;
    transformed = false;
    anim_offset_x = 0;
    anim_offset_y = 0;
    pixmap.load(":/images/qt-logo.png");

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    SetLedStates();
    startTimer(ANIM_INTERVAL);

    m_pLedDataBuf = (char*)malloc(POV_FRAME_SIZE);
    if(LDListen(m_pLedDataBuf, POV_FRAME_SIZE))
    {
      qDebug() << "Failed to launch POV listener!";
    }

    update();
}
//! [0]


void RenderArea::timerEvent(QTimerEvent *event)
{
   const int max_offset = 120;

   if( (anim_offset_x > max_offset) || (anim_offset_x < -max_offset) )
   {
     anim_dir_x *= -1;
   }
   anim_offset_x += anim_dir_x;

   if( (anim_offset_y > max_offset) || (anim_offset_y < -max_offset) )
   {
     anim_dir_y *= -1;
   }
   anim_offset_y += anim_dir_y;


//   qDebug() << "Timer ID:" << event->timerId() << " anim:" << anim << "offset:" << anim_offset_x << " " << anim_offset_y;

   if(LDGetReceivedFrames() == 0)
   {
     SetLedStates();
   }
   update();
}

void RenderArea::mousePressEvent ( QMouseEvent * event )
{

    int sector;
    int shiftbits;
    int i;
    int aLen;
    int startAng;
    double startAngRad;
    int supersector;
    int x;
    int y;


    aLen = 5760 / NOF_SECTORS;

    for(sector=0; sector<NOF_SECTORS; sector++)
    {
        startAng = aLen * sector + aLen/2;

        shiftbits = 7 - sector % 8;  //[0,7]
        supersector = sector / 8; //[0, NOF_SECTORS/8]

        startAngRad = ((double)startAng / 5760) * 2 * MYPI;

        for(i=0; i<NOF_LEDS; i++)
        {
            /*x = cos((double)startAngRad) * (ledRadius[i] / 2);
            y = sin((double)startAngRad) * (ledRadius[i] / 2);

            x = x + 40 + 10 + ledRadius[0];// - ledRadius[i]/4;
            y = y - 40 + 20 + ledRadius[0];
            y = 460 - y;*/

            x = xc + cos((double)startAngRad) * ledRadius[i];
            y = yc - sin((double)startAngRad) * ledRadius[i];

            /*mypoints[sector*NOF_LEDS + i].setX(x);
            mypoints[sector*NOF_LEDS + i].setY(y);*/

            if((abs(x-event->x()) < 5) && (abs(y-event->y()) < 5))
            {

                if((led_states[i][supersector] >> shiftbits) & 0x01)
                {
                    //reset bit
                    led_states[i][supersector] &= ~(0x01 << shiftbits);
                }
                else
                {
                    //set bit
                    led_states[i][supersector] |= (0x01 << shiftbits);
                }
            }

        }
    }

  update();
  savedata();
}



//! [1]
QSize RenderArea::minimumSizeHint() const
{
    return QSize(100, 100);
}
//! [1]

//! [2]
QSize RenderArea::sizeHint() const
{
//    return QSize(400, 200);
        return QSize(600, 600);
}
//! [2]

//! [3]
void RenderArea::setShape(Shape shape)
{
    this->shape = shape;
    update();
}
//! [3]

//! [4]
void RenderArea::setPen(const QPen &pen)
{
    this->pen = pen;
    update();
}
//! [4]

//! [5]
void RenderArea::setBrush(const QBrush &brush)
{
    this->brush = brush;
    update();
}
//! [5]

//! [6]
void RenderArea::setAntialiased(bool antialiased)
{
    this->antialiased = antialiased;
    update();
}
//! [6]

//! [7]
void RenderArea::setTransformed(bool transformed)
{
    this->transformed = transformed;
    update();
}
//! [7]


//! [8]
void RenderArea::paintEvent(QPaintEvent * /* event */)
{
    static const QPoint points[4] = {
        QPoint(10, 80),
        QPoint(20, 10),
        QPoint(80, 30),
        QPoint(90, 70)
    };

    //QRect rect(10, 20, 280, 280);
    QRect rect(10, 20, 1000, 1000);

    QPainterPath path;
    path.moveTo(20, 80);
    path.lineTo(20, 30);
    path.cubicTo(80, 0, 50, 50, 80, 80);

    int startAngle = 20 * 16;
    int arcLength = 120 * 16;
//! [8]

//! [9]
    QPainter painter(this);
    painter.setPen(pen);
    painter.setBrush(brush);
    if (antialiased)
        painter.setRenderHint(QPainter::Antialiasing, true);
//! [9]

//! [10]
//    for (int x = 0; x < width(); x += 100) {
      for (int x = 0; x < 1; x += 100) {
//        for (int y = 0; y < height(); y += 100) {
        for (int y = 0; y < 1; y += 100) {
            painter.save();
//            painter.translate(x, y);
//            painter.translate(40, 40);
//! [10] //! [11]
            if (transformed) {
                painter.translate(50, 50);
                painter.rotate(60.0);
                painter.scale(0.6, 0.9);
                painter.translate(-50, -50);
            }
//! [11]

//! [12]
            switch (shape) {
            case Line:
                painter.drawLine(rect.bottomLeft(), rect.topRight());
                break;
            case Points:
                painter.drawPoints(points, 4);
                break;
            case Polyline:
                painter.drawPolyline(points, 4);
                break;
            case Polygon:
                painter.drawPolygon(points, 4);
                break;
            case Rect:
                painter.drawRect(rect);
                break;
            case RoundedRect:
                painter.drawRoundedRect(rect, 25, 25, Qt::RelativeSize);
                break;
            case Ellipse:
                painter.drawEllipse(rect);
                break;
            case Arc:
                painter.drawArc(rect, startAngle, arcLength);
                break;
            case Chord:
////                painter.drawChord(rect, startAngle, arcLength);
                {
                    int sector;
                    int shiftbits;
                    int i;
                    int aLen;
                    int startAng;
                    int supersector;
                    int tmpwidth;
                    unsigned char* pChar = (unsigned char*)m_pLedDataBuf;


                    aLen = 5760 / NOF_SECTORS;

                    painter.drawRect(rect);

                    pen.setWidth(60 * povLedScale);

                    for(sector=0; sector<NOF_SECTORS; sector++)
                    {
                        startAng = aLen * sector;

                        shiftbits = 7 - sector % 8;  //[0,7]
                        supersector = sector / 8; //[0, NOF_SECTORS/8]

                        for(i=0; i<NOF_LEDS; i++)
                        {

                           if(LDGetReceivedFrames() > 0)
                           {
                               *pChar++; //header always contains 0xFF. Skip it.
                               int ledblue = *pChar++;
                               int ledgreeen = *pChar++;
                               int ledred = *pChar++;

                               pen.setColor(QColor(ledred, ledgreeen, ledblue));
                               painter.setPen(pen);
                               painter.drawArc(xc - povLedScale * povledRadius[i], yc - povLedScale * povledRadius[i], povLedScale * povledRadius[i] * 2, povLedScale * povledRadius[i] * 2, startAng, aLen);
                           }
                           else
                           {
                               if((led_states[i][supersector] >> shiftbits) & 0x01)
                               {
                                   pen.setColor(QColor("red"));
                               }
                               else
                               {
                                   pen.setColor(QColor("lightgray"));
                               }
    #ifdef MULTI_COLOR
                               /*if(color_states[sector][i])
                               {
                                   pen.setColor(QColor("blue"));
                               }
                               else
                               {
                                   pen.setColor(QColor("lightgray"));
                               }*/

                               // Convert from BGR to RGB
                               pen.setColor(QColor(color_states[sector][i] & 0xFF,             //red
                                                   (color_states[sector][i] & 0xFF00) >> 8,    //green
                                                   (color_states[sector][i] & 0xFF0000) >> 16  //blue
                                                   ));
                               painter.setPen(pen);
                               painter.drawArc(xc - ledRadius[i], yc - ledRadius[i], ledRadius[i] * 2, ledRadius[i] * 2, startAng, aLen);
                           }

#endif

                        }

                    }

                    if(LDGetReceivedFrames() == 0)
                    {
                      pen.setColor(QColor("black"));
                      tmpwidth = pen.width();
                      //pen.setWidth(3);
                      pen.setWidth(2);
                      painter.setPen(pen);
                      painter.drawPoints(mypoints, NOF_LEDS * NOF_SECTORS);
                      pen.setWidth(tmpwidth); //restore
                    }

                    //////////////////////////////////////////////////////////////////
                    /*
                    BITMAPHEADER bmh;
                    char *pBuf;

                    const char *bmpfilename = "/home/anders/src/sim/test.bmp";

                    //if(OpenBmp("c:\\tmp\\test.bmp", &pBuf, &bmh))
                    if(OpenBmp(bmpfilename, &pBuf, &bmh))
                    //if(1)
                    {
                        //failed to open bmp file
                        printf("Failed to openbmp file %s\n", bmpfilename);

                    }
                    else
                    {
                        unsigned int x, y;

                        tmpwidth = pen.width();
                        pen.setWidth(0);
                        painter.setPen(pen);

                        for(x=0; x < bmh.Width; x++)
                        {
                           for(y=0; y < bmh.Height; y++)
                           {
                               if(GetPixel(&bmh, pBuf, x, y) == 0)
                               {
                                   pen.setColor(QColor("black"));
                               }
                               else
                               {
                                   pen.setColor(QColor("white"));
                                   //pen.setColor(QColor(GetPixel(&bmh, pBuf, x, y), GetPixel(&bmh, pBuf, x, y), GetPixel(&bmh, pBuf, x, y)));
                               }
                               painter.setPen(pen);
                               painter.drawPoint(xc + x, yc + y);
                           }
                        }

                        pen.setWidth(tmpwidth); //restore

                        ReleaseBmp(&pBuf);
                    }
                    */

                    //////////////////////////////////////////////////////////////////
                }

                break;
            case Pie:
                painter.drawPie(rect, startAngle, arcLength);
                break;
            case Path:
                painter.drawPath(path);
                break;
            case Text:
                painter.drawText(rect, Qt::AlignCenter, tr("Qt by\nNokia"));
                break;
            case Pixmap:
                painter.drawPixmap(10, 10, pixmap);
            }
//! [12] //! [13]
            painter.restore();
        }
    }

    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(palette().dark().color());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(QRect(0, 0, width() - 1, height() - 1));
}
//! [13]
