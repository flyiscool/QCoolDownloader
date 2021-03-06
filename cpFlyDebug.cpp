﻿#include "stdafx.h"
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
			libusb_free_device_list(pCPthThis->devsList, 1);
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


bool isDirExist(QString fullPath)
{
	QDir dir(fullPath);
	if (dir.exists())
	{
		return true;
	}
	else
	{
		bool ok = dir.mkdir(fullPath);//只创建一级子目录，即必须保证上级目录存在
		return ok;
	}
}


void CPThreadFlyDebugParse::FlyDebugParse_main(void)
{

	DebugBuffPackage* debugrx; 
	uint16_t msg_id;
	flight_data_t g_debug;
	
	qDebug("---------------------------------");
	qDebug("g_debug = %d  addr = %08x", sizeof(flight_data_t), &g_debug);
	qDebug("magichead = %d  addr = %08x", sizeof(uint32_t), &g_debug.magichead);
	qDebug("baro = %d  addr = %08x", sizeof(baro_t), &g_debug.baro);
	qDebug("accel = %d  addr = %08x", sizeof(accel_t), &g_debug.accel);
	qDebug("gyro = %d  addr = %08x", sizeof(gyro_t), &g_debug.gyro);
	qDebug("mag = %d  addr = %08x", sizeof(mag_t), &g_debug.mag);
	qDebug("gps = %d  addr = %08x", sizeof(gps_t), &g_debug.gps);
	qDebug("manual0 = %d  addr = %08x", sizeof(manual_t), &g_debug.manual0);
	qDebug("end = %d  addr = %08x", sizeof(uint32_t), &g_debug.end);

	uint8_t buff[sizeof(flight_data_t)];
	uint8_t state = 0;
	QString temp;
	int i;
	bool ret;


	QDateTime current_time = QDateTime::currentDateTime();
	//显示时间，格式为：年-月-日 时：分：秒 周几
	QString StrCurrentTime = current_time.toString("yyyy_MM_dd_hh_mm_ss");
	qDebug() << "StrCurrentTime = " << StrCurrentTime << endl;
	qDebug() << "flight_data_t size = " << sizeof(flight_data_t) << endl;

	ret = isDirExist(QDir::currentPath() + "/"+ StrCurrentTime);

	qDebug() << "ret size = " << ret << QDir::currentPath() + StrCurrentTime <<  endl;

	QFile file_baro("./"+ StrCurrentTime + "/baro.txt");
	QFile file_accel("./" + StrCurrentTime + "/accel.txt");
	QFile file_gyro("./" + StrCurrentTime + "/gyro.txt");
	QFile file_mag("./" + StrCurrentTime + "/mag.txt");
	QFile file_gps("./" + StrCurrentTime + "/gps.txt");
	QFile file_manual("./" + StrCurrentTime + "/manual.txt");


	bool ret_file = true;

	ret_file = file_baro.open(QIODevice::ReadWrite | QIODevice::Text);
	ret_file &= file_accel.open(QIODevice::ReadWrite | QIODevice::Text);
	ret_file &= file_gyro.open(QIODevice::ReadWrite | QIODevice::Text);
	ret_file &= file_mag.open(QIODevice::ReadWrite | QIODevice::Text);
	ret_file &= file_gps.open(QIODevice::ReadWrite | QIODevice::Text);
	ret_file &= file_manual.open(QIODevice::ReadWrite | QIODevice::Text);


	if (ret_file) //QIODevice::ReadWrite支持读写
	{
		QTextStream stream_baro(&file_baro);
		stream_baro << "timestamp, temperature, pressure, altitude" << endl;

		QTextStream stream_accel(&file_accel);
		stream_accel << "timestamp, temperature, x, y, z,  x_raw, y_raw, z_raw" << endl;

		QTextStream stream_gyro(&file_gyro);
		stream_gyro << "timestamp, temperature, x, y, z,  x_raw, y_raw, z_raw" << endl;

		QTextStream stream_mag(&file_mag);
		stream_mag << "timestamp, temperature, x, y, z,  x_raw, y_raw, z_raw" << endl;

		QTextStream stream_gps(&file_gps);
		stream_gps << "timestamp, timestamp_time_relative, time_utc_usec, lat, lon, alt, alt_ellipsoid, s_variance_m_s,";
		stream_gps << "c_variance_rad, fix_type, eph, epv, hdop, vdop, noise_per_ms, jamming_indicator,";
		stream_gps << "vel_m_s, vel_n_m_s, vel_e_m_s, vel_d_m_s, cog_rad, vel_ned_valid, satellites_used " << endl;

		QTextStream stream_manual(&file_manual);
		stream_manual << "timestamp, x, y, z, r, scroll[0], scroll[1], offset[0], offset[1]";
		stream_manual << "button, gear, rssi, regained" << endl;

	while (m_isCanRun)
	{
		
		switch (state)
		{
			case 0:
				memset(buff, 0, sizeof(flight_data_t));

				do {
					ret = get_n_byte(1, buff);
					qDebug() << "buff0"<< buff[0] << endl;
					if (ret == false)
					{
						break;
					}
				} while (buff[0] != 0xFF);

				state = 1;
				break;

			case 1:
				
				ret = get_n_byte(1, &buff[1]);
				qDebug() << "buff1" << buff[1] << endl;
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
				ret = get_n_byte(sizeof(flight_data_t) - 2, &buff[2]);
				qDebug() <<"ret = " <<ret << endl;
				if (ret == false)
				{
					break;
				}
				//for (int k = 0; k < sizeof(flight_data_t); k++)
				//{
				//	qDebug() << " bi = "<< k <<" ==" << buff[k] << endl;
				//}
				//qDebug() << "sizeof(flight_data_t) = " << sizeof(flight_data_t) << endl;
				//qDebug() << "buff[sizeof(flight_data_t) - 1] = " << buff[sizeof(flight_data_t) - 1] << endl;
				//qDebug() << "buff[sizeof(flight_data_t) - 2] = " << buff[sizeof(flight_data_t) - 2] << endl;
				if ((buff[sizeof(flight_data_t) - 5 ] == 0xFF) && (buff[sizeof(flight_data_t) - 6] == 0x5A))
				{
					state = 3;
				}
				else
				{
					state = 0;
				}
				break;

			case 3:
	
				qDebug() << "get a package!!!" << endl;
				static uint32_t pkdid = 0;
				pkdid++;
				flight_data_t flight_pkt;
				memcpy(&flight_pkt, buff, sizeof(flight_data_t));

				if ((pkdid % 1000 == 0) || pkdid == 1)
				{
					QString temp2;
					temp2.sprintf("pkdid = %d", pkdid);
					emit signalupdateTextUi(temp2);
					
				}
				
				stream_baro << flight_pkt.baro.timestamp << ",";
				stream_baro << flight_pkt.baro.temperature << ",";	// temperature in degrees celsius
				stream_baro << flight_pkt.baro.pressure << ",";	// barometric pressure, already temp. comp.
				stream_baro << flight_pkt.baro.altitude << endl;	 // altitude, already temp. comp.
                       	

				stream_accel << flight_pkt.accel.timestamp << ",";
				stream_accel << flight_pkt.accel.temperature << ",";	
				stream_accel << flight_pkt.accel.x << ",";	
				stream_accel << flight_pkt.accel.y << ",";
				stream_accel << flight_pkt.accel.z << ",";
				stream_accel << flight_pkt.accel.x_raw << ",";
				stream_accel << flight_pkt.accel.y_raw << ",";
				stream_accel << flight_pkt.accel.z_raw << endl;

				stream_gyro << flight_pkt.gyro.timestamp << ",";
				stream_gyro << flight_pkt.gyro.temperature << ",";
				stream_gyro << flight_pkt.gyro.x << ",";
				stream_gyro << flight_pkt.gyro.y << ",";
				stream_gyro << flight_pkt.gyro.z << ",";
				stream_gyro << flight_pkt.gyro.x_raw << ",";
				stream_gyro << flight_pkt.gyro.y_raw << ",";
				stream_gyro << flight_pkt.gyro.z_raw << endl;

				stream_mag << flight_pkt.mag.timestamp << ",";
				stream_mag << flight_pkt.mag.temperature << ",";
				stream_mag << flight_pkt.mag.x << ",";
				stream_mag << flight_pkt.mag.y << ",";
				stream_mag << flight_pkt.mag.z << ",";
				stream_mag << flight_pkt.mag.x_raw << ",";
				stream_mag << flight_pkt.mag.y_raw << ",";
				stream_mag << flight_pkt.mag.z_raw << endl;

				stream_gps << flight_pkt.gps.timestamp << ",";
				stream_gps << flight_pkt.gps.timestamp_time_relative << ",";
				stream_gps << flight_pkt.gps.time_utc_usec << ",";
				stream_gps << flight_pkt.gps.lat << ",";
				stream_gps << flight_pkt.gps.lon << ",";
				stream_gps << flight_pkt.gps.alt << ",";
				stream_gps << flight_pkt.gps.alt_ellipsoid << ",";
				stream_gps << flight_pkt.gps.s_variance_m_s << ",";
				stream_gps << flight_pkt.gps.c_variance_rad << ",";
				stream_gps << flight_pkt.gps.fix_type << ",";
				stream_gps << flight_pkt.gps.eph << ",";
				stream_gps << flight_pkt.gps.epv << ",";
				stream_gps << flight_pkt.gps.hdop << ",";
				stream_gps << flight_pkt.gps.vdop << ",";
				stream_gps << flight_pkt.gps.noise_per_ms << ",";
				stream_gps << flight_pkt.gps.jamming_indicator << ",";
				stream_gps << flight_pkt.gps.vel_m_s << ",";
				stream_gps << flight_pkt.gps.vel_n_m_s << ",";
				stream_gps << flight_pkt.gps.vel_e_m_s << ",";
				stream_gps << flight_pkt.gps.vel_d_m_s << ",";
				stream_gps << flight_pkt.gps.cog_rad << ","; 
				stream_gps << flight_pkt.gps.vel_ned_valid << ",";
				stream_gps << flight_pkt.gps.satellites_used << endl;

				stream_manual << flight_pkt.manual0.timestamp << ",";
				stream_manual << flight_pkt.manual0.x << ",";
				stream_manual << flight_pkt.manual0.y << ",";
				stream_manual << flight_pkt.manual0.z << ",";
				stream_manual << flight_pkt.manual0.r << ",";
				stream_manual << flight_pkt.manual0.scroll[0] << ",";
				stream_manual << flight_pkt.manual0.scroll[1] << ",";
				stream_manual << flight_pkt.manual0.offset[0] << ",";
				stream_manual << flight_pkt.manual0.offset[1] << ",";
				stream_manual << flight_pkt.manual0.button << ",";
				stream_manual << flight_pkt.manual0.gear << ",";
				stream_manual << flight_pkt.manual0.rssi << ",";
				stream_manual << flight_pkt.manual0.regained << endl;

				state = 0;
				break;
		}


		
		//QString temp2((char*)debugrx->data);
		//emit signalupdateTextUi(temp2);
		
		

		//printf("head1 = %02x %02x", debugrx->data[0], debugrx->data[1]);

	}
	}
	file_baro.close();

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

