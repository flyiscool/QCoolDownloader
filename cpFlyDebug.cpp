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
#include "cpFlyDebug.h"

#define VID_COOLFLY		0xAAAA

//#define PID_CP_F1_SKY   0x0001
#define PID_CP_F1_GND   0xAA97

#define IFAUDIO			2

#define ENDPOINT_AUDIO_IN	0x85


threadsafe_queue<DebugBuffPackage*> gListFlyDebug;

int gettimeofday(struct timeval* tp, void* tzp)
{

	static const DWORDLONG FILETIME_to_timval_skew = 116444736000000000;
	FILETIME   tfile;
	::GetSystemTimeAsFileTime(&tfile);

	ULARGE_INTEGER _100ns;
	_100ns.LowPart = tfile.dwLowDateTime;
	_100ns.HighPart = tfile.dwHighDateTime;

	_100ns.QuadPart -= FILETIME_to_timval_skew;

	// Convert 100ns units to seconds;
	ULARGE_INTEGER largeint;
	largeint.QuadPart = _100ns.QuadPart / (10000 * 1000);

	// Convert 100ns units to seconds;
	tp->tv_sec = (long)(_100ns.QuadPart / (10000 * 1000));
	// Convert remainder to microseconds;
	tp->tv_usec = (long)((_100ns.QuadPart % (10000 * 1000)) / 10);

	return (0);
}



void CPThreadFlyDebug::threadCPFlyDebug_main(CPThreadFlyDebug* pCPThreadFlyDebug)
{
	CPThreadFlyDebug* pCPthThis = pCPThreadFlyDebug;
	QString temp;
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
	DebugBuffPackage* debugrx;
	DebugBuffPackage* giveup;

	uint32_t cnt = 0;
	while (m_isCanRun){

		// exit 
		if (!(pCPthThis->IsRun())) {

			if (pCPthThis->devGround != NULL)
			{
				r = libusb_release_interface(pCPthThis->devGround, IFAUDIO); //release the claimed interface
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
			if (cnt_dev < 0){
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
				r = libusb_claim_interface(pCPthThis->devGround, IFAUDIO); //claim interface 0 (the first) of device (mine had jsut 1)
				if (r < 0) {
					emit signalupdateTextUi("Cannot Claim Interface");
					libusb_free_device_list(pCPthThis->devsList, 1);
					pCPthThis->devGround = NULL;
					continue;
				}
				emit pCPthThis->signalupdateStateLED(normal);
			}
		}

		transferred = 0;
		debugrx = new DebugBuffPackage;
		r = libusb_interrupt_transfer(pCPthThis->devGround, ENDPOINT_AUDIO_IN, debugrx->data, 4096, &transferred, 1000);

		switch (r)
		{
		case 0:
			debugrx->length = transferred;
			cnt++;
			debugrx->packageID = cnt;
			if (0 == gListFlyDebug.push(debugrx, giveup, 1000))
			{
				emit signalupdateTextUi("RXFLYDEBUG LIST is Full,Give Up the oldest package");
				delete giveup;
			}
			break;

		case LIBUSB_ERROR_TIMEOUT:	// timeout also need to check the transferred;
			if (transferred == 0)
			{
				delete debugrx;
			}
			else
			{			
				debugrx->length = transferred;
				cnt++;
				debugrx->packageID = cnt;
				temp.sprintf("time out RXFLYDEBUG rx = %d", transferred);
				emit signalupdateTextUi(temp);
				if (0 == gListFlyDebug.push(debugrx, giveup, 1000))
				{
					emit signalupdateTextUi("RXFLYDEBUG LIST is Full,Give Up the oldest package");
					delete giveup;
				}
			}
			break;
		case LIBUSB_ERROR_PIPE:	// the endpoint halted,so  retry to open the interface.
			emit signalupdateTextUi("RXFLYDEBUG endpoint halted. LIBUSB_ERROR_PIPE");
			pCPthThis->devGround = NULL;
			pCPthThis->msleep(500);
			break;
		case LIBUSB_ERROR_OVERFLOW:	 // give up the data because it's maybe lost. need fix.
			emit signalupdateTextUi("RXFLYDEBUG Buff is to small. LIBUSB_ERROR_OVERFLOW");
			break;

		case LIBUSB_ERROR_NO_DEVICE: // maybe lost the device.
			emit signalupdateTextUi("RXFLYDEBUG device lost. LIBUSB_ERROR_NO_DEVICE");
			break;
		case LIBUSB_ERROR_BUSY:
			emit signalupdateTextUi("RXFLYDEBUG LIBUSB_ERROR_BUSY");
			pCPthThis->msleep(500);
			break;
		default:
			emit signalupdateTextUi("RXFLYDEBUG error unknow :");
			break;
		}

	}


	if (!(pCPthThis->IsRun())) {

		if (pCPthThis->devGround != NULL)
		{
			r = libusb_release_interface(pCPthThis->devGround, IFAUDIO); //release the claimed interface
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
}

void CPThreadFlyDebug::run()
{
	m_isCanRun = true;
	threadCPFlyDebug_main(this);
}





void CPThreadFlyDebugParse::FlyDebugParse_main(void)
{
#define MSG_LEN	108
	DebugBuffPackage* debugrx;
	uint16_t msg_id;

	uint8_t buff[MSG_LEN];
	uint8_t state = 0;
	QString temp;
	int i;
	bool ret;


	QDateTime current_time = QDateTime::currentDateTime();
	//显示时间，格式为：年-月-日 时：分：秒 周几
	QString StrCurrentTime = current_time.toString("yyyy_MM_dd_hh_mm_ss");
	qDebug() << "StrCurrentTime = " << StrCurrentTime << endl;


	QFile file(StrCurrentTime+".txt");
	if (file.open(QIODevice::ReadWrite | QIODevice::Text)) //QIODevice::ReadWrite支持读写
	{
		QTextStream stream(&file);
		stream << "ax ay az gx gy gz mx my mz baro yaw pitch roll rc_r rc_p rc_th rc_yaw rc_mod mfl mfr mbl mbr po_r po_p po_th po_ya" << endl; //"123" 为写入文本的字符-- endl表示换行-- 理解就ok；
	


	while (m_isCanRun)
	{
		
		switch (state)
		{
			case 0:
				memset(buff, 0, MSG_LEN);

				do {
					ret = get_n_byte(1, buff);
					//qDebug() << "buff0"<< buff[0] << endl;
					if (ret == false)
					{
						break;
					}
				} while (buff[0] != 0xFF);

				state = 1;
				break;

			case 1:
				
				ret = get_n_byte(1, &buff[1]);
				//qDebug() << "buff1" << buff[1] << endl;
				if (ret == false)
				{
					break;
				}
				if (buff[1] == 0x5A)
				{
					state = 2;
				}
				else if(buff[1] == 0xFF)
				{
					state = 1;
				}
				else
				{
					state = 0;
				}
				break;

			case 2:
				ret = get_n_byte(MSG_LEN - 2, &buff[2]);
				if (ret == false)
				{
					break;
				}
				if ((buff[MSG_LEN - 1] == 0xFF) && (buff[MSG_LEN - 2] == 0x5A))
				{
					state = 3;
				}
				else
				{
					state = 0;
				}
				break;
			case 3:
				uint32_t pkdid = (buff[7] << 24) | (buff[6] << 16) | (buff[5] << 8) | buff[4];
				//qDebug() << "get a package!!!" << endl;
				flight_data_t flight_pkt;
				memcpy(&flight_pkt, buff, MSG_LEN);

				if (pkdid % 1000 == 0)
				{
					QString temp2;
					temp2.sprintf("pkdid = %d", pkdid);
					emit signalupdateTextUi(temp2);

					qDebug() << flight_pkt.cnt;
					qDebug() << flight_pkt.imu9.acc_x << flight_pkt.imu9.acc_y << flight_pkt.imu9.acc_z ;
					qDebug() << flight_pkt.imu9.gyro_x << flight_pkt.imu9.gyro_y << flight_pkt.imu9.gyro_z;
					qDebug() << flight_pkt.imu9.mag_x << flight_pkt.imu9.mag_y << flight_pkt.imu9.mag_z;
					qDebug() << flight_pkt.attitude3.yaw << flight_pkt.attitude3.pitch << flight_pkt.attitude3.roll;
					qDebug() << flight_pkt.rc_ch.roll << flight_pkt.rc_ch.pitch << flight_pkt.rc_ch.throttle << flight_pkt.rc_ch.yaw << flight_pkt.rc_ch.mode;
					qDebug() << flight_pkt.baro.height_comp;
					qDebug() << flight_pkt.motor.motor_back_right << flight_pkt.motor.motor_front_right << flight_pkt.motor.motor_back_left << flight_pkt.motor.motor_front_left;
					qDebug() << flight_pkt.pid_out.roll << flight_pkt.pid_out.pitch << flight_pkt.pid_out.throttle << flight_pkt.pid_out.yaw;


					
				}
				
				stream << flight_pkt.cnt << ",";
				stream << flight_pkt.imu9.acc_x << "," << flight_pkt.imu9.acc_y << "," << flight_pkt.imu9.acc_z << ",";
				stream << flight_pkt.imu9.gyro_x << "," << flight_pkt.imu9.gyro_y << "," << flight_pkt.imu9.gyro_z << ",";
				stream << flight_pkt.imu9.mag_x << "," << flight_pkt.imu9.mag_y << "," << flight_pkt.imu9.mag_z << ",";
				stream << flight_pkt.attitude3.yaw << "," << flight_pkt.attitude3.pitch << "," << flight_pkt.attitude3.roll << ",";
				stream << flight_pkt.rc_ch.roll << "," << flight_pkt.rc_ch.pitch << "," << flight_pkt.rc_ch.throttle << "," << flight_pkt.rc_ch.yaw << "," << flight_pkt.rc_ch.mode << ",";
				stream << flight_pkt.baro.height_comp << ",";
				stream << flight_pkt.motor.motor_back_right << "," << flight_pkt.motor.motor_front_right << "," << flight_pkt.motor.motor_back_left << "," << flight_pkt.motor.motor_front_left;
				stream << flight_pkt.pid_out.roll << "," << flight_pkt.pid_out.pitch << "," << flight_pkt.pid_out.throttle << "," << flight_pkt.pid_out.yaw << endl;

				state = 0;
				break;
		}


		
		//QString temp2((char*)debugrx->data);
		//emit signalupdateTextUi(temp2);
		
		

		//printf("head1 = %02x %02x", debugrx->data[0], debugrx->data[1]);

	}
	}
	file.close();

}



void CPThreadFlyDebugParse::run()
{
	m_isCanRun = true;
	FlyDebugParse_main();
}


bool CPThreadFlyDebugParse::get_n_byte(uint32_t length, uint8_t* Framed_databuf)
{
	unsigned int sizeNum = length;
	unsigned int NumGet = 0;

	uint8_t buf_tmp[4096];
	DebugBuffPackage* pBuff;

	do {

		while (gListFlyDebug.empty() )
		{
			Sleep(5);

			if (m_isCanRun == false)
			{
				return false;
			}
		}

		gListFlyDebug.try_front(pBuff);

		if (sizeNum < pBuff->length)
		{

			memcpy(Framed_databuf + NumGet, pBuff->data, sizeNum);
			pBuff->length -= sizeNum;
			NumGet += sizeNum;

			memcpy(buf_tmp, &pBuff->data[sizeNum], pBuff->length);
			sizeNum -= sizeNum;
			memcpy(pBuff->data, buf_tmp, pBuff->length);
		}
		else 
		{
			memcpy(Framed_databuf + NumGet, pBuff->data, pBuff->length);
			NumGet += pBuff->length;
			sizeNum -= pBuff->length;

			gListFlyDebug.try_pop(pBuff);
			delete pBuff;
		}
		
	} while (sizeNum > 0);

	return true;
}

