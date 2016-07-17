/**
 * Copyright (c) 2012 Nokia Corporation.
 */

#ifndef __IMAGEANALYZER__
#define __IMAGEANALYZER__

#include <QDeclarativeItem>
#include <QImage>

class ImageAnalyzer : public QDeclarativeItem
{
    Q_OBJECT
    Q_PROPERTY(float overLitAmount READ getOverLitAmount NOTIFY overLitAmountChanged)

public:
    ImageAnalyzer(QDeclarativeItem *parent = 0);
    virtual ~ImageAnalyzer();

    void analyze(unsigned int *sourceData,
                 int sourceWidth, int sourceHeight,
                 int sourcePitch, bool highDetail);

    QImage getThumbnailImage() const { return m_thumbnailImage; }

    // Histogram information
    inline unsigned int *getRedHistogram() { return m_histogram; }
    inline unsigned int *getGreenHistogram() { return m_histogram + 256; }
    inline unsigned int *getBlueHistogram() { return m_histogram + 512; }

    inline unsigned int *getLowdetailHistogram() {
        return m_lowDetailHistogram;
    }

    // Return movement indicator between 0 and 1
    inline float getMovement() const {
        return (float)m_movementSensor / 255.0f;
    }
    inline int getMovementSensor() const { return m_movementSensor;}
    inline float getOverLitAmount() const { return m_overLitAmount; }

    // Return the raw amount of movement
    inline int getRawAmountOfMovement() const { return m_amountOfMovement; }

    inline unsigned int getCurrentFrame() const { return m_currentFrame; }

signals:
    void overLitAmountChanged();

protected:
    void createThumbnail(QImage source);

protected:
    QImage m_thumbnailImage;

    // Between 0 and 1. How large part of the image is overlit.
    float m_overLitAmount;

    unsigned int m_currentFrame;

    // 0 - 65535 : Raw change sigma from previous frame
    int m_amountOfMovement;
    // 0 - 255 : Tweaked "movement" - value from amount of movement
    int m_movementSensor;

    // RGB-component histogram: 256*3 = 768
    // Latest frame
    unsigned int m_histogram[768];

    // Low detail, downscaled version of the histogram
    unsigned int m_lowDetailHistogram[64 * 3];
    unsigned int m_prevLowDetailHistogram[64 * 3];
};

QML_DECLARE_TYPE(ImageAnalyzer)

#endif
