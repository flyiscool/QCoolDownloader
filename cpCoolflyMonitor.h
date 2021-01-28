#pragma once

#include "cpThread.h"



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
signals:
	void signalupdateStateLED(State_LED);
	void signalupdateTextUi(QString);
	
};
