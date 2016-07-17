/**
 * Copyright (c) 2012 Nokia Corporation.
 */

#ifndef __HISTOGRAM__
#define __HISTOGRAM__


#include <QDeclarativeItem>
#include <QImage>

class ImageAnalyzer;

class HistogramDisplay : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(ImageAnalyzer *imageAnalyzer READ imageAnalyzer WRITE setImageAnalyzer NOTIFY imageAnalyzerChanged)

signals:
    void imageAnalyzerChanged();

public:
    HistogramDisplay(QDeclarativeItem *parent = 0);
    virtual ~HistogramDisplay();

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget);

    inline bool isDisplayingDownsampledData() {
        return m_displayDownsampledData;
    }

    inline void setDisplayDownsampledData(bool set) {
        m_displayDownsampledData = set;
    }

    ImageAnalyzer* imageAnalyzer() const;
    void setImageAnalyzer(ImageAnalyzer *imageAnalyzer);
    void renderHistogram(unsigned int *target, int pitch,
                         int width, int height, bool lowDetail);
protected:


    static void drawRect(unsigned int *target, int pitch, int x, int y,
                         int width, int height, unsigned int color);

protected: // Data
    ImageAnalyzer *m_analyzer; // Not owned

    // The image actually shown on a component.
    QImage m_infoImage;

    // Which frame of the imageanalyzer is currently rendered.
    unsigned int m_renderedFrame;

    bool m_displayDownsampledData;

    // For histogram rendering
    int m_colorTable[512];
};

#endif
