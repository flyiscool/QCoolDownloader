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
signals:
	void signalFrameRateUpdate(int);
	void signalupdateTextUi(QString);
};


#endif