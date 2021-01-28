#pragma once

#include "cpThread.h"



// The thread monitor the usb device hot-plug
class CPThreadFlyDebug : public CPThread
{
	Q_OBJECT
public:
	virtual void run();
	libusb_device** devsList;
	libusb_context* ctx;
	libusb_device_handle* devGround;
	libusb_device_handle* devSky;
	void threadCPFlyDebug_main(CPThreadFlyDebug* pCPThreadFlyDebug);
signals:
	void signalupdateStateLED(State_LED);
	void signalupdateTextUi(QString);
	
};



class CPThreadFlyDebugParse : public CPThread
{
	Q_OBJECT
public:
	virtual void run();
	void	FlyDebugParse_main(void);
	bool get_n_byte(uint32_t length, uint8_t* Framed_databuf);

signals:
	void signalFrameRateUpdate(int);
	void signalupdateTextUi(QString);
	
};
