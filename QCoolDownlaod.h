#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCoolDownlaod.h"
#include "libusb.h"
#include <QThread>
#include <QMutex>
#include <QMetaType>

class CPThread : public QThread
{
    Q_OBJECT
public slots:
    virtual void stopImmediately()
    {
        m_isCanRun = false;
    }

    QMutex* GetMutex() { return &m_lock; }

    bool IsRun()
    {
        return m_isCanRun;
    };

public:
    QMutex m_lock;
    bool m_isCanRun;
};

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


signals:
    void signalupdateTextUi(QString);
};


class QCoolDownlaod : public QMainWindow
{
    Q_OBJECT
            
  
public:
    QCoolDownlaod(QWidget *parent = Q_NULLPTR);


public slots:
    void slotselfile(void);
    void slotdownloadfileflag(void);
    void slotupdateTextUi(QString);
private:
    Ui::QCoolDownlaodClass ui;
    CPThreadUsbMonitor		thUsbMonitor;
};



