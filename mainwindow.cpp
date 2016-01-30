#include "mainwindow.h"

#include <QTimer>

#include <QKeyEvent>
#include <QShowEvent>

#include <QDebug>

#include <iostream>

using namespace std;

MainWindow::MainWindow() :
    QOpenGLWindow()
{
    // Create gphoto context
    context = gp_context_new();

    // Load abilities list
    gp_abilities_list_new    (&abilitieslist);
    gp_abilities_list_load(abilitieslist, context);

    // Load port info list
    // TODO be able to do it again later ?
    gp_port_info_list_new(&portinfolist);
    gp_port_info_list_load(portinfolist);

    // Init to 0
    camera = 0;

    lookupCameraTimer = new QTimer(this);
    lookupCameraTimer->setInterval(1000);
    connect(lookupCameraTimer, SIGNAL(timeout()), this, SLOT(lookupCamera()));
    lookupCameraTimer->start();


    liveviewTimer = new QTimer(this);
    liveviewTimer->setInterval(200);
    connect(liveviewTimer, SIGNAL(timeout()), this, SLOT(capturePreview()));
}

MainWindow::~MainWindow()
{
    if (camera)
    {
        gp_camera_free(camera);
    }

    gp_abilities_list_free(abilitieslist);
    gp_context_unref(context);
}

void MainWindow::lookupCamera()
{
    qInfo() << "Lookup camera";

    // Autodetect camera
    CameraList *list;
    gp_list_new (&list);
    int count = gp_camera_autodetect(list, context);



    if (count == 0)
    {
        return;
    }

    qInfo() << count << "cameras detected.";

    // Open first camera
    int cameraNumber = 0, ret;

    if (camera) {
        gp_camera_free(camera);
    }
    gp_camera_new(&camera);
    const char *modelNamePtr = NULL, *portNamePtr = NULL;
    CameraAbilities cameraAbilities;

    gp_list_get_name  (list, cameraNumber, &modelNamePtr);
    gp_list_get_value (list, cameraNumber, &portNamePtr);

    QString modelName(modelNamePtr);
    QString portName(portNamePtr);

    gp_list_free(list);

    qInfo() << "Open camera :" << modelName << "at port" << portName;

    int model = gp_abilities_list_lookup_model(abilitieslist, modelName.toStdString().c_str());
    if (model < GP_OK) {
        qWarning() << "Model not supported (yet)" ;
        return;
    }

    ret = gp_abilities_list_get_abilities(abilitieslist, model, &cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to get abilities list";
        return;
    }

    ret = gp_camera_set_abilities(camera, cameraAbilities);
    if (ret < GP_OK) {
        qWarning() << "Unable to set abilities on camera";
        return;
    }

    // Then associate the camera with the specified port
    int port = gp_port_info_list_lookup_path(portinfolist, portName.toStdString().c_str());

    if (port < GP_OK) {
        qWarning() << "Unable to lookup port";
        return;
    }

    GPPortInfo portInfo;
    ret = gp_port_info_list_get_info (portinfolist, port, &portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to get info on port";
        return;
    }

    ret = gp_camera_set_port_info (camera, portInfo);
    if (ret < GP_OK) {
        qWarning() << "Unable to set port info on camera";
        return;
    }

    ret = gp_camera_get_config(camera, &cameraWindow, context);
    if (ret < GP_OK) {
        qWarning() << "Unable to get root widget";
        return;
    }

    ret = findWidgets(cameraWindow);
    if (ret < GP_OK) {
        qWarning() << "Unable to find widgets";
        return;
    }

    qInfo() << "Camera successfully opened";

    lookupCameraTimer->stop();

    ret = toggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 1);
    if (ret < GP_OK) {
        qWarning() << "Unable to set viewfinder";
        return;
    }

    //liveviewTimer->start();
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

        widgets[QString(widgetName)] = child;

        ret = findWidgets(child);
        if (ret < GP_OK) {
            return ret;
        }
    }

    return GP_OK;
}

void MainWindow::capturePreview()
{
    CameraFile *file;

    int ret = gp_file_new(&file);
    if (ret < GP_OK) {
        qWarning() << "Unable to create camera file for preview";
        return;
    }

    ret = gp_camera_capture_preview(camera, file, context);
    if (ret < GP_OK) {
        qWarning() << "Unable to capture preview";
        gp_file_free(file);
        return;
    }

    gp_file_free(file);
}

int MainWindow::toggleWidget(QString widgetName, int toggleValue)
{
    CameraWidget* widget = widgets[widgetName];

    if (widget)
    {
        int ret = gp_widget_set_value(widget, &toggleValue);
        if (ret < GP_OK) {
            qWarning() << "Unable to set value to widget :" << widgetName;
            return ret;
        }

        ret = gp_camera_set_config(camera, cameraWindow, context);
        if (ret < GP_OK) {
            qWarning() << "Unable to set camera config :" << widgetName;
            return ret;
        }

    } else {
        qWarning() << "Widget not found :" << widgetName;
        return -1;
    }

    return GP_OK;
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        lookupCameraTimer->stop();
        liveviewTimer->stop();
        if (camera) {
            toggleWidget(HPIS_CONFIG_KEY_VIEWFINDER, 0);
        }
        this->close();
        break;
    default:
        QOpenGLWindow::keyPressEvent(event);
    }
}
