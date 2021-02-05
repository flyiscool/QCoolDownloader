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

//#define RELEASE_VERISON        1

QString int8_t2Str(uint8_t* data, int len, int filedwidth, int base);



void QCoolDownlaod::slotselfile(void)
{
	fileNames = QFileDialog::getOpenFileName(this, tr("open file"), " ", tr("bin(*.bin)"));

    ui.label_update->setText(fileNames);
	ui.textBrowser->setText(fileNames);

    ui.statusBar->showMessage(fileNames, 5000);
}

void QCoolDownlaod::slotdownloadfileflag(void)
{
    //thCoolflyMonitor.stopImmediately();
    //thCoolflyMonitor.wait();

    //thThreadCMDParse.stopImmediately();
    //thThreadCMDParse.wait();


    //thUsbMonitor.setfilename(fileNames);
    //thUsbMonitor.start();

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


    connect(ui.sel_connect, SIGNAL(clicked()), this, SLOT(slotconnectusb()));

    connect(ui.sel_bt, SIGNAL(clicked()), this, SLOT(slotselfile()));

    connect(ui.download_pt, SIGNAL(clicked()), this, SLOT(slotdownloadfileflag()));

    connect(&thCoolflyMonitor, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadCMDParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebug, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));
    connect(&thThreadFlyDebugParse, SIGNAL(signalupdateTextUi(QString)), this, SLOT(slotupdateTextUi(QString)));

    qRegisterMetaType<State_LED>("State_LED");
    connect(&thCoolflyMonitor, SIGNAL(signalupdateStateLED(State_LED)), this, SLOT(slotsetbt_connect_color(State_LED)));

    connect(&thCoolflyMonitor, SIGNAL(signalreboot()), this, SLOT(slotcmd_reboot()));

    connect(ui.sel_get_verison, SIGNAL(clicked()), this, SLOT(slotcmd_get_version()));
    connect(ui.bt_get_setting, SIGNAL(clicked()), this, SLOT(slotcmd_get_setting()));
    connect(ui.bt_save_setting, SIGNAL(clicked()), this, SLOT(slotcmd_save_setting()));
    connect(ui.bt_cf_protocol, SIGNAL(clicked()), this, SLOT(slotcmd_start_cf_protocol()));
    connect(ui.bt_usb_update, SIGNAL(clicked()), this, SLOT(slotcmd_usb_update()));

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
    connect(ui.bt_imu_cali, SIGNAL(clicked()), this, SLOT(slotcmd_imu_cali()));
    connect(ui.bt_mag_cali, SIGNAL(clicked()), this, SLOT(slotcmd_mag_cali()));

    connect(&thThreadCMDParse, SIGNAL(signalupdateFactorySetting()), this, SLOT(slotupdate_factroy_setting()));
    connect(&thThreadCMDParse, SIGNAL(signalupdateChipID()), this, SLOT(slotupdateChipID()));

    ui.comboBox_year->setCurrentIndex(3);

    thCoolflyMonitor.update_app_flag = false;
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

        if ((uart3_baudrate > 4) || (uart3_baudrate<0))
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
                    QString::fromLocal8Bit("0 :    9600 \n1 :   19200 \n2 :   38400 \n3 :   57600 \n4 :  115200"));

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


