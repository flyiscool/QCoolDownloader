#pragma once
#ifndef CPCMDPARSE_H
#define CPCMDPARSE_H


#include <QtWidgets/QMainWindow>
#include <QThread>
#include <QMetaType>
#include <QMutex>
#include <qDebug>

#include "cpStructData.h"
#include "libusb.h"
#include "cpThread.h"

class CPThreadCMDParse : public CPThread
{
	Q_OBJECT
public:
	virtual void run();
	void	CMDParse_main(void);
	void	parse_cmd(char* buff, uint32_t length);
	void	parse_cmd_msgid_FlyStateData_V2(char* buff, uint8_t length);
	void	parse_cmd_msgid_flystate(char* buff, uint8_t length);

signals:
	void signalFrameRateUpdate(int);
	void signalupdateTextUi(QString);
	void signalupdateFactorySetting(void);
	void signalupdateChipID(void);
	void signalupdateFlyStateData(void);
	void signalupdateFlyState(void);
};


#endif