#include "camerathread.h"

#include "decoderthread.h"

#include "jpeglib.h"
#include <setjmp.h>
#include <turbojpeg.h>
#include <gphoto2/gphoto2-port-info-list.h>

#include <QDebug>

CameraThread::CameraThread(QObject *parent) : QThread(parent),
    m_context(0), m_abilitiesList(0), m_portInfoList(0), m_camera(0), m_cameraWindow(0), m_stop(false), m_liveview(false), m_decoderThread(0)
{

}

void CameraThread::init()
{
    m_decoderThread = new DecoderThread(this);
    m_decoderThread->start();

    // Create gphoto context
    m_context = gp_context_new();

    // Load abilities list
    gp_abilities_list_new    (&m_abilitiesList);
    gp_abilities_list_load(m_abilitiesList, m_context);

    // Load port info list
    // TODO be able to do it again later ?
    gp_port_info_list_new(&m_portInfoList);
    gp_port_info_list_load(m_portInfoList);
}

void CameraThread::shutdown()
{
    m_decoderThread->stop();
    m_decoderThread->wait();

    if (m_camera) {
        // TODO setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        // TODO updateConfig();
        gp_camera_free(m_camera);
    }

    gp_abilities_list_free(m_abilitiesList);
    gp_context_unref(m_context);
}

bool CameraThread::lookupCamera()
{
    // Autodetect camera
    CameraList *cameraList;
    gp_list_new (&cameraList);
    int count = gp_camera_autodetect(cameraList, m_context);

    if (count == 0)
    {
        return false;
    }

    qInfo() << count << "cameras detected.";

    // Open first camera
    int cameraNumber = 0, ret;

    // If previous camera, free it
    if (m_camera) {
        gp_camera_free(m_camera);
    }
    gp_camera_new(&m_camera);
    const char *modelNamePtr = NULL, *portNamePtr = NULL;

    gp_list_get_name  (cameraList, cameraNumber, &modelNamePtr);
    gp_list_get_value (cameraList, cameraNumber, &portNamePtr);

    m_cameraModel = QString(modelNamePtr);
    m_cameraPort = QString(portNamePtr);

    gp_list_free(cameraList);

    qInfo() << "Open camera :" << m_cameraModel << "at port" << m_cameraPort;

    int model = gp_abilities_list_lookup_model(m_abilitiesList, m_cameraModel.toStdString().c_str());
    if (model < GP_OK) {
        qWarning() << "Model not supported (yet)" ;
        return false;
    }

    ret = gp_abilities_list_get_abilities(m_abilitiesList, model, &m_cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to get abilities list";
        return false;
    }

    ret = gp_camera_set_abilities(m_camera, m_cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to set abilities on camera";
        return false;
    }

    // Then associate the camera with the specified port
    int port = gp_port_info_list_lookup_path(m_portInfoList, m_cameraPort.toStdString().c_str());

    if (port < GP_OK) {
        qWarning() << "Unable to lookup port";
        return false;
    }

    GPPortInfo portInfo;
    ret = gp_port_info_list_get_info (m_portInfoList, port, &portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to get info on port";
        return false;
    }

    ret = gp_camera_set_port_info (m_camera, portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to set port info on camera";
        return false;
    }

    ret = gp_camera_get_config(m_camera, &m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to get root widget";
        return false;
    }

    ret = lookupWidgets(m_cameraWindow);
    if (ret < GP_OK) {
        qWarning() << "Unable to find widgets";
        return false;
    }

    qInfo() << "Camera successfully opened";

    extractCameraCapabilities();
    refreshCameraSettings();


    return true;
    // TODO
    /*
    ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
    if (ret < GP_OK) {
        qWarning() << "Unable to set viewfinder";
        return;
    }
    ret = updateConfig();
    if (ret < GP_OK) {
        qWarning() << "Unable to update config";
        return;
    }

    m_liveviewThread = new CameraThread(m_context, m_camera, this);
    m_liveviewThread->start();

    m_liveviewTimer = new QTimer(this);
    m_liveviewTimer->setInterval(40);
    //connect(m_liveviewTimer, SIGNAL(timeout()), m_liveviewThread, SLOT(capturePreview()));
    connect(m_liveviewThread, SIGNAL(previewAvailable(QPixmap)), this, SLOT(showPreview(QPixmap)));
    connect(m_liveviewThread, SIGNAL(imageAvailable(QImage)), this, SLOT(showImage(QImage)));
    m_liveviewTimer->start();*/
}

int CameraThread::lookupWidgets(CameraWidget* widget) {
    int n = gp_widget_count_children(widget);
    CameraWidget* child;
    const char* widgetName;

    gp_widget_get_name(widget, &widgetName);
    qDebug() << "Found widget" << widgetName;

    m_widgets[widgetName] = widget;

    for (int i = 0; i < n; i ++) {
        int ret = gp_widget_get_child(widget, i, &child);
        if (ret < GP_OK) {
            return ret;
        }

        ret = lookupWidgets(child);
        if (ret < GP_OK) {
            return ret;
        }
    }

    return GP_OK;
}

void CameraThread::extractCameraCapabilities()
{
    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_APERTURE))) {
        m_cameraApertures = extractWidgetChoices(m_widgets[HPIS_CONFIG_KEY_APERTURE]);
    }
}

void CameraThread::refreshCameraSettings()
{
    char* currentValue;

    if (m_widgets.contains(QString(HPIS_CONFIG_KEY_APERTURE))) {

        gp_widget_get_value(m_widgets[HPIS_CONFIG_KEY_APERTURE], &currentValue);
        m_cameraAperture = m_cameraApertures.indexOf(QString(currentValue));
        qInfo() << "Current aperture" << currentValue << "index" << m_cameraAperture;
    }
}

QList<QString> CameraThread::extractWidgetChoices(CameraWidget* widget)
{
    QList<QString> choices;

    const char* choiceLabel;
    int n = gp_widget_count_choices(widget);

    for (int i = 0; i < n; i ++) {
        gp_widget_get_choice(widget, i, &choiceLabel);
        choices.append(QString(choiceLabel));

        qInfo() << "Found choice :" << QString(choiceLabel);
    }

    return choices;
}

void CameraThread::run()
{
    qInfo() << "Start camera thread";
    init();

    while (!m_stop && !lookupCamera()) {
        msleep(500);
    }

    Command command;
    while (!m_stop) {
        /*
        if (m_commandQueue.isEmpty())
        {
            m_mutex.lock();
            m_condition.wait(&m_mutex);
            m_mutex.unlock();
        }*/

        if (!m_stop && m_liveview) {
            doCapturePreview();
        } else if (!m_commandQueue.isEmpty()) {
            m_mutex.lock();
            m_condition.wait(&m_mutex);
            m_mutex.unlock();
        }

        m_mutex.lock();
        if (!m_stop && !m_commandQueue.isEmpty()) {
            command = m_commandQueue.dequeue();
            qDebug() << "Process command" << command;
            doCommand(command);
        }
        m_mutex.unlock();


    }

    shutdown();
    qInfo() << "Stop camera thread";
}

void CameraThread::doCapturePreview()
{
    CameraFile *file;

    int ret = gp_file_new(&file);
    if (ret < GP_OK) {
        qWarning() << "Unable to create camera file for preview";
        return;
    }

    ret = gp_camera_capture_preview(m_camera, file, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to capture preview";
        gp_file_free(file);
        return;
    }

    unsigned long int size;
    const char *data;

    gp_file_get_data_and_size(file, &data, &size);

    //QImage image = decodeImageTurbo(data, size);
    //QImage image = decodeImage(data, size);
    //emit imageAvailable(image);
    if (!m_decoderThread->decodePreview(file))
    {
        gp_file_free(file);
    }

    //m_preview.loadFromData((uchar*) data, size, "JPG");
    //emit previewAvailable(m_preview);


}

void CameraThread::stop()
{
    m_mutex.lock();
    m_stop = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void CameraThread::executeCommand(Command command)
{
    qDebug() << "Received command" << command;
    m_mutex.lock();
    m_commandQueue.append(command);
    m_condition.wakeOne();
    m_mutex.unlock();
}

void CameraThread::capturePreview()
{
    m_condition.wakeOne();
}

void CameraThread::previewDecoded(QImage image)
{
    emit imageAvailable(image);
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

QImage CameraThread::decodeImage(const char *data, unsigned long size)
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    /* More stuff */
    //JSAMPARRAY buffer;            /* Output row buffer */
    int row_stride;               /* physical row width in output buffer */

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object, close the input file, and return.
       */
      jpeg_destroy_decompress(&cinfo);
      return QImage(1, 1, QImage::Format_RGB888);
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_mem_src(&cinfo, (unsigned char*) data, size);

    /* Step 3: read file parameters with jpeg_read_header() */

     (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    /*buffer = (*cinfo.mem->alloc_sarray)
                  ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);*/

    QImage image = QImage(cinfo.output_width, cinfo.output_height, QImage::Format_RGB888);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    //qWarning() << "rowstride :" << row_stride << "output width" << cinfo.output_width << "output height :" << cinfo.output_height;
    int y = 0;
    //uchar* buffer = image.bits();
    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */

        uchar *row = image.scanLine(cinfo.output_scanline);
        (void) jpeg_read_scanlines(&cinfo, &row, 1);
/*
        for (int x = 0; x < cinfo.output_width; x += 3) {
            image.setPixel(x, y, (unsigned int) buffer[0][x]);
        }*/

        /* Assume put_scanline_someplace wants a pointer and sample count. */
        // TODO put_scanline_someplace(buffer[0], row_stride);
        y ++;
    }

    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    return image;
}

QImage CameraThread::decodeImageTurbo(const char *data, unsigned long size)
{
    int jpegSubsamp, width, height;

    tjhandle _jpegDecompressor = tjInitDecompress();

    tjDecompressHeader2(_jpegDecompressor, (uchar*) data, size, &width, &height, &jpegSubsamp);

    QImage image = QImage(width, height, QImage::Format_RGB888);

    tjDecompress2(_jpegDecompressor, (uchar*) data, size, image.bits(), width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);

    tjDestroy(_jpegDecompressor);

    return image;
}

void CameraThread::doCommand(Command command)
{
    int ret;
    switch (command) {
    case CommandStartLiveview:
        ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
        if (ret == GP_OK) {
            m_liveview = true;
        }
        break;

    case CommandStopLiveview:
        ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        if (ret == GP_OK) {
            m_liveview = false;
        }
        break;
    case CommandToggleLiveview:
        if (m_liveview) {
            ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        } else {
            ret = setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
        }
        if (ret == GP_OK) {
            m_liveview = !m_liveview;
        }
        break;

    default: break;
    }

    updateConfig();
}




int CameraThread::setToggleWidget(QString widgetName, int toggleValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &toggleValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to toggle widget :" << widgetName;
        }
        return ret;
    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

int CameraThread::setRangeWidget(QString widgetName, float rangeValue)
{
    CameraWidget* widget = m_widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &rangeValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to set range value to widget :" << widgetName;
        }

        return ret;
    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

int CameraThread::updateConfig()
{
    int ret = gp_camera_set_config(m_camera, m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to update camera config";
    }
    return ret;
}
