#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_QCoolDownlaod.h"
#include "libusb.h"
#include "cpThread.h"
#include <QThread>
#include <QMutex>
#include <QMetaType>
#include "cpUpdataApp.h"
#include "cpStructData.h"
#include "cpCoolflyMonitor.h"
#include "cpCMDParse.h"
#include "cpFlyDebug.h"

class QCoolDownlaod : public QMainWindow
{
    Q_OBJECT
            
  
public:
    QCoolDownlaod(QWidget *parent = Q_NULLPTR);
    ~QCoolDownlaod(void);

    QString g_fileNames;

public slots:
    void slotselfile(void);
    void slotdownloadfileflag(void);
    void slotupdateTextUi(QString);
    void slotconnectusb(void);
    void slotsetbt_connect_color(State_LED st);
    void slotcmd_get_version(void);
    void slotcmd_get_setting(void);
    void slotflydebug_start(void);
    void slotflydebug_stop(void);
private:
    Ui::QCoolDownlaodClass ui;
    CPThreadUsbMonitor		thUsbMonitor;
    CPThreadCoolflyMonitor	thCoolflyMonitor;
    CPThreadCMDParse	thThreadCMDParse;
    CPThreadFlyDebug	thThreadFlyDebug;
    CPThreadFlyDebugParse	thThreadFlyDebugParse;
};

