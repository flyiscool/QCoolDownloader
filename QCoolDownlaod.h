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
    
    QString fileNames;

public slots:
    void slotselfile(void);
    void slotupdateTextUi(QString);
    void slotconnectusb(void);
    void slotsetbt_connect_color(State_LED st);
    void slotcmd_get_version(void);
    void slotcmd_get_setting(void);
    void slotcmd_save_setting(void);
    void slotcmd_get_chipid(void);
    void slotcmd_start_cf_protocol(void);
    void slotcmd_take_pic(void);
    void slotcmd_video_start(void);
    void slotcmd_video_stop(void);
    void slotcmd_imu_cali(void);
    void slotcmd_mag_cali(void);
    void slotcmd_systemstate_rboot(void);

    void slotcmd_reboot(void);
    void slotcmd_usb_update(void);
    void slotcmd_usb_remote_update(void);

    void slotflydebug_start(void);
    void slotflydebug_stop(void);
    void slotupdate_factroy_setting(void);
    void slotchanged_factroy_setting(QTreeWidgetItem* item, int column);
    void slotupdateChipID(void);
    void slotnum_plus(void);
    void slotnum_minus(void);
    void slot_generate_chip_id(void);
    void slot_set_chip_id(void);
    void slot_clear_log(void);
private:
    Ui::QCoolDownlaodClass ui;
    CPThreadCoolflyMonitor	thCoolflyMonitor;
    CPThreadCMDParse	thThreadCMDParse;
    CPThreadFlyDebug	thThreadFlyDebug;
    CPThreadFlyDebugParse	thThreadFlyDebugParse;

    void set_factory_setting_treeWidget(void);
    void add_factory_setting_data(void);
    QTreeWidgetItem* addNodeRoot(QString name, QString value, QString des);
    void addNodeItem(QTreeWidgetItem* parent, QString name, QString value, QString des);
    void addItemValue(QTreeWidgetItem* parent, QString value, QString des);
    TAB_FACTORY_SET tab_fs;
};

