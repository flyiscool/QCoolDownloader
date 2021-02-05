#pragma once

#include "cpThread.h"
#include "cpUpdataApp.h"


// The thread monitor the usb device hot-plug
class CPThreadCoolflyMonitor : public CPThread
{
	Q_OBJECT
public:
	virtual void run();
	libusb_device** devsList;
	libusb_context* ctx;
	libusb_device_handle* devGround;
	libusb_device_handle* devSky;
	void threadCPCoolflyMonitor_main(CPThreadCoolflyMonitor* pCPThreadCoolflyMonitor);
	QString file;
	bool update_app_flag = false;
	int Cmd_Upgrade_V2(libusb_device_handle* dev, QString* upgrade_file);

signals:
	void signalupdateStateLED(State_LED);
	void signalupdateTextUi(QString);
	void signalreboot(void);
	
};
