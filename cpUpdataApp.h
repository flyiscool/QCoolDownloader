#pragma once

#include <QtWidgets/QMainWindow>
#include "libusb.h"
#include "cpThread.h"
#include <QThread>
#include <QMutex>
#include <QMetaType>



// The thread monitor the usb device hot-plug
class CPThreadUsbMonitor : public CPThread
{
    Q_OBJECT
public:
    virtual void run();
    libusb_device** devsList;
    libusb_context* ctx;
    libusb_device_handle* devGround;
    libusb_device_handle* devSky;
    int Cmd_Upgrade(libusb_device_handle* dev, QString* fileNames);
    int Get_Upgrade_Version(libusb_device_handle* dev, unsigned char* buf);
    int Cmd_Upgrade_V2(libusb_device_handle* dev, QString* fileNames);
    int Cmd_Upgrade_V3(libusb_device_handle* dev, QString* fileNames);
    void threadCPUsbMonitor_main(void);
    void setfilename(QString fileNames);
    QString file;

signals:
    void signalupdateTextUi(QString);
};
