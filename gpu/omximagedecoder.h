#ifndef OMXIMAGEDECODER_H
#define OMXIMAGEDECODER_H

#include <QImage>


#define OMXJPEG_OK                  0
#define OMXJPEG_ERROR_ILCLIENT_INIT    -1024
#define OMXJPEG_ERROR_OMX_INIT         -1025
#define OMXJPEG_ERROR_MEMORY         -1026
#define OMXJPEG_ERROR_CREATING_COMP    -1027
#define OMXJPEG_ERROR_WRONG_NO_PORTS   -1028
#define OMXJPEG_ERROR_EXECUTING         -1029
#define OMXJPEG_ERROR_NOSETTINGS   -1030

extern "C" {
    #include "../omx/ilclient.h"
}

typedef struct _OPENMAX_JPEG_DECODER OPENMAX_JPEG_DECODER;

class OMXImageDecoder
{
public:
    OMXImageDecoder();
    ~OMXImageDecoder();

    bool decodeImage(const char *sourceImage, unsigned long imageSize);
    QImage& outputImage();
protected:
    int doDecodeImage(OPENMAX_JPEG_DECODER * decoder, char *sourceImage,
            size_t imageSize);

    int setupOpenMaxJpegDecoder(OPENMAX_JPEG_DECODER ** pDecoder);
    int startupImageDecoder(OPENMAX_JPEG_DECODER * decoder);
    int prepareImageDecoder(OPENMAX_JPEG_DECODER * decoder);
    int prepareResizer(OPENMAX_JPEG_DECODER * decoder);
    int portSettingsChangedAgain(OPENMAX_JPEG_DECODER * decoder);
    int portSettingsChanged(OPENMAX_JPEG_DECODER * decoder);
    void printState(OMX_HANDLETYPE handle);
    void cleanup(OPENMAX_JPEG_DECODER * decoder);
    void save_image(OPENMAX_JPEG_DECODER *decoder);

private:
    int m_bufferIndex;
    OPENMAX_JPEG_DECODER *m_pDecoder;

    int m_width;
    int m_height;
    QImage m_outputImage;
};

#endif // OMXIMAGEDECODER_H
