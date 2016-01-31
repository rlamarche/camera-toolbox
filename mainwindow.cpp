#include "mainwindow.h"

#include <QTimer>

#include <QKeyEvent>
#include <QShowEvent>

#include <QPainter>
#include <QTime>

#include <QDebug>

#include <iostream>

MainWindow::MainWindow() :
    QOpenGLWindow(), m_liveviewTimer(0), m_liveviewThread(0), m_camera(0), fps(0)
{
    // Create gphoto context
    m_context = gp_context_new();

    // Load abilities list
    gp_abilities_list_new    (&m_abilitieslist);
    gp_abilities_list_load(m_abilitieslist, m_context);

    // Load port info list
    // TODO be able to do it again later ?
    gp_port_info_list_new(&m_portInfoList);
    gp_port_info_list_load(m_portInfoList);

    m_lookupCameraTimer = new QTimer(this);
    m_lookupCameraTimer->setInterval(1000);
    connect(m_lookupCameraTimer, SIGNAL(timeout()), this, SLOT(lookupCamera()));
    m_lookupCameraTimer->start();

    m_overscanLeft = -32;
    m_overscanRight = -32;
    m_overscanTop = -32;
    m_overscanBottom = -32;
}

MainWindow::~MainWindow()
{
    if (m_liveviewTimer) {
        m_liveviewTimer->stop();
    }
    if (m_liveviewThread) {
        m_liveviewThread->stop();
        m_liveviewThread->wait();
    }

    m_lookupCameraTimer->stop();
    if (m_camera) {
        setToggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        updateConfig();
        gp_camera_free(m_camera);
    }

    gp_abilities_list_free(m_abilitieslist);
    gp_context_unref(m_context);
}

void MainWindow::lookupCamera()
{
    update();

    // Autodetect camera
    CameraList *list;
    gp_list_new (&list);
    int count = gp_camera_autodetect(list, m_context);



    if (count == 0)
    {
        return;
    }

    qInfo() << count << "cameras detected.";

    // Open first camera
    int cameraNumber = 0, ret;

    if (m_camera) {
        gp_camera_free(m_camera);
    }
    gp_camera_new(&m_camera);
    const char *modelNamePtr = NULL, *portNamePtr = NULL;
    CameraAbilities cameraAbilities;

    gp_list_get_name  (list, cameraNumber, &modelNamePtr);
    gp_list_get_value (list, cameraNumber, &portNamePtr);

    QString modelName(modelNamePtr);
    QString portName(portNamePtr);

    gp_list_free(list);

    qInfo() << "Open camera :" << modelName << "at port" << portName;

    int model = gp_abilities_list_lookup_model(m_abilitieslist, modelName.toStdString().c_str());
    if (model < GP_OK) {
        qWarning() << "Model not supported (yet)" ;
        return;
    }

    ret = gp_abilities_list_get_abilities(m_abilitieslist, model, &cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to get abilities list";
        return;
    }

    ret = gp_camera_set_abilities(m_camera, cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to set abilities on camera";
        return;
    }

    // Then associate the camera with the specified port
    int port = gp_port_info_list_lookup_path(m_portInfoList, portName.toStdString().c_str());

    if (port < GP_OK) {
        qWarning() << "Unable to lookup port";
        return;
    }

    GPPortInfo portInfo;
    ret = gp_port_info_list_get_info (m_portInfoList, port, &portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to get info on port";
        return;
    }

    ret = gp_camera_set_port_info (m_camera, portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to set port info on camera";
        return;
    }

    ret = gp_camera_get_config(m_camera, &m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to get root widget";
        return;
    }

    ret = findWidgets(m_cameraWindow);
    if (ret < GP_OK) {
        qWarning() << "Unable to find widgets";
        return;
    }

    qInfo() << "Camera successfully opened";

    m_lookupCameraTimer->stop();

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
    m_liveviewTimer->start();
}

int MainWindow::findWidgets(CameraWidget* widget) {
    int n = gp_widget_count_children(widget);
    CameraWidget* child;
    const char* widgetName;

    for (int i = 0; i < n; i ++) {

        int ret = gp_widget_get_child(widget, i, &child);
        if (ret < GP_OK) {
            return ret;
        }

        gp_widget_get_name(child, &widgetName);
        qDebug() << "Found widget" << widgetName;

        m_widgets[QString(widgetName)] = child;

        ret = findWidgets(child);
        if (ret < GP_OK) {
            return ret;
        }
    }

    return GP_OK;
}

void MainWindow::showPreview(QPixmap preview)
{
    static QTime time;
    static int frameCount = 0;

    frameCount ++;

    if (frameCount > 0 && frameCount % 10 == 0) {
        frameCount = 0;
        fps = 10.0f / time.elapsed() * 1000.0f;
        time.restart();
    }

    m_preview = preview;
    update();
}

void MainWindow::showImage(QImage image)
{
    static QTime time;
    static int frameCount = 0;

    frameCount ++;

    if (frameCount > 0 && frameCount % 10 == 0) {
        frameCount = 0;
        fps = 10.0f / time.elapsed() * 1000.0f;
        time.restart();
    }

    m_image = image;
    update();
}

int MainWindow::setToggleWidget(QString widgetName, int toggleValue)
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

int MainWindow::setRangeWidget(QString widgetName, float rangeValue)
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

int MainWindow::updateConfig()
{
    int ret = gp_camera_set_config(m_camera, m_cameraWindow, m_context);
    if (ret < GP_OK) {
        qWarning() << "Unable to update camera config";
    }
    return ret;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;
    default:
        QOpenGLWindow::keyPressEvent(event);
    }
}

void MainWindow::resizeGL(int w, int h)
{

}

void MainWindow::paintGL()
{
    QPainter p(this);

    float ratio = (float) m_image.height() / (float) m_image.width();

    float width = (float) m_image.width();
    float height = (this->width() + m_overscanLeft + m_overscanRight) * ratio;

    if (height > (this->height() + m_overscanBottom + m_overscanTop))
    {
        height = (float) (this->height() + m_overscanBottom + m_overscanTop);
        width = height / ratio;
    }

    int x = ((this->width() + m_overscanLeft + m_overscanRight) - (int) width) / 2 - m_overscanLeft;
    int y = ((this->height() + m_overscanBottom + m_overscanTop) - (int) height) / 2 - m_overscanTop;


    //p.drawPixmap(x, y, (int) width, (int) height, m_preview);
    p.drawImage(QRect(x, y, (int) width, (int) height), m_image);
    //p.drawImage(QRect(0, 0, m_image.width(), m_image.height()), m_image);

    p.setPen(QColor(255, 255, 255));
    //p.drawText(QPointF(0, 0), QString("FPS : %1").arg(fps));
    p.drawText(0 - m_overscanLeft, this->height() + m_overscanBottom, QString("FPS : %1").arg(fps));
   // p.drawText(0, 0, this->width(), this->height(), Qt::AlignCenter, QString("Hello, World"));

}
