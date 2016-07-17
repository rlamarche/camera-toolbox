/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include "imageanalyzer.h"

#include <QtGui>
#include <math.h>


const int ThumbnailDiv(8);
const int ThumbnailMiddle(4);


/*!
  \class ImageAnalyzer
  \brief Analyze an image for other components to display.
*/


/*!
  Construct the analyzer and reset it's information
*/
ImageAnalyzer::ImageAnalyzer(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_overLitAmount(0.0f),
      m_currentFrame(0),
      m_amountOfMovement(0)
{
    for (int f = 0; f < 256 * 3; f++) {
        m_histogram[f] = 100;
    }

    for (int f = 0; f < 64 * 3; f++) {
        m_lowDetailHistogram[f] = 100;
        m_prevLowDetailHistogram[f] = 100;
    }
}


/*!
  Destructor.
*/
ImageAnalyzer::~ImageAnalyzer()
{
}


/*!
  Creates a thumbnail image from the given source.
*/
void ImageAnalyzer::createThumbnail(QImage source)
{
    int tnWidth = source.width() / ThumbnailDiv;
    int tnHeight = source.height() / ThumbnailDiv;

    if (m_thumbnailImage.isNull() ||
            tnWidth != m_thumbnailImage.width() ||
            tnHeight != m_thumbnailImage.height())
        m_thumbnailImage = QImage(tnWidth, tnHeight,
                                  QImage::Format_ARGB32);

    int y;

    int source_pitch = (source.bytesPerLine() >> 2);
    unsigned int t;

    for (y=0; y<tnHeight; y++) {
        unsigned int *target_horline = (unsigned int*)m_thumbnailImage.bits() +
                (m_thumbnailImage.bytesPerLine()>>2)*y;
        unsigned int *target_horline_target = target_horline + tnWidth;

        unsigned int *source_horline = (unsigned int*)source.bits() +
                y * ThumbnailDiv * source_pitch;

        while (target_horline != target_horline_target) {
            t = ((source_horline[0]>>2)&0x3f3f3f3f) +
                    ((source_horline[ThumbnailMiddle]>>2)&0x3f3f3f3f) +
                    ((source_horline[ThumbnailMiddle*source_pitch]>>2)&0x3f3f3f3f) +
                    ((source_horline[ThumbnailMiddle*source_pitch+ThumbnailMiddle]>>2)&0x3f3f3f3f);

            // the result + 2 most significant bits to the least significant bits.
            *target_horline = t | ((t>>6)&0x03030303);
            target_horline++;
            source_horline+=ThumbnailDiv;
        }
    }
}


/*!
  Feeds new data to the analyzer. Analyses the image data, calculates histogram
  data, detects over exposed areas in the image and burns the over exposed areas
  with red transparent color.

  Example call to this method with QImage would be:
  analyze((unsigned int*)image.bits(),
          image.width(), image.height(),
          image.getBytesPerLine()/4,
          false);
*/
void ImageAnalyzer::analyze(unsigned int *sourceData, int sourceWidth,
                            int sourceHeight, int sourcePitch, bool highDetail)
{
    // Zero the histogram
    memset(m_histogram, 0, sizeof(unsigned int) * 256 * 3);


    // Collect the histogram from the original data
    int step = 1;
    if (highDetail == false)
        step = 4;

    // Collect the histrogram
    for (int y=0; y<sourceHeight; y+=step) {
        unsigned int *sourceHorline = sourceData + y*sourcePitch;
        unsigned int *sourceHorlineTarget = sourceHorline + sourceWidth;

        while (sourceHorline<sourceHorlineTarget)
        {
            m_histogram[ ((*sourceHorline)&255) ]++;
            m_histogram[ 256+(((*sourceHorline)>>8)&255) ]++;
            m_histogram[ 512+(((*sourceHorline)>>16)&255) ]++;
            sourceHorline+=step;
        }
    }


    // Calculate maximum values
    unsigned int largestR = 0;
    unsigned int largestG = 0;
    unsigned int largestB = 0;
    for (int f=0; f<256; f++) {
        if (m_histogram[f]>largestR) largestR = m_histogram[f];
        if (m_histogram[f+256]>largestG) largestG = m_histogram[f+256];
        if (m_histogram[f+512]>largestB) largestB = m_histogram[f+512];
    }

    // Normalize histogram
    if (largestR == 0) largestR = 1;
    if (largestG == 0) largestG = 1;
    if (largestB == 0) largestB = 1;
    for (int f=0; f<256; f++) {
        m_histogram[f] =
                (( m_histogram[f] * (( 256 * 65536 ) / largestR) ) >> 8);
        m_histogram[f+256] =
                (( m_histogram[f+256] * ((256 * 65536) / largestG)) >> 8);
        m_histogram[f+512] =
                (( m_histogram[f+512] * ((256 * 65536) / largestB)) >> 8);
    }


    // Calculate downscaled version and the amount of movement
    memcpy(m_prevLowDetailHistogram, m_lowDetailHistogram,
           sizeof(unsigned int) * 64*3);

    m_amountOfMovement = 0;

    for (int g=0; g<3; g++) {
        for (int f=0; f<64; f++) {
            m_lowDetailHistogram[(g<<6)+f] =
                    ((m_histogram[(g<<8)+(f<<2)+0] +
                      m_histogram[(g<<8)+(f<<2)+1] +
                      m_histogram[(g<<8)+(f<<2)+2] +
                      m_histogram[(g<<8)+(f<<2)+3] ) >> 2);

            m_amountOfMovement += abs((int)m_lowDetailHistogram[(g<<6)+f] -
                                      (int)m_prevLowDetailHistogram[(g<<6)+f]);
        }

    }

    m_amountOfMovement /= (64*3);

    m_movementSensor = (m_amountOfMovement)-2000;

    if (m_movementSensor < 0)
        m_movementSensor = 0;

    m_movementSensor >>= 3;

    if (m_movementSensor > 255)
        m_movementSensor = 255;

    m_currentFrame++;


    // Create thumnbail image. It is required to find out the over exposed areas
    // efficiently. The over exposed indicator will have pixelated look but it
    // doesn't matter in this cas.
    createThumbnail(QImage((uchar*)sourceData, sourceWidth, sourceHeight,
                           QImage::Format_ARGB32));

    int overlitblocks = 0;

    // Burn over exposed info into the data.
    for (int y=0; y<m_thumbnailImage.height(); y++) {
        unsigned int *sourceHorline =
                (unsigned int*)m_thumbnailImage.bits() +
                y*(m_thumbnailImage.bytesPerLine()>>2);

        unsigned int *sourceHorlineTarget = sourceHorline
                + m_thumbnailImage.width();

        unsigned int *t = sourceData + y * sourcePitch * ThumbnailDiv;
        unsigned int *t_target;

        while (sourceHorline!=sourceHorlineTarget) {
            if ((*sourceHorline&0x00FFFFFF) == 0x00FFFFFF) {
                for (int f=0; f<ThumbnailDiv; f++) {
                    t_target = t+ThumbnailDiv;
                    while (t!=t_target) {
                        *t = ((*t&0xFEFEFEFE)>>1) + 0x77550000; t++;
                    }

                    t -= ThumbnailDiv;
                    t += sourcePitch;
                }
                t -= sourcePitch * ThumbnailDiv;
                overlitblocks++;
            }

            sourceHorline++;
            t += ThumbnailDiv;
        }
    }


    m_overLitAmount = (float)overlitblocks /
            (float)(m_thumbnailImage.width() * m_thumbnailImage.height());
}
