
#include "stdafx.h"
#include "cpThread.h"

#include "cpCMDParse.h"
#include "cpThreadSafeQueue.h"

extern threadsafe_queue<CMDBuffPackage*> gListTXCMD;
extern threadsafe_queue<CMDBuffPackage*> gListRXCMD;

STRU_VER_INFO cf_ver_info;
STRU_FACTORY_SETTING cf_factroy_setting;
STRU_DEVICE_INFO cf_device_info;
STRU_RC_STATUS cf_rc_status;
bool cf_checksum(CMDBuffPackage* cmdrx);


void CPThreadCMDParse::run()
{
	m_isCanRun = true;
	CMDParse_main();
}

void CPThreadCMDParse::CMDParse_main()
{
	CMDBuffPackage* cmdrx;
	uint16_t msg_id;
	uint16_t msg_len;

	STRU_WIRELESS_MSG_HEADER msg_rx_head;

	QString temp;
	int i;
	static char buff[4096];

	qDebug() << "CMDParse_main----" << endl;

	while (m_isCanRun)
	{
		if (gListRXCMD.empty())
		{
			Sleep(5);
			continue;
		}

		gListRXCMD.try_pop(cmdrx);

		if (cmdrx->length < 10)
		{
			delete cmdrx;
			continue;
		}

		memcpy(&msg_rx_head, cmdrx->data, 10);


		//1 check head 
		if (msg_rx_head.magic_header != 0x5AFF)
		{
			delete cmdrx;
			continue;
		}

		//2 check sum

		if (false == cf_checksum(cmdrx))
		{
			delete cmdrx;
			continue;
		}

		switch (msg_rx_head.msg_id)
		{
			case 0x0000: // version_info
				

				memset(buff,0,4096);
				for (i = 0; i < 6; i++)
				{
					buff[0 + i] = cmdrx->data[10 + i];
				}
				buff[6] = '\0';
				cf_ver_info.hardware_ver = "";
				cf_ver_info.hardware_ver.append(buff);

				memset(buff, 0, 4096);
				for (i = 0; i < 9; i++)
				{
					buff[0 + i] = cmdrx->data[16 + i];
				}
				buff[9] = '\0';
				cf_ver_info.sdk_ver = "";
				cf_ver_info.sdk_ver.append(buff);

				memset(buff, 0, 4096);
				for (i = 0; i < 22; i++)
				{
					buff[0 + i] = cmdrx->data[25 + i];
				}
				buff[22] = '\0';
				cf_ver_info.sdk_build_time = "";;
				cf_ver_info.sdk_build_time.append(buff);

				memset(buff, 0, 4096);
				for (i = 0; i < 9; i++)
				{
					buff[0 + i] = cmdrx->data[45 + i];
				}
				buff[9] = '\0';
				cf_ver_info.boot_ver = "";
				cf_ver_info.boot_ver.append(buff);

				memset(buff, 0, 4096);
				for (i = 0; i < 21; i++)
				{
					buff[0 + i] = cmdrx->data[54 + i];
				}
				buff[21] = '\0';
				cf_ver_info.boot_build_time = "";
				cf_ver_info.boot_build_time.append(buff);

				emit signalupdateTextUi("--Version Info--");
				emit signalupdateTextUi("hardware_ver :" + cf_ver_info.hardware_ver);
				emit signalupdateTextUi("sdk_ver :" + cf_ver_info.sdk_ver);
				emit signalupdateTextUi("sdk_build_time :" + cf_ver_info.sdk_build_time);
				emit signalupdateTextUi("boot_ver :" + cf_ver_info.boot_ver);
				emit signalupdateTextUi("boot_build_time :" + cf_ver_info.boot_build_time);

			break;

			case 0x005C: // FACTORY SETTING
				for (i = 0; i < msg_rx_head.msg_len; i++)
				{
					buff[msg_rx_head.packet_cur * 502 + i] = cmdrx->data[i+10];
				}

				if (msg_rx_head.packet_cur == (msg_rx_head.packet_num - 1))
				{
					if ((msg_rx_head.packet_cur * 502 + msg_rx_head.msg_len) == sizeof(STRU_FACTORY_SETTING))
					{
						memcpy(&cf_factroy_setting, buff, sizeof(STRU_FACTORY_SETTING));
						
						emit signalupdateFactorySetting();
						emit signalupdateTextUi("STRU_FACTORY_SETTING GET SUCCESS");
						
					}
				}
				break;

			case 0x005D: // SAVE FACTORY SETTING
				if (cmdrx->data[10] == 0x01)
				{
					emit signalupdateTextUi("SAVE_FACTORY_SETTING SUCCESS");
				}
				else
				{
					emit signalupdateTextUi("SAVE_FACTORY_SETTING FAILED");
				}
				break;

			case 0x000A: // reboot
				if (cmdrx->data[10] == 0x01)
				{
					emit signalupdateTextUi("REBOOT SUCCESS  goodbye!");
					emit signalupdateTextUi("----------------------------------------");
					
				}
				else
				{
					emit signalupdateTextUi("REBOOT FAILED");
				}
				break;
		
			case 0x0019: // device info
				for (i = 0; i < msg_rx_head.msg_len; i++)
				{
					buff[msg_rx_head.packet_cur * 502 + i] = cmdrx->data[i + 10];
				}

				if (msg_rx_head.packet_cur == (msg_rx_head.packet_num - 1))
				{
					if ((msg_rx_head.packet_cur * 502 + msg_rx_head.msg_len) == sizeof(STRU_DEVICE_INFO))
					{
						memcpy(&cf_device_info, buff, sizeof(STRU_DEVICE_INFO));
						
						emit signalupdateChipID();
						emit signalupdateTextUi("STRU_DEVICE_INFO GET SUCCESS");
					}
				}
				break;

			case 0x0093: // cf_protocol read info
				qDebug() << cmdrx->length << endl;
				parse_cmd((char *)&cmdrx->data[10], msg_rx_head.msg_len);
				break;				

			case 0x0082: // 
				qDebug() << "0x82 " <<cmdrx->length << endl;

				if (cmdrx->data[10] == 1)
				{
					memcpy(&cf_rc_status, cmdrx->data, sizeof(STRU_RC_STATUS));
				}

				break;
}
		delete cmdrx;
	}
}

bool cf_checksum(CMDBuffPackage* cmdrx)
{
	STRU_WIRELESS_MSG_HEADER msg_rx_head;
	memcpy(&msg_rx_head, cmdrx->data, 10);

	uint16_t sum = 0;
	for (int i = 0; i < msg_rx_head.msg_len; i++)
	{
		sum += cmdrx->data[i + 10];
	}

	if (sum == msg_rx_head.chk_sum)
	{
		return true;
	}
	else
	{
		return false;
	}

	
}



void CPThreadCMDParse::parse_cmd(char* buff, uint32_t length)
{
	//for (int i = 0; i < length; i++)
	//{
	//	qDebug("%d = 0x%02x \r\n", i, (uint8_t)buff[i]);
	//}

	//qDebug("0x%02x \r\n",(uint8_t)buff[0]);
	if ((uint8_t)buff[0] != 0xA5)
	{
		return;
	}

	uint8_t len = (uint8_t)buff[3];

	qDebug("len = %d  %d\r\n", buff[1], buff[3]);
	//qDebug("msgid = 0x%02x \r\n", buff[2]);

	len -= 2;

	switch ((uint8_t)buff[2]) {
	case MSGID_SKY2GND_FLYSTATE_V3:

		parse_cmd_msgid_flystate(&buff[4], len);
		break;

	default:
		emit signalupdateTextUi(" new , CMD  TO PARSE");
		break;
	}
}


extern fly_state_v3_t g_fly_state;
void CPThreadCMDParse::parse_cmd_msgid_flystate(char* buff, uint8_t length)
{
	memcpy(&g_fly_state, buff, sizeof(fly_state_v3_t));
	emit signalupdateFlyState();
}