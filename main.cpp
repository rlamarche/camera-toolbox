#include "mainwindow.h"
#include <QApplication>
#include <QOpenGLWindow>

#include <QDebug>

#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>


#include <bcm_host.h>
#include <assert.h>

#include "gphoto/gpcamera.h"

#include "hello_jpeg_v2/Logger.h"
#include "hello_jpeg_v2/JPEG.h"


#include "gpu/omximagedecoder.h"

bool lookupCamera(hpis::Camera** camera) {
    GPContext *context = gp_context_new();

    // Autodetect camera
    CameraList *cameraList;
    gp_list_new (&cameraList);
    int count = gp_camera_autodetect(cameraList, context);

    if (count == 0)
    {
        gp_context_unref(context);
        return false;
    }

    qInfo() << count << "cameras detected.";

    const char *modelName = NULL, *portName = NULL;

    gp_list_get_name  (cameraList, 0, &modelName);
    gp_list_get_value (cameraList, 0, &portName);

    *camera = new hpis::GPCamera(QString(modelName), QString(portName));

    gp_context_unref(context);
    return true;
}

int main(int argc, char *argv[])
{
/*
    OMXImageDecoder* decoder = new OMXImageDecoder();
    char           *sourceImage;
    size_t          imageSize;
    int             s;
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }
    FILE           *fp = fopen(argv[1], "rb");
    if (!fp) {
        printf("File %s not found.\n", argv[1]);
    }

    fseek(fp, 0L, SEEK_END);
    imageSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    sourceImage = (char*) malloc(imageSize);
    assert(sourceImage != NULL);
    s = fread(sourceImage, 1, imageSize, fp);
    assert(s == imageSize);
    fclose(fp);

    bool success = true;
    for (int i = 0; i < 10; i ++)
    {
        success = success && decoder->decodeImage(sourceImage, imageSize);
    }

    free(sourceImage);

    if (success) {
        printf("Success\n");
    }

    delete decoder;

    return 0;
*/

    /*
    if (argc < 2) {
        printf("Usage: %s <input filename>\n", argv[0]);
        return (1);
    }

    bcm_host_init();

    Logger logstdout;
    ILogger *pLogger = &logstdout;
    JPEG j(pLogger);

    int iRes = j.DoIt(argv[1]);

    bcm_host_deinit();

    return iRes;
*/


    QApplication a(argc, argv);

    hpis::Camera* camera;

    if (lookupCamera(&camera))
    {
        MainWindow w(camera);
        w.show();

        return a.exec();
    }

    return 1;
}



