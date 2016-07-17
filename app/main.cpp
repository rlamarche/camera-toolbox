#include "mainwindow.h"
#include <QApplication>
#include <QOpenGLWindow>

#include <QDebug>


#include <gphoto2/gphoto2-camera.h>
#include <gphoto2/gphoto2-context.h>
#include <gphoto2/gphoto2-port-info-list.h>

#ifdef USE_RPI
#include <bcm_host.h>
#endif

#include <assert.h>

#include "gphoto/gpcamera.h"

#ifdef USE_RPI
#include "hello_jpeg_v2/Logger.h"
#include "hello_jpeg_v2/JPEG.h"


#include "gpu/omximagedecoder.h"
#endif

#include <sys/time.h>
#include <ltdl.h>


// Server
#include <qhttpserver.hpp>
#include <qhttpserverresponse.hpp>


/* time zero for debug log time stamps */
static struct timeval glob_tv_zero = { 0, 0 };


static void
debug_func (GPLogLevel level, const char *domain, const char *str, void *data)
{
    struct timeval tv;
    long sec, usec;
    FILE *logfile = (data != NULL)?(FILE *)data:stderr;

    gettimeofday (&tv,NULL);
    sec = tv.tv_sec  - glob_tv_zero.tv_sec;
    usec = tv.tv_usec - glob_tv_zero.tv_usec;
    if (usec < 0) {sec--; usec += 1000000L;}
    fprintf (logfile, "%li.%06li %-28s(%i): %s\n", sec, usec, domain, level, str);
    printf ("%li.%06li %-28s(%i): %s\n", sec, usec, domain, level, str);
}


int
debug_action (const char *debug_loglevel, const char *debug_logfile_name)
{
    int n;
    FILE *logfile = NULL;
    GPLogLevel loglevel = GP_LOG_ALL;

    /* make sure we're only executed once */
    static int debug_flag = 0;
    if (debug_flag != 0)
        return(GP_OK);
    debug_flag = 1;

    if (debug_loglevel && !strcmp(debug_loglevel, "error"))
        loglevel = GP_LOG_ERROR;
    else if (debug_loglevel && !strcmp(debug_loglevel, "debug"))
        loglevel = GP_LOG_DEBUG;
    else if (debug_loglevel && !strcmp(debug_loglevel, "data"))
        loglevel = GP_LOG_DATA;
    else if (debug_loglevel && !strcmp(debug_loglevel, "all"))
        loglevel = GP_LOG_ALL;

    if (debug_logfile_name != NULL) {
      /* FIXME: Handle fopen() error besides using stderr? */
      logfile = fopen(debug_logfile_name, "a");
    }
    if (logfile == NULL) {
      logfile = stderr;
    }
    setbuf(logfile, NULL);
    setbuf(stdout, NULL);

    gettimeofday (&glob_tv_zero, NULL);

    gp_log_add_func (loglevel, debug_func, (void *) logfile);
    gp_log (GP_LOG_DEBUG, "main", "ALWAYS INCLUDE THE FOLLOWING LINES "
                    "WHEN SENDING DEBUG MESSAGES TO THE "
                    "MAILING LIST:");

    if (1) {
        /* This is internal debug stuff for developers - no
         * need for translation IMHO */
        const char *iolibs = getenv("IOLIBS");
        const char *camlibs = getenv("CAMLIBS");
        if (camlibs) {
            gp_log (GP_LOG_DEBUG, "main", "CAMLIBS = '%s'", camlibs);
        } else {
            gp_log (GP_LOG_DEBUG, "main",
                "CAMLIBS env var not set, using compile-time default instead");
        }
        if (iolibs) {
            gp_log (GP_LOG_DEBUG, "main", "IOLIBS = '%s'", iolibs);
        } else {
            gp_log (GP_LOG_DEBUG, "main",
                "IOLIBS env var not set, using compile-time default instead");
        }
    }

    return (GP_OK);
}


bool lookupCamera(hpis::Camera** camera) {
    GPContext *context = gp_context_new();

    // Autodetect camera
    CameraList *cameraList;
    gp_list_new (&cameraList);
    int count = gp_camera_autodetect(cameraList, context);

    if (count <= 0)
    {
        if (count == 0) {
            qInfo() << "No camera detected";
        } else {
            qInfo() << "GP ERROR :" << count;
        }
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
    qRegisterMetaType<hpis::CameraStatus>();
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


    debug_action(NULL, "out.log");

    QApplication a(argc, argv);

    hpis::Camera* camera;

    if (lookupCamera(&camera))
    {
        hpis::CameraThread cameraThread(camera);
        MainWindow w(&cameraThread);
        cameraThread.start();

        w.show();


        using namespace qhttp::server;

        QHttpServer server(&a);
        // listening on 0.0.0.0:8080
        server.listen(QHostAddress::Any, 8080, [](QHttpRequest* req, QHttpResponse* res) {

            res->setStatusCode(qhttp::ESTATUS_OK);      // status 200
            res->addHeader("connection", "close");      // it's the default header, this line can be omitted.
            res->end("Hello World!\n");                 // response body data

            // when "connection: close", the req and res will be deleted automatically.
        });


        if ( !server.isListening() ) {
            fprintf(stderr, "failed. can not listen at port 8080!\n");
            return -1;
        }

        return a.exec();
    }

    return 1;
}



