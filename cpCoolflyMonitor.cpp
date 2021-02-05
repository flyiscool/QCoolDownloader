#include "stdafx.h"
#include <QtWidgets/QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDataStream>
#include <QDateTime>
#include <QThread>
#include <queue>
#include <time.h>

#include "cpThread.h"
#include "libusb.h"
#include "cpThreadSafeQueue.h"
#include "cpCoolflyMonitor.h"
#include "cpUpdataApp.h"

#define VID_COOLFLY		0xAAAA

//#define PID_CP_F1_SKY   0x0001
#define PID_CP_F1_GND   0xAA97

#define IFCMD			0

#define ENDPOINT_VEDIO_OUT	0x06 
#define ENDPOINT_VEDIO_IN	0x86 

#define ENDPOINT_CMD_OUT	0x01 
#define ENDPOINT_CMD_IN		0x84 

#define MAX_BUFFER_IN_VEDIO1_LIST	200

threadsafe_queue<CMDBuffPackage*> gListTXCMD;
threadsafe_queue<CMDBuffPackage*> gListRXCMD;
QString temp;



extern bool update_app_flag = false;

void CPThreadCoolflyMonitor::threadCPCoolflyMonitor_main(CPThreadCoolflyMonitor* pCPThreadCoolflyMonitor)
{
	CPThreadCoolflyMonitor* pCPthThis = pCPThreadCoolflyMonitor;

	pCPthThis->ctx = NULL;
	pCPthThis->devGround = NULL;
	int r = libusb_init(&pCPthThis->ctx); //initialize a library session
	if (r)
	{
		temp.sprintf("libusb init libusb Error = ", r);
		emit signalupdateTextUi(temp);
		return;
	}
	
	libusb_set_debug(pCPthThis->ctx, 3);  //set verbosity level to 3, as suggested in the documentation

	int transferred;
	struct timeval cap_systime;
	CMDBuffPackage* cmdtx;
	CMDBuffPackage* cmdrx;
	CMDBuffPackage* giveup;
	while (m_isCanRun) {

		// exit 
		if (!(pCPthThis->IsRun())) {

			if (pCPthThis->devGround != NULL)
			{
				r = libusb_release_interface(pCPthThis->devGround, IFCMD); //release the claimed interface
				if (r != 0) {
					emit signalupdateTextUi("Cannot Release Interface");
				}
			}

			if (pCPthThis->devGround != NULL)
			{
				libusb_close(pCPthThis->devGround);
			}

			if (pCPthThis->ctx != NULL)
			{
				//libusb_exit(pCPthThis->ctx); //close the sessio
			}
			pCPthThis->ctx = NULL;
			return;
		}

		// fresh the dev Ground
		if (pCPthThis->devGround == NULL)
		{

			int cnt_dev = libusb_get_device_list(NULL, &pCPthThis->devsList); //get the list of devices
			if (cnt_dev < 0) {
				emit signalupdateTextUi("Error in enumerating devices !");
				continue;
			}

			pCPthThis->devGround = libusb_open_device_with_vid_pid(pCPthThis->ctx, VID_COOLFLY, PID_CP_F1_GND);
			if (pCPthThis->devGround == NULL) {
				emit signalupdateTextUi("Can't find the device Ground");
				emit pCPthThis->signalupdateStateLED(error);

				libusb_free_device_list(pCPthThis->devsList, 1);
				pCPthThis->msleep(1000);
				continue;
			}
			else
			{
				r = libusb_claim_interface(pCPthThis->devGround, IFCMD); //claim interface 0 (the first) of device (mine had jsut 1)
				if (r < 0) {
					emit signalupdateTextUi("Cannot Claim Interface");
					libusb_free_device_list(pCPthThis->devsList, 1);
					pCPthThis->devGround = NULL;
					continue;
				}
				emit pCPthThis->signalupdateStateLED(normal);
				emit signalupdateTextUi("GET USB DEVICE SUCCESS");
			}
		}

		if (pCPthThis->devGround != NULL)		// process txcmd
		{
			if (gListTXCMD.size() > 0)
			{
				gListTXCMD.try_front(cmdtx);

				r = libusb_interrupt_transfer(pCPthThis->devGround, ENDPOINT_CMD_OUT, cmdtx->data, cmdtx->length, &transferred, 1000);
				switch (r)
				{
				case 0:
					gListTXCMD.try_pop(cmdtx);
					delete cmdtx;
					emit signalupdateTextUi("TXCMD SEND SUCCESS");
					break;

				case LIBUSB_ERROR_TIMEOUT:	// timeout also need to check the transferred;
					emit signalupdateTextUi("TXCMD SEND FAILED");
					break;
				case LIBUSB_ERROR_PIPE:	// the endpoint halted,so  retry to open the interface.
					emit signalupdateTextUi("TXCMD endpoint halted. LIBUSB_ERROR_PIPE");
					pCPthThis->devGround = NULL;
					libusb_free_device_list(pCPthThis->devsList, 1);
					pCPthThis->msleep(500);
					break;
				case LIBUSB_ERROR_OVERFLOW:	 // give up the data because it's maybe lost. need fix.
					emit signalupdateTextUi("TXCMD Buff is to small. LIBUSB_ERROR_OVERFLOW");
					break;

				case LIBUSB_ERROR_NO_DEVICE: // maybe lost the device.
					emit signalupdateTextUi("TXCMD device lost. LIBUSB_ERROR_NO_DEVICE");
					break;
				case LIBUSB_ERROR_BUSY:
					emit signalupdateTextUi("TXCMD LIBUSB_ERROR_BUSY");
					pCPthThis->msleep(500);
					break;
				default:
					emit signalupdateTextUi("TXCMD error unknow :");
					break;
				}
			}
		}




		if (pCPthThis->devGround != NULL)		//	process rxcmd
		{ 
			transferred = 0;
			cmdrx = new CMDBuffPackage;
			r = libusb_interrupt_transfer(pCPthThis->devGround, ENDPOINT_CMD_IN, cmdrx->data, 512, &transferred, 200);
			switch (r)
			{
			case 0:
				cmdrx->length = transferred;
				temp.sprintf("RXCMD rx = %d", transferred);
				emit signalupdateTextUi(temp);
				if (0 == gListRXCMD.push(cmdrx, giveup, 1000))
				{
					emit signalupdateTextUi("RXCMD LIST is Full,Give Up the oldest package");
					delete giveup;
				}
				break;

			case LIBUSB_ERROR_TIMEOUT:	// timeout also need to check the transferred;
				if (transferred == 0)
				{
					delete cmdrx;
				}
				else
				{			
					cmdrx->length = transferred;
					temp.sprintf("time out RXCMD rx = %d", transferred);
					emit signalupdateTextUi(temp);
					if (0 == gListRXCMD.push(cmdrx, giveup, 1000))
					{
						emit signalupdateTextUi("RXCMD LIST is Full,Give Up the oldest package");
						delete giveup;
					}
				}
				break;
			case LIBUSB_ERROR_PIPE:	// the endpoint halted,so  retry to open the interface.
				emit signalupdateTextUi("RXCMD endpoint halted. LIBUSB_ERROR_PIPE");
				pCPthThis->devGround = NULL;
				libusb_free_device_list(pCPthThis->devsList, 1);
				pCPthThis->msleep(500);
				break;
			case LIBUSB_ERROR_OVERFLOW:	 // give up the data because it's maybe lost. need fix.
				emit signalupdateTextUi("RXCMD Buff is to small. LIBUSB_ERROR_OVERFLOW");
				break;

			case LIBUSB_ERROR_NO_DEVICE: // maybe lost the device.
				emit signalupdateTextUi("RXCMD device lost. LIBUSB_ERROR_NO_DEVICE");
				break;
			case LIBUSB_ERROR_BUSY:
				emit signalupdateTextUi("RXCMD LIBUSB_ERROR_BUSY");
				pCPthThis->msleep(500);
				break;
			default:
				qDebug() << r << endl;
				emit signalupdateTextUi("RXCMD error unknow :");
				break;
			}
		}


		if (pCPthThis->devGround != NULL)		// process update_app
		{
			if (update_app_flag == true)
			{
				r = Cmd_Upgrade_V2(pCPthThis->devGround, &file);
				switch (r)
				{
				case 0:
					break;

				case LIBUSB_ERROR_TIMEOUT:	// timeout also need to check the transferred;
					break;
				case LIBUSB_ERROR_PIPE:	// the endpoint halted,so  retry to open the interface.
					emit signalupdateTextUi("UPDATE_APP endpoint halted. LIBUSB_ERROR_PIPE");
					pCPthThis->devGround = NULL;
					libusb_free_device_list(pCPthThis->devsList, 1);
					pCPthThis->msleep(500);
					break;
				case LIBUSB_ERROR_OVERFLOW:	 // give up the data because it's maybe lost. need fix.
					emit signalupdateTextUi("UPDATE_APP is to small. LIBUSB_ERROR_OVERFLOW");
					break;

				case LIBUSB_ERROR_NO_DEVICE: // maybe lost the device.
					emit signalupdateTextUi("UPDATE_APP device lost. LIBUSB_ERROR_NO_DEVICE");
					break;
				case LIBUSB_ERROR_BUSY:
					emit signalupdateTextUi("UPDATE_APP LIBUSB_ERROR_BUSY");
					pCPthThis->msleep(500);
					break;
				default:
					emit signalupdateTextUi("UPDATE_APP error unknow :");
					break;
				}
			}
		}


	}


	if (pCPthThis->devGround != NULL)
	{
		r = libusb_release_interface(pCPthThis->devGround, IFCMD); //release the claimed interface
		if (r != 0) {
			emit signalupdateTextUi("Cannot Release Interface");
		}
	}

	if (pCPthThis->devGround != NULL)
	{
		libusb_close(pCPthThis->devGround);
	}

	if (pCPthThis->ctx != NULL)
	{
		//libusb_exit(pCPthThis->ctx); //close the sessio
	}
	pCPthThis->ctx = NULL;
	
}

void CPThreadCoolflyMonitor::run()
{
	m_isCanRun = true;
	threadCPCoolflyMonitor_main(this);
}
