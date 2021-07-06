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
#include "cpUpdataApp.h"

extern threadsafe_queue<CMDBuffPackage*> gListTXCMD;
extern threadsafe_queue<CMDBuffPackage*> gListRXCMD;
extern STRU_FACTORY_SETTING cf_factroy_setting;
extern STRU_DEVICE_INFO cf_device_info;
extern param_t cf_params;
fly_state_v3_t g_fly_state;
//#define RELEASE_VERISON        1

QString int8_t2Str(uint8_t* data, int len, int filedwidth, int base);



void QCoolDownlaod::slotselfile(void)
{
	fileNames = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("bin(*.bin)"));

    ui.label_update->setText(fileNames);
	ui.textBrowser->setText(fileNames);

    ui.statusBar->showMessage(fileNames, 5000);
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
            
        thThreadFlyDebug.stopImmediately();
        thThreadFlyDebug.wait();

        thThreadFlyDebugParse.stopImmediately();
        thThreadFlyDebugParse.wait();

        thCoolflyMonitor.start();
        thThreadCMDParse.start();


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

#ifdef RELEASE_VERISON

    ui.bt_generate_chip_id->setDisabled(true);
    ui.bt_reboot->setDisabled(true);
    ui.bt_set_chip_id->setDisabled(true);

#endif // RELEASE_VERISON

    ui.tabWidget->setCurrentIndex(0);

    set_factory_setting_treeWidget();
    set_params_setting_treeWidget();

    init_params();

    connect(ui.sel_connect, SIGNAL(clicked()), this, SLOT(slotconnectusb()));

    connect(ui.sel_bt, SIGNAL(clicked()), this, SLOT(slotselfile()));

    connect(&thCoolflyMonitor, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadCMDParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebug, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebugParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));

    connect(&thThreadCMDParse, SIGNAL(signalupdateFlyState()), this, SLOT(slotupdateFlyState()));

    qRegisterMetaType<State_LED>("State_LED");
    connect(&thCoolflyMonitor, SIGNAL(signalupdateStateLED(State_LED)), this, SLOT(slotsetbt_connect_color(State_LED)));

    connect(&thCoolflyMonitor, SIGNAL(signalreboot()), this, SLOT(slotcmd_reboot()));
    connect(&thCoolflyMonitor, SIGNAL(signalremotereboot()), this, SLOT(slotcmd_systemstate_rboot()));

    connect(ui.sel_get_verison, SIGNAL(clicked()), this, SLOT(slotcmd_get_version()));
    connect(ui.bt_get_setting, SIGNAL(clicked()), this, SLOT(slotcmd_get_setting()));
    connect(ui.bt_save_setting, SIGNAL(clicked()), this, SLOT(slotcmd_save_setting()));
    connect(ui.bt_cf_protocol, SIGNAL(clicked()), this, SLOT(slotcmd_start_cf_protocol()));
    connect(ui.bt_cf_protocol_stop, SIGNAL(clicked()), this, SLOT(slotcmd_stop_cf_protocol()));
    connect(ui.bt_usb_update, SIGNAL(clicked()), this, SLOT(slotcmd_usb_update()));
    connect(ui.bt_usb_update_sky, SIGNAL(clicked()), this, SLOT(slotcmd_usb_remote_update()));

    connect(ui.bt_flydebugstart, SIGNAL(clicked()), this, SLOT(slotflydebug_start()));
    connect(ui.bt_flydebugstop, SIGNAL(clicked()), this, SLOT(slotflydebug_stop()));
    connect(ui.bt_reboot, SIGNAL(clicked()), this, SLOT(slotcmd_reboot()));
    connect(ui.bt_reboot_sky, SIGNAL(clicked()), this, SLOT(slotcmd_systemstate_rboot()));
    connect(ui.bt_get_chip_id, SIGNAL(clicked()), this, SLOT(slotcmd_get_chipid()));
    connect(ui.bt_num_plus, SIGNAL(clicked()), this, SLOT(slotnum_plus()));
    connect(ui.bt_num_mi, SIGNAL(clicked()), this, SLOT(slotnum_minus()));
    connect(ui.bt_generate_chip_id, SIGNAL(clicked()), this, SLOT(slot_generate_chip_id()));
    connect(ui.bt_set_chip_id, SIGNAL(clicked()), this, SLOT(slot_set_chip_id()));
    connect(ui.bt_clear_log, SIGNAL(clicked()), this, SLOT(slot_clear_log()));
    connect(ui.bt_take_pic, SIGNAL(clicked()), this, SLOT(slotcmd_take_pic()));
    connect(ui.bt_record_start, SIGNAL(clicked()), this, SLOT(slotcmd_video_start()));
    connect(ui.bt_record_stop, SIGNAL(clicked()), this, SLOT(slotcmd_video_stop()));
    connect(ui.bt_searchid_gnd, SIGNAL(clicked()), this, SLOT(slotcmd_searchid_gnd()));

    connect(ui.bt_imu_cali, SIGNAL(clicked()), this, SLOT(slotcmd_imu_cali()));
    connect(ui.bt_mag_cali, SIGNAL(clicked()), this, SLOT(slotcmd_mag_cali()));

    connect(ui.bt_set_home, SIGNAL(clicked()), this, SLOT(slotcmd_set_gps_home()));
    connect(ui.bt_armed, SIGNAL(clicked()), this, SLOT(slotcmd_set_armed()));
    connect(ui.bt_disarmed, SIGNAL(clicked()), this, SLOT(slotcmd_set_disarmed()));

    connect(&thThreadCMDParse, SIGNAL(signalupdateFactorySetting()), this, SLOT(slotupdate_factroy_setting()));
    connect(&thThreadCMDParse, SIGNAL(signalupdateChipID()), this, SLOT(slotupdateChipID()));


    connect(ui.bt_test_app, SIGNAL(clicked()), this, SLOT(slotcmd_test_app()));
    connect(ui.bt_test_app, SIGNAL(clicked()), this, SLOT(slotcmd_test_mission()));

    connect(ui.bt_get_params, SIGNAL(clicked()), this, SLOT(slotcmd_get_params()));
    connect(ui.bt_send_params, SIGNAL(clicked()), this, SLOT(slotcmd_send_params()));

    ui.comboBox_year->setCurrentIndex(3);

    thCoolflyMonitor.update_app_flag = false;

    ui.textBrowser->append("VERSION: V2.4  2021-02-06");

    qDebug() << "state-----------------------------------------------------------------------" << endl;
    qDebug() << "state" << sizeof(fly_state_v3_t) << endl;

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
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
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
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotcmd_save_setting(void)
{
    
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_SAVE_SETTING;
    uint8_t buff[4096];
    memset(buff,0, 4096);
    memcpy(buff, &cf_factroy_setting, sizeof(cf_factroy_setting));

    uint8_t pkt_num = sizeof(STRU_FACTORY_SETTING) / 502 + ((sizeof(STRU_FACTORY_SETTING) % 502 == 0) ? 0 : 1);
    uint32_t total_num = sizeof(STRU_FACTORY_SETTING);

    CMD_SAVE_SETTING.magic_header = 0x5AFF;
    CMD_SAVE_SETTING.msg_id = 0x005D;
    CMD_SAVE_SETTING.packet_num = pkt_num;
    CMD_SAVE_SETTING.packet_cur = 0;
    

    for (int i = 0; i < CMD_SAVE_SETTING.packet_num; i++)
    {
        CMDBuffPackage* cmd = new CMDBuffPackage;
        cmd->length = 0;
        CMD_SAVE_SETTING.chk_sum = 0;
        CMD_SAVE_SETTING.packet_cur = i;
        if (total_num > 502)
        {
            CMD_SAVE_SETTING.msg_len = 502;
            total_num -= 502;
        }
        else
        {
            CMD_SAVE_SETTING.msg_len = total_num ; //  对齐4字节
            //cmd->length = 2;
        }
        
        memcpy(&cmd->data[10], &buff[CMD_SAVE_SETTING.packet_cur * 502], CMD_SAVE_SETTING.msg_len);

        for (int j = 0; j < CMD_SAVE_SETTING.msg_len; j++)
        {
            CMD_SAVE_SETTING.chk_sum += cmd->data[10 + j];
        }
        
        memcpy(cmd->data, &CMD_SAVE_SETTING, 10);
    
        cmd->length = CMD_SAVE_SETTING.msg_len + 10;
        if (0 == gListTXCMD.push(cmd, giveup, 100))
        {
            ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
            delete giveup;
        }
    
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







void QCoolDownlaod::set_factory_setting_treeWidget(void)
{
    ui.factory_setting_treeWidget->setColumnCount(3);  //设置列

    ui.factory_setting_treeWidget->setColumnWidth(0, 200);
    ui.factory_setting_treeWidget->setColumnWidth(1, 150);
    ui.factory_setting_treeWidget->setColumnWidth(2, 150);
    
    QTextCodec* codec = QTextCodec::codecForName("GBK");

    tab_fs.top_labelList.append(codec->toUnicode("设置项"));
    tab_fs.top_labelList.append(codec->toUnicode("当前值"));
    tab_fs.top_labelList.append(codec->toUnicode("说明"));

    ui.factory_setting_treeWidget->setHeaderLabels(tab_fs.top_labelList);

    //添加顶层节点
    ui.factory_setting_treeWidget->insertTopLevelItems(0, tab_fs.items);
    //节点始终保持展开
    ui.factory_setting_treeWidget->setItemsExpandable(false);
    ui.factory_setting_treeWidget->expandAll();

    ui.factory_setting_treeWidget->setMouseTracking(true);

}

void QCoolDownlaod::slotchanged_factroy_setting(QTreeWidgetItem* item, int column)
{
    if (QString::compare(item->text(0), "Uart3Baudrate") == 0)
    {
        int uart3_baudrate = item->text(1).toInt();

        if ((uart3_baudrate > 10) || (uart3_baudrate<0))
        {
            uart3_baudrate = 4;
            item->setText(1, int8_t2Str( (uint8_t*)&uart3_baudrate, 1, 2, 16));
        }
        
        cf_factroy_setting.st_factorySetting.st_uartBaudData.st_uartBaudr[1] = uart3_baudrate;
        ui.textBrowser->append("uart3_baudrate changed");
    }

    
    if (QString::compare(item->text(0), "Uart3BypassMode") == 0)
    {
        int uart3_bypassmode = item->text(1).toInt();

        if ((uart3_bypassmode != 1) && (uart3_bypassmode != 5))
        {
            uart3_bypassmode = 1;
            item->setText(1, int8_t2Str((uint8_t*)&uart3_bypassmode, 1, 2, 16));
        }

        cf_factroy_setting.st_factorySetting.st_rfBandMcsData.bypass_ch_mode = (uint8_t)uart3_bypassmode;
        ui.textBrowser->append("uart3_bypassmode changed");
    }
    
}

void QCoolDownlaod::slotupdate_factroy_setting(void)
{
    disconnect(ui.factory_setting_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotchanged_factroy_setting(QTreeWidgetItem*, int)));
    ui.factory_setting_treeWidget->clear();
    add_factory_setting_data();
    connect(ui.factory_setting_treeWidget, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotchanged_factroy_setting(QTreeWidgetItem*, int)));
}
//new fac setting
QTreeWidgetItem* QCoolDownlaod::addNodeRoot(QString name, QString value, QString des)
{
	// QTreeWidgetItem(QTreeWidget * parent, int type = Type)
	QTreeWidgetItem* treeItem = new QTreeWidgetItem(ui.factory_setting_treeWidget);
    treeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	treeItem->setText(0, name);
	treeItem->setText(1, value);
	treeItem->setText(2, des);
	treeItem->setExpanded(true);
	return treeItem;
}



void QCoolDownlaod::addNodeItem(QTreeWidgetItem* parent, QString name, QString value, QString des)
{
	QTreeWidgetItem* treeItem = new QTreeWidgetItem();
	//QPlainTextEdit *lineEdit = new QPlainTextEdit();
	//lineEdit->insertPlainText(value);

	treeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	treeItem->setText(0, name);
	treeItem->setText(1, value);
	treeItem->setText(2, des);
	parent->addChild(treeItem);
	treeItem->setExpanded(true);
}

void QCoolDownlaod::addItemValue(QTreeWidgetItem* parent, QString value, QString des)
{
    QTreeWidgetItem* treeItem = new QTreeWidgetItem();
    treeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    treeItem->setText(1, value);
    treeItem->setText(2, des);
    parent->addChild(treeItem);
    treeItem->setExpanded(true);
}

QString int8_t2Str(uint8_t* data, int len, int filedwidth, int base)
{
	QString str;
	for (int i = 0; i < len; i++)
	{
		str.append(QString("%1").arg(data[i], filedwidth, base, QChar('0')).toUpper());
		str.append(" ");
	}
	str = str.trimmed();
	return str;
}

QString int16_t2Str(uint16_t* data, int len, int filedwidth, int base)
{
	QString str;
	for (int i = 0; i < len; i++)
	{
		str.append(QString("%1").arg(data[i], filedwidth, base, QChar('0')).toUpper());
		str.append(" ");
	}
	str = str.trimmed();
	return str;
}

QString sint16_t2Str(int16_t* data, int len, int filedwidth, int base)
{
	QString str;
	for (int i = 0; i < len; i++)
	{
		str.append(QString("%1").arg(data[i], filedwidth, base, QChar('0')).toUpper());
		str.append(" ");
	}
	str = str.trimmed();
	return str;
}

bool Str2int8_t(uint8_t* data, QString str, int base, int len)
{
	QList<uint8_t> u8list;
	QStringList strList = str.split(' ');
	if (strList.count() != len)
		return false;
	for (int i = 0; i < strList.count(); i++)
	{
		bool ok;
		uint8_t a = strList[i].toInt(&ok, base);
		if (!ok)
			return false;
		data[i] = a;
	}
	return true;
}

bool Str2int16_t(uint16_t* data, QString str, int base, int len)
{
	QList<uint16_t> u16list;
	QStringList strList = str.split(' ');
	if (strList.count() < len)
		return false;
	for (int i = 0; i < len; i++)
	{
		bool ok;
		uint16_t a = strList[i].toInt(&ok, base);
		if (!ok)
			return false;
		data[i] = a;
	}
	return true;
}

bool Str2sint16_t(int16_t* data, QString str, int base, int len)
{
	QList<int16_t> u16list;
	QStringList strList = str.split(' ');
	if (strList.count() < len)
		return false;
	for (int i = 0; i < len; i++)
	{
		bool ok;
		int16_t a = strList[i].toInt(&ok, base);
		if (!ok)
			return false;
		data[i] = a;
	}
	return true;
}



void QCoolDownlaod::add_factory_setting_data(void)
{
	
	//BB_AES_CFG_Node
    STRU_FACTORY_SETTING* fcData = &cf_factroy_setting;

	//BAUDR_CTRL_Node
    STRU_UART_BAUDR* m_BAUDR_CTRL_Node = &fcData->st_factorySetting.st_uartBaudData;
	QTreeWidgetItem* twiBAUDR_CTRL_Node = addNodeRoot("Uart3Baudrate", 
                    int8_t2Str(&m_BAUDR_CTRL_Node->st_uartBaudr[1], 1, 2, 16),
                    QString::fromLocal8Bit("0 :    9600 \n1 :   19200 \n2 :   38400 \n3 :   57600 \n4 :  115200\n10 : 3125000"));

    STRU_RF_BAND_MCS_OPT* m_RF_BAND_MCS_OPT = &fcData->st_factorySetting.st_rfBandMcsData;
    QTreeWidgetItem* twiRF_BAND_MCS_OPT_Node = addNodeRoot("Uart3BypassMode",
        int8_t2Str(&m_RF_BAND_MCS_OPT->bypass_ch_mode, 1, 2, 16),
        QString::fromLocal8Bit("1 :  Uart3 Bypass\n5 :  SKY Uart3 => Gnd USB"));

}




void QCoolDownlaod::slotcmd_reboot(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_REBOOT;

    CMD_REBOOT.magic_header = 0x5AFF;
    CMD_REBOOT.msg_id = 0x000A;
    CMD_REBOOT.packet_num = 1;
    CMD_REBOOT.packet_cur = 0;
    CMD_REBOOT.msg_len = 0;
    CMD_REBOOT.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_REBOOT, cmd->length);

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}



void QCoolDownlaod::slotcmd_get_chipid(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_GET_CHIPID;

    CMD_GET_CHIPID.magic_header = 0x5AFF;
    CMD_GET_CHIPID.msg_id = 0x0019;
    CMD_GET_CHIPID.packet_num = 1;
    CMD_GET_CHIPID.packet_cur = 0;
    CMD_GET_CHIPID.msg_len = 0;
    CMD_GET_CHIPID.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_GET_CHIPID, cmd->length);

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotupdateChipID(void)
{
    QString temp;
    temp.sprintf("CHIP ID : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X", 
          cf_device_info.chipId[0], 
          cf_device_info.chipId[1],
          cf_device_info.chipId[2], 
          cf_device_info.chipId[3], 
          cf_device_info.chipId[4]);
    ui.textBrowser->append(temp);

    temp.sprintf("RC ID : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
        cf_device_info.rcId[0],
        cf_device_info.rcId[1],
        cf_device_info.rcId[2],
        cf_device_info.rcId[3],
        cf_device_info.rcId[4]);
    ui.textBrowser->append(temp);

    temp.sprintf("VT ID : 0x%02X 0x%02X",
        cf_device_info.rcId[0],
        cf_device_info.rcId[1]);
    ui.textBrowser->append(temp);
}


void QCoolDownlaod::slotnum_minus(void)
{
    uint32_t  cnt = 0;
    cnt = ui.lineEdit->text().toInt();
    
    if (cnt > 0)
    {
        cnt--;
    }
    
    QString newCNT;
    newCNT.sprintf("%d", cnt);
    ui.lineEdit->setText(newCNT);
}

void QCoolDownlaod::slotnum_plus(void)
{
    uint32_t  cnt = 0;
    cnt = ui.lineEdit->text().toInt();

    if (cnt < 32768)
    {
        cnt++;
    }

    QString newCNT;
    newCNT.sprintf("%d", cnt);
    ui.lineEdit->setText(newCNT);
}

uint8_t g_cf_chip_id[5];
void QCoolDownlaod::slot_generate_chip_id(void)
{
    g_cf_chip_id[0] = 0x02;
    g_cf_chip_id[1] = (ui.comboBox_class->currentIndex() << 4 | ui.comboBox_year->currentIndex());

    g_cf_chip_id[2] = (ui.comboBox_month->currentIndex() << 4 | ui.comboBox_day->currentIndex() >> 1);
    g_cf_chip_id[3] = (ui.comboBox_day->currentIndex() << 7 | ui.lineEdit->text().toInt() >> 8);
    g_cf_chip_id[4] =  ui.lineEdit->text().toInt();

    QString temp;
    temp.sprintf("GENERATE CHIP ID : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
        g_cf_chip_id[0],
        g_cf_chip_id[1],
        g_cf_chip_id[2],
        g_cf_chip_id[3],
        g_cf_chip_id[4]);
    ui.textBrowser->append(temp);

}


void QCoolDownlaod::slot_set_chip_id(void)
{
    slot_generate_chip_id();

    QString temp;
    temp.sprintf("SET CHIP ID : 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
        g_cf_chip_id[0],
        g_cf_chip_id[1],
        g_cf_chip_id[2],
        g_cf_chip_id[3],
        g_cf_chip_id[4]);
    ui.textBrowser->append(temp);

    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_SET_CHIPID;

    CMD_SET_CHIPID.magic_header = 0x5AFF;
    CMD_SET_CHIPID.msg_id = 0x0066;
    CMD_SET_CHIPID.packet_num = 1;
    CMD_SET_CHIPID.packet_cur = 0;
    CMD_SET_CHIPID.msg_len = 5;
    CMD_SET_CHIPID.chk_sum = 0;
    cmd->length = 15;

    memset(cmd->data, 0, 512);
   
    cmd->data[10] = g_cf_chip_id[0];
    cmd->data[11] = g_cf_chip_id[1];
    cmd->data[12] = g_cf_chip_id[2];
    cmd->data[13] = g_cf_chip_id[3];
    cmd->data[14] = g_cf_chip_id[4];

    for (int j = 0; j < 5; j++)
    {
        CMD_SET_CHIPID.chk_sum += cmd->data[10 + j];
    }

    memcpy(cmd->data, &CMD_SET_CHIPID, sizeof(CMD_SET_CHIPID));

    qDebug() << "CMD_SET_CHIPID.chk_sum" << CMD_SET_CHIPID.chk_sum << endl;
    qDebug() << "cmd->data[10]" << cmd->data[10] << endl;
    qDebug() << "cmd->data[11]" << cmd->data[11] << endl;
    qDebug() << "cmd->data[12]" << cmd->data[12] << endl;
    qDebug() << "cmd->data[13]" << cmd->data[13] << endl;
    qDebug() << "cmd->data[14]" << cmd->data[14] << endl;

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slot_clear_log(void)
{
    ui.textBrowser->clear();
}




void QCoolDownlaod::slotcmd_start_cf_protocol(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = 0x0093;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 0;
    CMD_TX.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}



void QCoolDownlaod::slotcmd_stop_cf_protocol(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = 0x0093;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 1;
    CMD_TX.chk_sum = 0x90;
    cmd->length = 11;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    cmd->data[10] = 0x90;

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


/* 打包发送 */
void packToSend(uint8_t Cmd, uint8_t* p_data, uint32_t Length, uint8_t* p_buff) {

    uint8_t pChkSum = 0;

    /* 按照协议 填充数据 */
    Length += 6;
    
    // heard
    *p_buff = 0xA5;
    p_buff++;

    // length
    *p_buff = Length;
    p_buff++;

    // cmd
    *p_buff = Cmd;
    p_buff++;

    // msglength
    Length -= 4;
    *p_buff = Length;
    p_buff++;

    pChkSum = pChkSum ^ Cmd;
    pChkSum = pChkSum ^ Length;
    
    /**/
    Length -= 2;
    int k;
    for (k = 0; k < Length; k++) {
        pChkSum = pChkSum ^ *(p_data+k);
        *p_buff = *(p_data + k);
        p_buff++;
    }

    *p_buff = pChkSum;
    p_buff++;

    *p_buff = 0x5A;
}


void QCoolDownlaod::slotcmd_take_pic(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;
   

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_CAMERA_TAKE_PIC;

    packToSend(CF_PRO_MSGID_CAMERA, cf_payload,1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotcmd_video_start(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_CAMERA_RECORD_START;

    packToSend(CF_PRO_MSGID_CAMERA, cf_payload, 1, &cmd->data[10]);



    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;




    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

        for (int j = 0; j < 17; j++)
    {
        qDebug(" %d = %02x", j , cmd->data[j]); 

    }

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }



}

void QCoolDownlaod::slotcmd_video_stop(void) 
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_CAMERA_RECORD_STOP;

    packToSend(CF_PRO_MSGID_CAMERA, cf_payload, 1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));


    for (int j = 0; j < 17; j++)
    {
        qDebug(" %d = %02x", j, cmd->data[j]);

    }
    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_searchid_gnd(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_SEARCHID_GND;

    CMD_SEARCHID_GND.magic_header = 0x5AFF;
    CMD_SEARCHID_GND.msg_id = 0x0013;
    CMD_SEARCHID_GND.packet_num = 1;
    CMD_SEARCHID_GND.packet_cur = 0;
    CMD_SEARCHID_GND.msg_len = 0;
    CMD_SEARCHID_GND.chk_sum = 0;
    cmd->length = 10;

    memcpy(cmd->data, &CMD_SEARCHID_GND, cmd->length);

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotcmd_imu_cali(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_CALIBRATE_IMU;

    packToSend(CF_PRO_MSGID_CALIBRATE, cf_payload, 1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotcmd_mag_cali(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_CALIBRATE_MAG;

    packToSend(CF_PRO_MSGID_CALIBRATE, cf_payload, 1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_systemstate_rboot(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_REBOOT;

    packToSend(CF_PRO_MSGID_SYSTEM, cf_payload, 1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}

void QCoolDownlaod::slotcmd_usb_update(void)
{
    int32_t fsize;
    QString pri;
    //upgrade send data

    if (fileNames == NULL)
    {
        pri.sprintf("please select the app.bin file !!!!");
        ui.textBrowser->append(pri);
        return;
    }

    
    fsize = Get_File_Size(&fileNames);
    pri.sprintf("get file size: %d K", fsize/1024);
    ui.textBrowser->append(pri);

    if (fsize > 0x300000)   
    {
        pri.sprintf(" file is to big,not the app.bin");
        ui.textBrowser->append(pri);
        return;
    }

    thCoolflyMonitor.file = fileNames;
    thCoolflyMonitor.update_app_flag = true;
    
}



void QCoolDownlaod::slotcmd_usb_remote_update(void)
{
    int32_t fsize;
    QString pri;
    //upgrade send data

    if (fileNames == NULL)
    {
        pri.sprintf("please select the app.bin file !!!!");
        ui.textBrowser->append(pri);
        return;
    }

    fsize = Get_File_Size(&fileNames);
    pri.sprintf("get file size: %d K", fsize / 1024);
    ui.textBrowser->append(pri);

    if (fsize > 0x300000)
    {
        pri.sprintf(" file is to big,not the app.bin");
        ui.textBrowser->append(pri);
        return;
    }

    thCoolflyMonitor.file = fileNames;
    thCoolflyMonitor.update_remote_app_flag = true;
}

void QCoolDownlaod::slotupdateFlyState(void)
{
    QString pri;

    qDebug() << "g_fly_state.picth" << g_fly_state.picth;

    pri.sprintf("%0.2f", g_fly_state.picth);
    ui.label_ATTpitch_value->setText(pri);

    pri.sprintf("%0.2f", g_fly_state.roll);
    ui.label_ATTroll_value->setText(pri);

    pri.sprintf("%0.2f", g_fly_state.yaw);
    ui.label_ATTYaw_value->setText(pri);

    qDebug() << "g_fly_state.rc_is_failed" << g_fly_state.rc_is_failed;
    qDebug() << "g_fly_state.flymode" << g_fly_state.flymode;
    pri.sprintf("%d", g_fly_state.rc_is_failed);
    ui.label_HomeHead_value->setText(pri);
}


void QCoolDownlaod::slotcmd_set_gps_home(void)
{

    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_SET_GPS_HOME;

    packToSend(CF_PRO_MSGID_CTRL, cf_payload, 1, &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_set_armed(void)
{

    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[1];
    cf_payload[0] = MSGID_SET_ARMED;

    packToSend(CF_PRO_MSGID_CTRL, cf_payload, 1, &cmd->data[10]);



    CMD_TX.chk_sum = 0;
    for (int i = 0; i < 7; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = 7;
    cmd->length = 17;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_set_disarmed(void)
{

    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    uint8_t cf_payload[sizeof(app_t)];
    qDebug() << "sizeof(app_t)" << sizeof(app_t) <<endl;
    app_t test_app;

    

    memset(&test_app,0,sizeof(test_app));
    test_app.lat = 100;
    memcpy(cf_payload, &test_app, sizeof(app_t));

    packToSend(CF_PRO_MSGID_APP_CRTL, cf_payload, sizeof(app_t), &cmd->data[10]);

    for (int i = 0; i < sizeof(app_t)+6; i++)
    {
        qDebug() << cmd->data[10 + i];
    }


    CMD_TX.chk_sum = 0;
    for (int i = 0; i < sizeof(app_t) + 6; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = sizeof(app_t)+ 6;
    cmd->length = 10 + sizeof(app_t) + 6;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_test_app(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    app_t i_app_t;

    i_app_t.cali_level = 1;

    packToSend(CF_PRO_MSGID_APP_CRTL, (uint8_t*)&i_app_t, sizeof(app_t), &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < sizeof(app_t) + 6; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = sizeof(app_t) + 6;
    cmd->length = sizeof(app_t) + 6 + 10;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


void QCoolDownlaod::slotcmd_test_mission(void)
{
    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    mission_t i_mission;

    i_mission.count = 1;

    packToSend(CF_PRO_MSGID_APP_MISSION, (uint8_t*)&i_mission, sizeof(mission_t), &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < sizeof(mission_t) + 6; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = sizeof(mission_t) + 6;
    cmd->length = sizeof(mission_t) + 6 + 10;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }
}


//new fac setting
QTreeWidgetItem* QCoolDownlaod::addNodeRoot3(QString name, QString value0, QString value1, QString value2)
{
    // QTreeWidgetItem(QTreeWidget * parent, int type = Type)
    QTreeWidgetItem* treeItem = new QTreeWidgetItem(ui.params_setting);
    treeItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
    treeItem->setText(0, name);
    treeItem->setText(1, value0);
    treeItem->setText(2, value1);
    treeItem->setText(3, value2);
    treeItem->setExpanded(true);
    return treeItem;
}


void QCoolDownlaod::set_params_setting_treeWidget(void)
{
    ui.params_setting->setColumnCount(4);  //设置列

    ui.params_setting->setColumnWidth(0, 200);
    ui.params_setting->setColumnWidth(1, 150);
    ui.params_setting->setColumnWidth(2, 150);
    ui.params_setting->setColumnWidth(3, 150);
    
    QTextCodec* codec = QTextCodec::codecForName("GBK");
   
    tab_params.top_labelList.append(codec->toUnicode("设置项"));
    tab_params.top_labelList.append(codec->toUnicode("值0"));
    tab_params.top_labelList.append(codec->toUnicode("值1"));
    tab_params.top_labelList.append(codec->toUnicode("值2"));

    ui.params_setting->setHeaderLabels(tab_params.top_labelList);

    //添加顶层节点
    ui.params_setting->insertTopLevelItems(0, tab_params.items);
    //节点始终保持展开
    ui.params_setting->setItemsExpandable(false);
    ui.params_setting->expandAll();

    ui.params_setting->setMouseTracking(true);

}


void QCoolDownlaod::slotcmd_get_params(void)
{
    disconnect(ui.params_setting, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotchanged_params_setting(QTreeWidgetItem*, int)));
    
    //////////////////////////////////////////


    ui.params_setting->clear();
    
    param_t* param_Data = &cf_params;

    
    QString s_0 = QString("%1").arg(cf_params.att_p[0]);
    QString s_1 = QString("%1").arg(cf_params.att_p[1]);
    QString s_2 = QString("%1").arg(cf_params.att_p[2]);
    addNodeRoot3("att_p", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.rate_p[0]);
    s_1 = QString("%1").arg(cf_params.rate_p[1]);
    s_2 = QString("%1").arg(cf_params.rate_p[2]);
    addNodeRoot3("rate_p", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.rate_i[0]);
    s_1 = QString("%1").arg(cf_params.rate_i[1]);
    s_2 = QString("%1").arg(cf_params.rate_i[2]);
    addNodeRoot3("rate_i", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.rate_d[0]);
    s_1 = QString("%1").arg(cf_params.rate_d[1]);
    s_2 = QString("%1").arg(cf_params.rate_d[2]);
    addNodeRoot3("rate_d", s_0, s_1, s_2);
    
    s_0 = QString("%1").arg(cf_params.rate_ff[0]);
    s_1 = QString("%1").arg(cf_params.rate_ff[1]);
    s_2 = QString("%1").arg(cf_params.rate_ff[2]);
    addNodeRoot3("rate_ff", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.rate_int_lim[0]);
    s_1 = QString("%1").arg(cf_params.rate_int_lim[1]);
    s_2 = QString("%1").arg(cf_params.rate_int_lim[2]);
    addNodeRoot3("rate_int_lim", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.auto_rate_max[0]);
    s_1 = QString("%1").arg(cf_params.auto_rate_max[1]);
    s_2 = QString("%1").arg(cf_params.auto_rate_max[2]);
    addNodeRoot3("auto_rate_max", s_0, s_1, s_2);
    
    s_0 = QString("%1").arg(cf_params.mc_rate_max[0]);
    s_1 = QString("%1").arg(cf_params.mc_rate_max[1]);
    s_2 = QString("%1").arg(cf_params.mc_rate_max[2]);
    addNodeRoot3("mc_rate_max", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.thr_hover);
    s_1  = s_2 = "-";
    addNodeRoot3("thr_hover", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.thr_min);
    s_1 = s_2 = "-";
    addNodeRoot3("thr_min", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.thr_max);
    s_1 = s_2 = "-";
    addNodeRoot3("thr_max", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.tilt_max_air);
    s_1 = s_2 = "-";
    addNodeRoot3("tilt_max_air", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.tilt_max_land);
    s_1 = s_2 = "-";
    addNodeRoot3("tilt_max_land", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.tko_alt);
    s_1 = s_2 = "-";
    addNodeRoot3("tko_alt", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.tko_speed);
    s_1 = s_2 = "-";
    addNodeRoot3("tko_speed", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.land_speed);
    s_1 = s_2 = "-";
    addNodeRoot3("land_speed", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.battery_low1);
    s_1 = s_2 = "-";
    addNodeRoot3("battery_low1", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.battery_low2);
    s_1 = s_2 = "-";
    addNodeRoot3("battery_low2", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.battery_critical);
    s_1 = s_2 = "-";
    addNodeRoot3("battery_critical", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.pos_p[0]);
    s_1 = QString("%1").arg(cf_params.pos_p[1]);
    s_2 = QString("%1").arg(cf_params.pos_p[2]);
    addNodeRoot3("pos_p", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_p[0]);
    s_1 = QString("%1").arg(cf_params.vel_p[1]);
    s_2 = QString("%1").arg(cf_params.vel_p[2]);
    addNodeRoot3("vel_p", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_i[0]);
    s_1 = QString("%1").arg(cf_params.vel_i[1]);
    s_2 = QString("%1").arg(cf_params.vel_i[2]);
    addNodeRoot3("vel_i", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_d[0]);
    s_1 = QString("%1").arg(cf_params.vel_d[1]);
    s_2 = QString("%1").arg(cf_params.vel_d[2]);
    addNodeRoot3("vel_d", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_ff[0]);
    s_1 = QString("%1").arg(cf_params.vel_ff[1]);
    s_2 = QString("%1").arg(cf_params.vel_ff[2]);
    addNodeRoot3("vel_ff", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_i_max[0]);
    s_1 = QString("%1").arg(cf_params.vel_i_max[1]);
    s_2 = QString("%1").arg(cf_params.vel_i_max[2]);
    addNodeRoot3("vel_i_max", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.vel_hor_max);
    s_1 = s_2 = "-";
    addNodeRoot3("vel_hor_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_down_max);
    s_1 = s_2 = "-";
    addNodeRoot3("vel_down_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_up_max);
    s_1 = s_2 = "-";
    addNodeRoot3("vel_up_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.dec_hor_max);
    s_1 = s_2 = "-";
    addNodeRoot3("dec_hor_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_hor_max);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_hor_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_z_p);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_z_p", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_z_i);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_z_i", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_z_d);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_z_d", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_up_max);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_up_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.acc_down_max);
    s_1 = s_2 = "-";
    addNodeRoot3("acc_down_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.man_vel_xy_max);
    s_1 = s_2 = "-";
    addNodeRoot3("man_vel_xy_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.man_tilt_max);
    s_1 = s_2 = "-";
    addNodeRoot3("man_tilt_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.man_yaw_max);
    s_1 = s_2 = "-";
    addNodeRoot3("man_yaw_max", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.global_yaw_max);
    s_1 = s_2 = "-";
    addNodeRoot3("global_yaw_max", s_0, s_1, s_2);


    //s_0 = QString("%1").arg(cf_params.gyro_offset[0]);
    //s_1 = QString("%1").arg(cf_params.gyro_offset[1]);
    //s_2 = QString("%1").arg(cf_params.gyro_offset[2]);
    //addNodeRoot3("gyro_offset", s_0, s_1, s_2);

    //s_0 = QString("%1").arg(cf_params.gyro_scale[0]);
    //s_1 = QString("%1").arg(cf_params.gyro_scale[1]);
    //s_2 = QString("%1").arg(cf_params.gyro_scale[2]);
    //addNodeRoot3("gyro_scale", s_0, s_1, s_2);


    //s_0 = QString("%1").arg(cf_params.accel_offset[0]);
    //s_1 = QString("%1").arg(cf_params.accel_offset[1]);
    //s_2 = QString("%1").arg(cf_params.accel_offset[2]);
    //addNodeRoot3("accel_offset", s_0, s_1, s_2);

    //s_0 = QString("%1").arg(cf_params.accel_scale[0]);
    //s_1 = QString("%1").arg(cf_params.accel_scale[1]);
    //s_2 = QString("%1").arg(cf_params.accel_scale[2]);
    //addNodeRoot3("accel_scale", s_0, s_1, s_2);

    //s_0 = QString("%1").arg(cf_params.mag_offset[0]);
    //s_1 = QString("%1").arg(cf_params.mag_offset[1]);
    //s_2 = QString("%1").arg(cf_params.mag_offset[2]);
    //addNodeRoot3("mag_offset", s_0, s_1, s_2);

    //s_0 = QString("%1").arg(cf_params.mag_scale[0]);
    //s_1 = QString("%1").arg(cf_params.mag_scale[1]);
    //s_2 = QString("%1").arg(cf_params.mag_scale[2]);
    //addNodeRoot3("mag_scale", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.roll_offset);
    s_1 = s_2 = "-";
    addNodeRoot3("roll_offset", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.pitch_offset);
    s_1 = s_2 = "-";
    addNodeRoot3("pitch_offset", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.limit_altitude);
    s_1 = s_2 = "-";
    addNodeRoot3("limit_altitude", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.limit_altitude_rtl);
    s_1 = s_2 = "-";
    addNodeRoot3("limit_altitude_rtl", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.limit_distance);
    s_1 = s_2 = "-";
    addNodeRoot3("limit_distance", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.limit_newer);
    s_1 = s_2 = "-";
    addNodeRoot3("limit_newer", s_0, s_1, s_2);


    s_0 = QString("%1").arg(cf_params.man_tilt_max_scale[0]);
    s_1 = QString("%1").arg(cf_params.man_tilt_max_scale[1]);
    s_2 = QString("%1").arg(cf_params.man_tilt_max_scale[2]);
    addNodeRoot3("man_tilt_max_scale", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.man_yaw_max_scale[0]);
    s_1 = QString("%1").arg(cf_params.man_yaw_max_scale[1]);
    s_2 = QString("%1").arg(cf_params.man_yaw_max_scale[2]);
    addNodeRoot3("man_yaw_max_scale", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_up_max_scale[0]);
    s_1 = QString("%1").arg(cf_params.vel_up_max_scale[1]);
    s_2 = QString("%1").arg(cf_params.vel_up_max_scale[2]);
    addNodeRoot3("man_yaw_max_scale", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_down_max_scale[0]);
    s_1 = QString("%1").arg(cf_params.vel_down_max_scale[1]);
    s_2 = QString("%1").arg(cf_params.vel_down_max_scale[2]);
    addNodeRoot3("vel_down_max_scale", s_0, s_1, s_2);

    s_0 = QString("%1").arg(cf_params.vel_hor_max_scale[0]);
    s_1 = QString("%1").arg(cf_params.vel_hor_max_scale[1]);
    s_2 = QString("%1").arg(cf_params.vel_hor_max_scale[2]);
    addNodeRoot3("vel_hor_max_scale", s_0, s_1, s_2);


    //////////////////////////////////////////////

    connect(ui.params_setting, SIGNAL(itemChanged(QTreeWidgetItem*, int)), this, SLOT(slotchanged_params_setting(QTreeWidgetItem*, int)));

}


void QCoolDownlaod::slotcmd_send_params(void)
{
    qDebug() << " param1_t "<< sizeof(param1_t) << endl;

    CMDBuffPackage* cmd = new CMDBuffPackage;
    CMDBuffPackage* giveup = NULL;
    STRU_WIRELESS_MSG_HEADER CMD_TX;

    param1_t i_param1;


    memcpy(&i_param1,&cf_params,sizeof(i_param1));

    packToSend(CF_MSGID_PARAMS1, (uint8_t*)&i_param1, sizeof(param1_t), &cmd->data[10]);


    CMD_TX.chk_sum = 0;
    for (int i = 0; i < sizeof(i_param1) + 6; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = sizeof(i_param1) + 6;
    cmd->length = sizeof(i_param1) + 6 + 10;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }


    //////////////////////////////////////////////////////////////////////

    qDebug() << " param2_t " << sizeof(param2_t) << endl;

    cmd = new CMDBuffPackage;

    param2_t i_param2;
    memcpy(&i_param2, &cf_params + sizeof(i_param2), sizeof(i_param2));

    packToSend(CF_MSGID_PARAMS2, (uint8_t*)&i_param2, sizeof(param2_t), &cmd->data[10]);

    CMD_TX.chk_sum = 0;
    for (int i = 0; i < sizeof(i_param2) + 6; i++)
    {
        CMD_TX.chk_sum += cmd->data[10 + i];
    }

    CMD_TX.magic_header = 0x5AFF;
    CMD_TX.msg_id = CMD_CF_PROTOCOL_TX;
    CMD_TX.packet_num = 1;
    CMD_TX.packet_cur = 0;
    CMD_TX.msg_len = sizeof(i_param2) + 6;
    cmd->length = sizeof(i_param2) + 6 + 10;

    memcpy(cmd->data, &CMD_TX, sizeof(CMD_TX));

    if (0 == gListTXCMD.push(cmd, giveup, 100))
    {
        ui.textBrowser->append("TXCMD LIST is Full,Give Up the oldest package");
        delete giveup;
    }

}

void QCoolDownlaod::init_params(void)
{
    param_t* p = &cf_params;

    p->magic_head = 0x20171007;

    // attitude control
    p->att_p[0] = 5.0f;
    p->att_p[1] = 5.0f;
    p->att_p[2] = 4.0f;

    p->rate_p[0] = 0.15f;
    p->rate_p[1] = 0.15f;
    p->rate_p[2] = 0.25f;

    p->rate_i[0] = 0.05f;
    p->rate_i[1] = 0.05f;
    p->rate_i[2] = 0.20f;

    p->rate_d[0] = 0.005f;
    p->rate_d[1] = 0.005f;
    p->rate_d[2] = 0.0f;
    
    p->rate_ff[0] = 0.0f;
    p->rate_ff[1] = 0.0f;
    p->rate_ff[2] = 0.00f;

    p->rate_int_lim[0] = 0.3f;
    p->rate_int_lim[1] = 0.3f;
    p->rate_int_lim[2] = 0.3f;

    p->auto_rate_max[0] = 3.8f;
    p->auto_rate_max[1] = 3.8f;
    p->auto_rate_max[2] = 4.5f;

    p->mc_rate_max[0] = 3.8f;
    p->mc_rate_max[1] = 3.8f;
    p->mc_rate_max[2] = 4.5f;

    // position control
    p->thr_hover = 0.55f;
    p->thr_min = 0.10f;
    p->thr_max = 0.90f;

    p->tilt_max_air = 0.52f;
    p->tilt_max_land = 0.20f;

    p->tko_alt = 0.8f;
    p->tko_speed = 0.5f;

    p->land_speed = 0.6f;

    p->battery_low1 = 11.5f;
    p->battery_low2 = 11.0f;
    p->battery_critical = 10.5f;

    p->pos_p[0] = 1.0f;
    p->pos_p[1] = 1.0f;
    p->pos_p[2] = 1.0f;

    p->vel_p[0] = 0.15f;
    p->vel_p[1] = 0.15f;
    p->vel_p[2] = 0.5f;

    p->vel_i[0] = 0.01f;
    p->vel_i[1] = 0.01f;
    p->vel_i[2] = 0.1f;

    p->vel_d[0] = 0.00f;
    p->vel_d[1] = 0.00f;
    p->vel_d[2] = 0.0f;

    p->vel_ff[0] = 0.002f;
    p->vel_ff[1] = 0.002f;
    p->vel_ff[2] = 0.0f;

    p->vel_i_max[0] = 0.3f;
    p->vel_i_max[1] = 0.3f;
    p->vel_i_max[2] = 0.3f;

    p->vel_hor_max = 8.0f,
    p->vel_down_max = 2.0f,
    p->vel_up_max = 2.0f,

    p->dec_hor_max = 10.0f,
    p->acc_hor_max = 10.0f,

    p->acc_z_p = 3.5f;
    p->acc_z_i = 0.012f;
    p->acc_z_d = 0.0f;
    p->acc_up_max = 4.0f;
    p->acc_down_max = 8.0f;

    p->man_vel_xy_max = 2.0f;
    p->man_tilt_max = 0.6f;
    p->man_yaw_max = 3.14f;

    p->global_yaw_max = 3.14f;

    // sensor calibration

    p->gyro_offset[0] = 0.0f;
    p->gyro_offset[1] = 0.0f;
    p->gyro_offset[2] = 0.0f;

    p->gyro_scale[0] = 1.0f;
    p->gyro_scale[1] = 1.0f;
    p->gyro_scale[2] = 1.0f;

    p->accel_offset[0] = 0.0f;
    p->accel_offset[1] = 0.0f;
    p->accel_offset[2] = 0.0f;

    p->accel_scale[0] = 1.0f;
    p->accel_scale[1] = 1.0f;
    p->accel_scale[2] = 1.0f;

    p->mag_offset[0] = 0.0f;
    p->mag_offset[1] = 0.0f;
    p->mag_offset[2] = 0.0f;

    p->mag_scale[0] = 1.0f;
    p->mag_scale[1] = 1.0f;
    p->mag_scale[2] = 1.0f;

    // sensor offset
    p->roll_offset = 0.0f;
    p->pitch_offset = 0.0f;

    // limits

    p->limit_altitude = 150.0f;
    p->limit_altitude_rtl = 30.0f;
    p->limit_distance = 30.0f;
    p->limit_newer = 1;

    // manual control scale

    p->man_tilt_max_scale[0] = 0.3f;
    p->man_tilt_max_scale[1] = 0.3f;
    p->man_tilt_max_scale[2] = 0.3f;
    
    p->man_yaw_max_scale[0] = 1.2f;
    p->man_yaw_max_scale[1] = 1.2f;
    p->man_yaw_max_scale[2] = 1.2f;

    p->vel_up_max_scale[0] = 1.5f;
    p->vel_up_max_scale[1] = 1.5f;
    p->vel_up_max_scale[2] = 1.5f;

    p->vel_down_max_scale[0] = 2.0f;
    p->vel_down_max_scale[1] = 2.0f;
    p->vel_down_max_scale[2] = 2.0f;


    p->vel_hor_max_scale[0] = 2.0f;
    p->vel_hor_max_scale[1] = 2.0f;
    p->vel_hor_max_scale[2] = 2.0f;

    p->magic_end = 0x20171007;
}





void QCoolDownlaod::slotchanged_params_setting(QTreeWidgetItem* item, int column)
{
    if (QString::compare(item->text(0), "att_p") == 0)
    {
        float att_p[3];

        att_p[0] = item->text(1).toFloat();
        att_p[1] = item->text(2).toFloat();
        att_p[2] = item->text(3).toFloat();

        cf_params.att_p[0] = att_p[0];
        cf_params.att_p[1] = att_p[1];
        cf_params.att_p[2] = att_p[2];
        ui.textBrowser->append("att_p changed");
    }

    if (QString::compare(item->text(0), "rate_p") == 0)
    {
        float rate_p[3];

        rate_p[0] = item->text(1).toFloat();
        rate_p[1] = item->text(2).toFloat();
        rate_p[2] = item->text(3).toFloat();

        cf_params.rate_p[0] = rate_p[0];
        cf_params.rate_p[1] = rate_p[1];
        cf_params.rate_p[2] = rate_p[2];
        ui.textBrowser->append("rate_p changed");
    }


    if (QString::compare(item->text(0), "rate_i") == 0)
    {
        float rate_i[3];

        rate_i[0] = item->text(1).toFloat();
        rate_i[1] = item->text(2).toFloat();
        rate_i[2] = item->text(3).toFloat();

        cf_params.rate_i[0] = rate_i[0];
        cf_params.rate_i[1] = rate_i[1];
        cf_params.rate_i[2] = rate_i[2];
        ui.textBrowser->append("rate_i changed");
    }

    if (QString::compare(item->text(0), "rate_d") == 0)
    {
        float rate_d[3];

        rate_d[0] = item->text(1).toFloat();
        rate_d[1] = item->text(2).toFloat();
        rate_d[2] = item->text(3).toFloat();

        cf_params.rate_d[0] = rate_d[0];
        cf_params.rate_d[1] = rate_d[1];
        cf_params.rate_d[2] = rate_d[2];
        ui.textBrowser->append("rate_d changed");
    }


    if (QString::compare(item->text(0), "rate_ff") == 0)
    {
        float rate_ff[3];

        rate_ff[0] = item->text(1).toFloat();
        rate_ff[1] = item->text(2).toFloat();
        rate_ff[2] = item->text(3).toFloat();

        cf_params.rate_d[0] = rate_ff[0];
        cf_params.rate_d[1] = rate_ff[1];
        cf_params.rate_d[2] = rate_ff[2];
        ui.textBrowser->append("rate_ff changed");
    }

    if (QString::compare(item->text(0), "rate_int_lim") == 0)
    {
        float rate_int_lim[3];

        rate_int_lim[0] = item->text(1).toFloat();
        rate_int_lim[1] = item->text(2).toFloat();
        rate_int_lim[2] = item->text(3).toFloat();

        cf_params.rate_int_lim[0] = rate_int_lim[0];
        cf_params.rate_int_lim[1] = rate_int_lim[1];
        cf_params.rate_int_lim[2] = rate_int_lim[2];
        ui.textBrowser->append("rate_int_lim changed");
    }

    if (QString::compare(item->text(0), "auto_rate_max") == 0)
    {
        float auto_rate_max[3];

        auto_rate_max[0] = item->text(1).toFloat();
        auto_rate_max[1] = item->text(2).toFloat();
        auto_rate_max[2] = item->text(3).toFloat();

        cf_params.auto_rate_max[0] = auto_rate_max[0];
        cf_params.auto_rate_max[1] = auto_rate_max[1];
        cf_params.auto_rate_max[2] = auto_rate_max[2];
        ui.textBrowser->append("auto_rate_max changed");
    }

    if (QString::compare(item->text(0), "mc_rate_max") == 0)
    {
        float mc_rate_max[3];

        mc_rate_max[0] = item->text(1).toFloat();
        mc_rate_max[1] = item->text(2).toFloat();
        mc_rate_max[2] = item->text(3).toFloat();

        cf_params.mc_rate_max[0] = mc_rate_max[0];
        cf_params.mc_rate_max[1] = mc_rate_max[1];
        cf_params.mc_rate_max[2] = mc_rate_max[2];
        ui.textBrowser->append("mc_rate_max changed");
    }

    if (QString::compare(item->text(0), "thr_hover") == 0)
    {
        float thr_hover;
        thr_hover = item->text(1).toFloat();
        cf_params.thr_hover = thr_hover;
        ui.textBrowser->append("thr_hover changed");
    }

    if (QString::compare(item->text(0), "thr_min") == 0)
    {
        float thr_min;
        thr_min = item->text(1).toFloat();
        cf_params.thr_min = thr_min;
        ui.textBrowser->append("thr_min changed");
    }

    if (QString::compare(item->text(0), "thr_max") == 0)
    {
        float thr_max;
        thr_max = item->text(1).toFloat();
        cf_params.thr_max = thr_max;
        ui.textBrowser->append("thr_max changed");
    }

    if (QString::compare(item->text(0), "tilt_max_air") == 0)
    {
        float tilt_max_air;
        tilt_max_air = item->text(1).toFloat();
        cf_params.tilt_max_air = tilt_max_air;
        ui.textBrowser->append("tilt_max_air changed");
    }

    if (QString::compare(item->text(0), "tilt_max_land") == 0)
    {
        float tilt_max_land;
        tilt_max_land = item->text(1).toFloat();
        cf_params.tilt_max_land = tilt_max_land;
        ui.textBrowser->append("tilt_max_land changed");
    }

    if (QString::compare(item->text(0), "tko_alt") == 0)
    {
        float tko_alt;
        tko_alt = item->text(1).toFloat();
        cf_params.tko_alt = tko_alt;
        ui.textBrowser->append("tko_alt changed");
    }

    if (QString::compare(item->text(0), "tko_speed") == 0)
    {
        float tko_speed;
        tko_speed = item->text(1).toFloat();
        cf_params.tko_speed = tko_speed;
        ui.textBrowser->append("tko_speed changed");
    }

    if (QString::compare(item->text(0), "land_speed") == 0)
    {
        float land_speed;
        land_speed = item->text(1).toFloat();
        cf_params.land_speed = land_speed;
        ui.textBrowser->append("land_speed changed");
    }

    if (QString::compare(item->text(0), "battery_low1") == 0)
    {
        float battery_low1;
        battery_low1 = item->text(1).toFloat();
        cf_params.battery_low1 = battery_low1;
        ui.textBrowser->append("battery_low1 changed");
    }

    if (QString::compare(item->text(0), "battery_low2") == 0)
    {
        float battery_low2;
        battery_low2 = item->text(1).toFloat();
        cf_params.battery_low2 = battery_low2;
        ui.textBrowser->append("battery_low2 changed");
    }

    if (QString::compare(item->text(0), "battery_critical") == 0)
    {
        float battery_critical;
        battery_critical = item->text(1).toFloat();
        cf_params.battery_critical = battery_critical;
        ui.textBrowser->append("battery_critical changed");
    }

    if (QString::compare(item->text(0), "pos_p") == 0)
    {
        float pos_p[3];

        pos_p[0] = item->text(1).toFloat();
        pos_p[1] = item->text(2).toFloat();
        pos_p[2] = item->text(3).toFloat();

        cf_params.pos_p[0] = pos_p[0];
        cf_params.pos_p[1] = pos_p[1];
        cf_params.pos_p[2] = pos_p[2];
        ui.textBrowser->append("pos_p changed");
    }

    if (QString::compare(item->text(0), "vel_p") == 0)
    {
        float vel_p[3];

        vel_p[0] = item->text(1).toFloat();
        vel_p[1] = item->text(2).toFloat();
        vel_p[2] = item->text(3).toFloat();

        cf_params.vel_p[0] = vel_p[0];
        cf_params.vel_p[1] = vel_p[1];
        cf_params.vel_p[2] = vel_p[2];
        ui.textBrowser->append("vel_p changed");
    }

    if (QString::compare(item->text(0), "vel_i") == 0)
    {
        float vel_i[3];

        vel_i[0] = item->text(1).toFloat();
        vel_i[1] = item->text(2).toFloat();
        vel_i[2] = item->text(3).toFloat();

        cf_params.vel_i[0] = vel_i[0];
        cf_params.vel_i[1] = vel_i[1];
        cf_params.vel_i[2] = vel_i[2];
        ui.textBrowser->append("vel_i changed");
    }

    if (QString::compare(item->text(0), "vel_d") == 0)
    {
        float vel_d[3];

        vel_d[0] = item->text(1).toFloat();
        vel_d[1] = item->text(2).toFloat();
        vel_d[2] = item->text(3).toFloat();

        cf_params.vel_d[0] = vel_d[0];
        cf_params.vel_d[1] = vel_d[1];
        cf_params.vel_d[2] = vel_d[2];
        ui.textBrowser->append("vel_d changed");
    }

    if (QString::compare(item->text(0), "vel_ff") == 0)
    {
        float vel_ff[3];

        vel_ff[0] = item->text(1).toFloat();
        vel_ff[1] = item->text(2).toFloat();
        vel_ff[2] = item->text(3).toFloat();

        cf_params.vel_ff[0] = vel_ff[0];
        cf_params.vel_ff[1] = vel_ff[1];
        cf_params.vel_ff[2] = vel_ff[2];
        ui.textBrowser->append("vel_ff changed");
    }

    if (QString::compare(item->text(0), "vel_i_max") == 0)
    {
        float vel_i_max[3];

        vel_i_max[0] = item->text(1).toFloat();
        vel_i_max[1] = item->text(2).toFloat();
        vel_i_max[2] = item->text(3).toFloat();

        cf_params.vel_i_max[0] = vel_i_max[0];
        cf_params.vel_i_max[1] = vel_i_max[1];
        cf_params.vel_i_max[2] = vel_i_max[2];
        ui.textBrowser->append("vel_i_max changed");
    }


    if (QString::compare(item->text(0), "vel_hor_max") == 0)
    {
        float vel_hor_max;
        vel_hor_max = item->text(1).toFloat();
        cf_params.vel_hor_max = vel_hor_max;
        ui.textBrowser->append("vel_hor_max changed");
    }

    if (QString::compare(item->text(0), "vel_down_max") == 0)
    {
        float vel_down_max;
        vel_down_max = item->text(1).toFloat();
        cf_params.vel_down_max = vel_down_max;
        ui.textBrowser->append("vel_down_max changed");
    }

    if (QString::compare(item->text(0), "vel_up_max") == 0)
    {
        float vel_up_max;
        vel_up_max = item->text(1).toFloat();
        cf_params.vel_up_max = vel_up_max;
        ui.textBrowser->append("vel_up_max changed");
    }

    if (QString::compare(item->text(0), "dec_hor_max") == 0)
    {
        float dec_hor_max;
        dec_hor_max = item->text(1).toFloat();
        cf_params.dec_hor_max = dec_hor_max;
        ui.textBrowser->append("dec_hor_max changed");
    }

    if (QString::compare(item->text(0), "acc_hor_max") == 0)
    {
        float acc_hor_max;
        acc_hor_max = item->text(1).toFloat();
        cf_params.acc_hor_max = acc_hor_max;
        ui.textBrowser->append("acc_hor_max changed");
    }

    if (QString::compare(item->text(0), "acc_z_p") == 0)
    {
        float acc_z_p;
        acc_z_p = item->text(1).toFloat();
        cf_params.acc_z_p = acc_z_p;
        ui.textBrowser->append("acc_z_p changed");
    }

    if (QString::compare(item->text(0), "acc_z_i") == 0)
    {
        float acc_z_i;
        acc_z_i = item->text(1).toFloat();
        cf_params.acc_z_i = acc_z_i;
        ui.textBrowser->append("acc_z_i changed");
    }

    if (QString::compare(item->text(0), "acc_z_d") == 0)
    {
        float acc_z_d;
        acc_z_d = item->text(1).toFloat();
        cf_params.acc_z_d = acc_z_d;
        ui.textBrowser->append("acc_z_d changed");
    }

    if (QString::compare(item->text(0), "acc_up_max") == 0)
    {
        float acc_up_max;
        acc_up_max = item->text(1).toFloat();
        cf_params.acc_up_max = acc_up_max;
        ui.textBrowser->append("acc_up_max changed");
    }

    if (QString::compare(item->text(0), "acc_down_max") == 0)
    {
        float acc_down_max;
        acc_down_max = item->text(1).toFloat();
        cf_params.acc_down_max = acc_down_max;
        ui.textBrowser->append("acc_down_max changed");
    }

    if (QString::compare(item->text(0), "man_vel_xy_max") == 0)
    {
        float man_vel_xy_max;
        man_vel_xy_max = item->text(1).toFloat();
        cf_params.man_vel_xy_max = man_vel_xy_max;
        ui.textBrowser->append("man_vel_xy_max changed");
    }


    if (QString::compare(item->text(0), "man_tilt_max") == 0)
    {
        float man_tilt_max;
        man_tilt_max = item->text(1).toFloat();
        cf_params.man_tilt_max = man_tilt_max;
        ui.textBrowser->append("man_tilt_max changed");
    }

    if (QString::compare(item->text(0), "man_yaw_max") == 0)
    {
        float man_yaw_max;
        man_yaw_max = item->text(1).toFloat();
        cf_params.man_yaw_max = man_yaw_max;
        ui.textBrowser->append("man_yaw_max changed");
    }

    if (QString::compare(item->text(0), "global_yaw_max") == 0)
    {
        float global_yaw_max;
        global_yaw_max = item->text(1).toFloat();
        cf_params.global_yaw_max = global_yaw_max;
        ui.textBrowser->append("global_yaw_max changed");
    }

    if (QString::compare(item->text(0), "limit_altitude") == 0)
    {
        float limit_altitude;
        limit_altitude = item->text(1).toFloat();
        cf_params.limit_altitude = limit_altitude;
        ui.textBrowser->append("limit_altitude changed");
    }

    if (QString::compare(item->text(0), "limit_altitude_rtl") == 0)
    {
        float limit_altitude_rtl;
        limit_altitude_rtl = item->text(1).toFloat();
        cf_params.limit_altitude_rtl = limit_altitude_rtl;
        ui.textBrowser->append("limit_altitude_rtl changed");
    }

    if (QString::compare(item->text(0), "limit_distance") == 0)
    {
        float limit_distance;
        limit_distance = item->text(1).toFloat();
        cf_params.limit_distance = limit_distance;
        ui.textBrowser->append("limit_distance changed");
    }

    if (QString::compare(item->text(0), "limit_newer") == 0)
    {
        float limit_newer;
        limit_newer = item->text(1).toFloat();
        cf_params.limit_newer = limit_newer;
        ui.textBrowser->append("limit_newer changed");
    }


    if (QString::compare(item->text(0), "man_tilt_max_scale") == 0)
    {
        float man_tilt_max_scale[3];

        man_tilt_max_scale[0] = item->text(1).toFloat();
        man_tilt_max_scale[1] = item->text(2).toFloat();
        man_tilt_max_scale[2] = item->text(3).toFloat();

        cf_params.man_tilt_max_scale[0] = man_tilt_max_scale[0];
        cf_params.man_tilt_max_scale[1] = man_tilt_max_scale[1];
        cf_params.man_tilt_max_scale[2] = man_tilt_max_scale[2];
        ui.textBrowser->append("man_tilt_max_scale changed");
    }


    if (QString::compare(item->text(0), "man_yaw_max_scale") == 0)
    {
        float man_yaw_max_scale[3];

        man_yaw_max_scale[0] = item->text(1).toFloat();
        man_yaw_max_scale[1] = item->text(2).toFloat();
        man_yaw_max_scale[2] = item->text(3).toFloat();

        cf_params.man_yaw_max_scale[0] = man_yaw_max_scale[0];
        cf_params.man_yaw_max_scale[1] = man_yaw_max_scale[1];
        cf_params.man_yaw_max_scale[2] = man_yaw_max_scale[2];
        ui.textBrowser->append("man_yaw_max_scale changed");
    }

    if (QString::compare(item->text(0), "vel_up_max_scale") == 0)
    {
        float vel_up_max_scale[3];

        vel_up_max_scale[0] = item->text(1).toFloat();
        vel_up_max_scale[1] = item->text(2).toFloat();
        vel_up_max_scale[2] = item->text(3).toFloat();

        cf_params.vel_up_max_scale[0] = vel_up_max_scale[0];
        cf_params.vel_up_max_scale[1] = vel_up_max_scale[1];
        cf_params.vel_up_max_scale[2] = vel_up_max_scale[2];
        ui.textBrowser->append("vel_up_max_scale changed");
    }

    if (QString::compare(item->text(0), "vel_down_max_scale") == 0)
    {
        float vel_down_max_scale[3];

        vel_down_max_scale[0] = item->text(1).toFloat();
        vel_down_max_scale[1] = item->text(2).toFloat();
        vel_down_max_scale[2] = item->text(3).toFloat();

        cf_params.vel_down_max_scale[0] = vel_down_max_scale[0];
        cf_params.vel_down_max_scale[1] = vel_down_max_scale[1];
        cf_params.vel_down_max_scale[2] = vel_down_max_scale[2];
        ui.textBrowser->append("vel_down_max_scale changed");
    }

    if (QString::compare(item->text(0), "vel_hor_max_scale") == 0)
    {
        float vel_hor_max_scale[3];

        vel_hor_max_scale[0] = item->text(1).toFloat();
        vel_hor_max_scale[1] = item->text(2).toFloat();
        vel_hor_max_scale[2] = item->text(3).toFloat();

        cf_params.vel_hor_max_scale[0] = vel_hor_max_scale[0];
        cf_params.vel_hor_max_scale[1] = vel_hor_max_scale[1];
        cf_params.vel_hor_max_scale[2] = vel_hor_max_scale[2];
        ui.textBrowser->append("vel_hor_max_scale changed");
    }

}