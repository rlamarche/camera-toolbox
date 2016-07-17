/**
 * Copyright (c) 2012-2014 Microsoft Mobile.
 */

#include "histogram.h"

#include <QPainter>

#ifndef Q_OS_SYMBIAN
    #include <qmath.h>
#endif

#include "imageanalyzer.h"

const int EdgeSize(3);


/*!
  \class HistogramDisplay
  \brief Displays a histogram. The data is retrieved from ImageAnalyzer. If
         it is not set, does not render anything.
*/


/*!
  Constructor.
*/
HistogramDisplay::HistogramDisplay(QDeclarativeItem *parent)
    : QDeclarativeItem(parent),
      m_analyzer(0),
      m_renderedFrame(0xFFFFFFFF),
      m_displayDownsampledData(true)
{
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    // Create colortable for rendering.
    memset(m_colorTable, 0, sizeof(int) * 512);

    for (int f=0; f<512; f++) {
        m_colorTable[f] =
                (int)((1.0f / (fabs((float)(f-256) / 16.0f) + 1.0f)) * 255.0f);

        if (f>255) {
            if (m_colorTable[f]<192)
                m_colorTable[f] = 192;
        }
        else {
            if (m_colorTable[f]<0)
                m_colorTable[f] = 0;
        }
    }
}


/*!
  Destructor.
*/
HistogramDisplay::~HistogramDisplay()
{
}


/*!
  Getter for the image analyzer.
*/
ImageAnalyzer* HistogramDisplay::imageAnalyzer() const
{
    return m_analyzer;
}


/*!
  Setter for the image analyzer.
*/
void HistogramDisplay::setImageAnalyzer(ImageAnalyzer *imageAnalyzer)
{
    m_analyzer = imageAnalyzer;

    emit imageAnalyzerChanged();
}


/*!
  Renders the histogram.
*/
void HistogramDisplay::paint(QPainter *painter,
                             const QStyleOptionGraphicsItem *option,
                             QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!m_analyzer) {
        qDebug() << "HistogramDisplay: No analyzer set. Cannot render.";
        return;
    }

    int myWidth = boundingRect().width();
    int myHeight = boundingRect().height();

    if (myWidth < 16 || myHeight < 16) {
        // area too small to be rendered
        return;
    }

    if (m_infoImage.width() != myWidth || m_infoImage.height() != myHeight) {
        m_infoImage = QImage(myWidth, myHeight, QImage::Format_ARGB32 );
    }

    if (m_renderedFrame != m_analyzer->getCurrentFrame()) {
        // Draw border
        int movementSensor = m_analyzer->getMovementSensor();
        unsigned int borderCol =
                (movementSensor>>2) | ((movementSensor>>2)<<8) |
                ((movementSensor)<<16);

        for (int f=0; f<EdgeSize; f++) {
            drawRect((unsigned int*)m_infoImage.bits(),
                     m_infoImage.bytesPerLine() / 4,
                     f,
                     f,
                     m_infoImage.width() - f*2,
                     m_infoImage.height() - f*2,
                     borderCol | ((f*80+80) << 24) );
        }

        renderHistogram((unsigned int*)m_infoImage.bits() + EdgeSize +
                        m_infoImage.bytesPerLine() / 4 * EdgeSize,
                        m_infoImage.bytesPerLine() / 4,
                        m_infoImage.width() - EdgeSize * 2,
                        m_infoImage.height() - EdgeSize * 2,
                        m_displayDownsampledData);

        m_renderedFrame = m_analyzer->getCurrentFrame();
    }

    painter->drawImage(0, 0, m_infoImage);
}


/*!
  Simple helper to draw a rectangle for the borders, \a target is pointer to the
  beginning of the image data, \a pitch defines the amount of pixels in one row,
  \a x and \a y are the beginning coordiates, \a width and \a height are the
  dimension of the rectangle and \a color is the color of the rectangle.
*/
void HistogramDisplay::drawRect(unsigned int *target, int pitch, int x, int y,
                                int width, int height, unsigned int color)
{
    target += pitch * y + x;
    for (int f=0; f<width; f++) {
        target[f] = color;
        target[(height-1)*pitch+f] = color;
    }

    for (int f=0; f<height; f++) {
        target[f*pitch] = color;
        target[f*pitch + width-1] = color;
    }
}


/*!
  Render the actual histogram (and only it). The \a target is pointer to the
  beginning of the image data, \a pitch defines the amount of pixels in one row,
  \a width and \a height are the dimension of the histogram and \a lowDetail
  defines is low detailed histogram will be used.
*/
void HistogramDisplay::renderHistogram(unsigned int *target, int pitch,
                                       int width, int height, bool lowDetail)
{
    int f, g;
    int rsample, gsample, bsample;
    int rr, gg, bb, aa;
    int ter, teg, teb;

    unsigned int *histogram = m_analyzer->getRedHistogram();
    unsigned int *lowDetailHistogram = m_analyzer->getLowdetailHistogram();

    int inc = 65536 / height;

    for (g=0; g<width; g++) {
        int colpow;
        if (lowDetail == false) {
            colpow = ((f>>5)&255) - 128;

            // Calculate sample position in 8/8 fixedpoint
            f = ((g*255)<<8) / width;

            // Lineary resmaple to this position
            rsample = (( histogram[(f>>8)]*(256-(f&255)) +
                         histogram[(f>>8)+1]*(f&255) ) >> 8);
            gsample = (( histogram[256+(f>>8)]*(256-(f&255)) +
                         histogram[(f>>8)+257]*(f&255) ) >> 8);
            bsample = (( histogram[512+(f>>8)]*(256-(f&255)) +
                         histogram[(f>>8)+513]*(f&255) ) >> 8);
        } else {
            colpow = ((f>>3)&255) - 128;
            f = ((g*63)<<8) / width;
            rsample = (( lowDetailHistogram[(f>>8)]*(256-(f&255)) +
                         lowDetailHistogram[(f>>8)+1]*(f&255) ) >> 8);
            gsample = (( lowDetailHistogram[64+(f>>8)]*(256-(f&255)) +
                         lowDetailHistogram[(f>>8)+65]*(f&255) ) >> 8);
            bsample = (( lowDetailHistogram[128+(f>>8)]*(256-(f&255)) +
                         lowDetailHistogram[(f>>8)+129]*(f&255) ) >> 8);
        }

        // Draw the vertical line
        unsigned int *verline = target + g;
        unsigned int *verline_target = target + g + height*pitch;

        colpow = ((colpow*colpow) >> 7);
        colpow = ((colpow*colpow) >> 7);
        colpow >>= 2;

        unsigned int linecol = 0x33000000 + colpow + (colpow<<8) + (colpow<<16);

        while (verline!=verline_target) {
            ter = m_colorTable[rsample >> 8];
            teg = m_colorTable[gsample >> 8];
            teb = m_colorTable[bsample >> 8];

            aa = (linecol>>24) + ter+teg+teb;
            rr = (linecol&255) + ter + ((teg+teb)>>2);
            gg = (linecol&255) + teg + ((ter+teb)>>2);
            bb = (linecol&255) + teb + ((teg+ter)>>2);

            if (aa>255) aa=255;
            if (rr>255) rr=255;
            if (bb>255) bb=255;
            if (gg>255) gg=255;

            *verline = (rr) | (gg<<8) | (bb<<16) | (aa<<24);

            rsample += inc;
            gsample += inc;
            bsample += inc;

            verline += pitch;
        }
    }
}
