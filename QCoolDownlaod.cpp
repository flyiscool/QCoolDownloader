#include "QCoolDownlaod.h"
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <QMutex>
#include <QMetaType>
#include "cpThreadSafeQueue.h"
#include "cpStructData.h"

extern threadsafe_queue<CMDBuffPackage*> gListTXCMD;
extern threadsafe_queue<CMDBuffPackage*> gListRXCMD;


void QCoolDownlaod::slotselfile(void)
{
	g_fileNames = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("bin(*.bin)"));

	ui.textBrowser->setText(g_fileNames);

    ui.statusBar->showMessage(g_fileNames, 5000);
}

void QCoolDownlaod::slotdownloadfileflag(void)
{
    thUsbMonitor.setfilename(g_fileNames);
     thUsbMonitor.start();
 }


void QCoolDownlaod::slotupdateTextUi(QString text)
{
    ui.textBrowser->append(text);
}


void QCoolDownlaod::slotconnectusb(void)
{
    static bool connect_flag = false;
    if (connect_flag == false)
    {
        connect_flag = true;
        CMDBuffPackage* cmd;
        while(gListTXCMD.size() > 0)
        {
            gListTXCMD.try_pop(cmd);
            delete cmd;
        }
            
        thCoolflyMonitor.start();
        thThreadCMDParse.start();

        thThreadFlyDebug.stopImmediately();
        thThreadFlyDebug.wait();

        thThreadFlyDebugParse.stopImmediately();
        thThreadFlyDebugParse.wait();
    }
    else
    {
        connect_flag = false;
        thCoolflyMonitor.stopImmediately();
        thCoolflyMonitor.wait();

        thThreadCMDParse.stopImmediately();
        thThreadCMDParse.wait();

        thThreadFlyDebug.stopImmediately();
        thThreadFlyDebug.wait();

        thThreadFlyDebugParse.stopImmediately();
        thThreadFlyDebugParse.wait();

        ui.sel_connect->setStyleSheet("background-color: rgb(220,220,220)");
    }
}

void QCoolDownlaod::slotsetbt_connect_color(State_LED st)
{
    switch(st)
    {
        case idle:   ui.sel_connect->setStyleSheet("background-color: rgb(220,220,220)"); break;
        case normal: ui.sel_connect->setStyleSheet("background-color: rgb(52,251,152)"); break;
        case error:  ui.sel_connect->setStyleSheet("background-color: rgb(255,69,0)"); break;
        case end:    ui.sel_connect->setStyleSheet("background-color: rgb(220,220,220)"); break;
    }
}



QCoolDownlaod::QCoolDownlaod(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
   
    connect(ui.sel_connect, SIGNAL(clicked()), this, SLOT(slotconnectusb()));

    connect(ui.sel_bt, SIGNAL(clicked()), this, SLOT(slotselfile()));

    connect(ui.download_pt, SIGNAL(clicked()), this, SLOT(slotdownloadfileflag()));

    connect(&thUsbMonitor, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thCoolflyMonitor, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadCMDParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebug, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebugParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));

    qRegisterMetaType<State_LED>("State_LED");
    connect(&thCoolflyMonitor, SIGNAL(signalupdateStateLED(State_LED)), this, SLOT(slotsetbt_connect_color(State_LED)));


    connect(ui.sel_get_verison, SIGNAL(clicked()), this, SLOT(slotcmd_get_version()));
    connect(ui.bt_get_setting, SIGNAL(clicked()), this, SLOT(slotcmd_get_setting()));

    connect(ui.bt_flydebugstart, SIGNAL(clicked()), this, SLOT(slotflydebug_start()));
    connect(ui.bt_flydebugstop, SIGNAL(clicked()), this, SLOT(slotflydebug_stop()));

}


QCoolDownlaod::~QCoolDownlaod(void)
{
    thCoolflyMonitor.stopImmediately();
    thCoolflyMonitor.wait();

    thThreadCMDParse.stopImmediately();
    thThreadCMDParse.wait();

    thThreadFlyDebug.stopImmediately();
    thThreadFlyDebug.wait();

    thThreadFlyDebugParse.stopImmediately();
    thThreadFlyDebugParse.wait();
}


void QCoolDownlaod::slotcmd_get_version(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_GET_VERSION;

    CMD_GET_VERSION.magic_header = 0x5AFF;
    CMD_GET_VERSION.msg_id = 0x0000;
    CMD_GET_VERSION.packet_num = 1;
    CMD_GET_VERSION.packet_cur = 0;
    CMD_GET_VERSION.msg_len = 0;
    CMD_GET_VERSION.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_GET_VERSION, cmd->length);
    
    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.sel_connect->setStyleSheet("TXCMD LIST is Full,Give Up the oldest package");
	    delete giveup;
    }
}


void QCoolDownlaod::slotcmd_get_setting(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_GET_SETTING;

    CMD_GET_SETTING.magic_header = 0x5AFF;
    CMD_GET_SETTING.msg_id = 0x005C;
    CMD_GET_SETTING.packet_num = 1;
    CMD_GET_SETTING.packet_cur = 0;
    CMD_GET_SETTING.msg_len = 0;
    CMD_GET_SETTING.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_GET_SETTING, cmd->length);

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.sel_connect->setStyleSheet("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotflydebug_start(void)
{
    thCoolflyMonitor.stopImmediately();
    thCoolflyMonitor.wait();

    thThreadCMDParse.stopImmediately();
    thThreadCMDParse.wait();

    thThreadFlyDebug.start();
    thThreadFlyDebugParse.start();

    
}



void QCoolDownlaod::slotflydebug_stop(void)
{
    thThreadFlyDebug.stopImmediately();
    thThreadFlyDebug.wait();

    thThreadFlyDebugParse.stopImmediately();
    thThreadFlyDebugParse.wait();

}