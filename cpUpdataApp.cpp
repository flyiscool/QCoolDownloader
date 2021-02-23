
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <QMutex>
#include <QMetaType>
#include "cpUpdataApp.h"
#include "cpCoolflyMonitor.h"

#define VID_COOLFLY		0xAAAA
#define PID_CP_F1_GND   0xAA97
#define IFGROUND	0

#define ENDPOINT_CTRL_IN	0x84 
#define ENDPOINT_CTRL_OUT	0x01 

wchar_t strUnicode[260];
bool UTF8ToUnicode(const char* UTF8, wchar_t* strUnicode)
{
    DWORD dwUnicodeLen;    //转换后Unicode的长度
    TCHAR* pwText;      //保存Unicode的指针
   // wchar_t* strUnicode;    //返回值
    //获得转换后的长度，并分配内存
    dwUnicodeLen = MultiByteToWideChar(CP_UTF8, 0, UTF8, -1, NULL, 0);
    pwText = new TCHAR[dwUnicodeLen];
    if (!pwText)
    {
        return false;
    }
    //转为Unicode
    MultiByteToWideChar(CP_UTF8, 0, UTF8, -1, pwText, dwUnicodeLen);
    //转为CString
    wcscpy(strUnicode, pwText);
    //清除内存
    delete[]pwText;
    return true;
}

int Get_File_Size(QString* filename)
{
    std::string str = filename->toStdString();
    const char* file = str.c_str();

    UTF8ToUnicode(file, strUnicode);
    FILE* fp = _wfopen(strUnicode, L"rb");

    if (!fp)return-1;

    fseek(fp, 0L, SEEK_END);

    int size = ftell(fp);

    fclose(fp);

    return size;
}



int CPThreadCoolflyMonitor::CMD_UPGRADE_LOCAL_APP(libusb_device_handle* dev, QString* upgrade_file)
{
    int i, j;
    int fsize;
    int fsize2 = 0;
    FILE* fd;
    int ret;

    unsigned char buffer[512];
    unsigned char pkg_ret[512];
    int totalframe;
    int nCurrentFrame = 0;
    int length;
    unsigned int sum;
    int wait_time;
    int need_retry = 0;

    unsigned char tbuff[512];
    int transferred = 1;
    int r;

    do {
        r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, tbuff, 512, &transferred, 1000);
        if ((r != 0) && (r != LIBUSB_ERROR_TIMEOUT))
        {
            return r;
        }
    } while (transferred != 0);

    //upgrade send data
    fsize = Get_File_Size(upgrade_file);

    totalframe = (fsize + 495) / 496;

    std::string str = upgrade_file->toStdString();
    const char* file = str.c_str();

    UTF8ToUnicode(file, strUnicode);
    fd = _wfopen(strUnicode, L"rb");

    if (fd < 0)
    {
        emit signalupdateTextUi("upgrade file not exist ");
        return 0;
    }

    while (m_isCanRun)
    {
        if (need_retry == 0)
        {
            length = fread(buffer + 16, 1, 496, fd);

            if (length <= 0)
            {
                emit signalupdateTextUi("length <= 0");
                fclose(fd);
                return 0;
            }

            length += 6; //data length

            buffer[0] = 0xff;
            buffer[1] = 0x5a;
            buffer[2] = 0x01;
            buffer[3] = 0x00;
            buffer[4] = 0x01;
            buffer[5] = 0x00;
            buffer[6] = (char)length;
            buffer[7] = (char)(length >> 8);

            buffer[10] = 0;
            buffer[11] = 0;
            buffer[12] = (char)nCurrentFrame;
            buffer[13] = (char)(nCurrentFrame >> 8);
            buffer[14] = (char)totalframe;
            buffer[15] = (char)(totalframe >> 8);

            //add check for frame
            for (i = 0, sum = 0; i < length; i++)
            {
                sum += (unsigned char)buffer[i + 10];
            }
            buffer[8] = (char)sum;
            buffer[9] = (char)(sum >> 8);
        }


        ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, buffer, length + 10, &transferred, 1000);

        if (nCurrentFrame < (totalframe - 1))
        {
            wait_time = 200;
        }
        else
        {
            wait_time = 2000;
            emit signalupdateTextUi("waiting for the ACK... about 10 secound ");
        }

        for (j = 0; j < wait_time; j++)
        {
            r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, pkg_ret, 512, &transferred, 100);
            if ((r < 0) && (nCurrentFrame == 0))
            {
                ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, buffer, length + 10, &transferred, 1000);
            }

            if (transferred > 0)
            {
                //pkg okg
                if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x01)
                {
                    nCurrentFrame++;
                    if (nCurrentFrame == totalframe)
                    {
                        QString temp;
                        temp.sprintf("progress : %d / %d ", nCurrentFrame, totalframe);
                        emit signalupdateTextUi(temp);
                        emit signalupdateTextUi("upgrade successed");

                        update_app_flag = false;
                        emit signalreboot();

                        return 0;
                    }
                    else
                        break;
                }
                //pkg failed
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x00)
                {
                    //last pkg failed
                    if (wait_time == 2000)
                    {
                        emit signalupdateTextUi("========================================================");
                        emit signalupdateTextUi("upgrade failed ,Please restart the module and update it");
                        emit signalupdateTextUi("========================================================");

                        return 0;
                    }
                    //common pkg failed
                    else
                    {
                        j = wait_time;
                        break;
                    }
                }
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 5) // end
                {
                    emit signalupdateTextUi("upgrade successed");

                    update_remote_app_flag = false;
                    emit signalreboot();
                }
                //pkg
                //pkg no ack
                else
                    continue;
            }
        }

        if (j == wait_time)
        {
            need_retry++;
            if (need_retry > 5)
            {

                emit signalupdateTextUi("========================================================");
                emit signalupdateTextUi("upgrade failed ,Please restart the module and update it");
                emit signalupdateTextUi("========================================================");
                update_app_flag = false;
                return 0;
            }
        }
        else
            need_retry = 0;

        if ((nCurrentFrame % 300 == 0) || (nCurrentFrame == (totalframe -1)))
        {
            QString temp2;
            temp2.sprintf("progress : %d / %d", nCurrentFrame, totalframe);
            emit signalupdateTextUi(temp2);
        }

    }

    return 0;
}

uint8_t retry_buffer[4096][512];
uint16_t retry_length[4096];


int CPThreadCoolflyMonitor::CMD_UPGRADE_REMOTE_APP(libusb_device_handle* dev, QString* upgrade_file)
{
    int i, j;
    int fsize;
    int fsize2 = 0;
    FILE* fd;
    int ret;

    memset(retry_buffer, 0, 4096 * 512);
    memset(retry_length, 0, 4096);

    unsigned char buffer[512];
    unsigned char pkg_ret[512];
    int totalframe;
    int nCurrentFrame = 0;
    uint16_t length;
    unsigned int sum;
    int wait_time;
    int need_retry = 0;

    unsigned char tbuff[512];
    int transferred = 1;
    int r;

    

    do {
        r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, tbuff, 512, &transferred, 1000);
        if ((r != 0) && (r != LIBUSB_ERROR_TIMEOUT))
        {
            return r;
        }
    } while (transferred != 0);

    //upgrade send data
    fsize = Get_File_Size(upgrade_file);

    totalframe = (fsize + 495) / 496;

    std::string str = upgrade_file->toStdString();
    const char* file = str.c_str();

    UTF8ToUnicode(file, strUnicode);
    fd = _wfopen(strUnicode, L"rb");

    if (fd < 0)
    {
        emit signalupdateTextUi("upgrade file not exist ");
        return 0;
    }

    
    uint16_t  lost_num = 0;
    uint16_t lost_length = 0;

    while (m_isCanRun)
    {
        if (need_retry == 0)
        {
            memset(buffer, 0, 512);
            length = fread(buffer + 16, 1, 496, fd);

            if (length <= 0)
            {
                emit signalupdateTextUi("length <= 0");
                fclose(fd);
                return 0;
            }

            length += 6; //data length

            buffer[0] = 0xff;
            buffer[1] = 0x5a;
            buffer[2] = 0x01;
            buffer[3] = 0x00;
            buffer[4] = 0x01;
            buffer[5] = 0x00;
            buffer[6] = (char)length;
            buffer[7] = (char)(length >> 8);

            buffer[10] = 01;    // remote app
            buffer[11] = 0;
            buffer[12] = (char)nCurrentFrame;
            buffer[13] = (char)(nCurrentFrame >> 8);
            buffer[14] = (char)totalframe;
            buffer[15] = (char)(totalframe >> 8);

            //add check for frame
            for (i = 0, sum = 0; i < length; i++)
            {
                sum += (unsigned char)buffer[i + 10];
            }
            buffer[8] = (char)sum;
            buffer[9] = (char)(sum >> 8);
        }

        memset(retry_buffer[i], 0, 512);

        for (i = 0; i < 512; i++)
        {
            retry_buffer[nCurrentFrame][i] = buffer[i];
        }
        
        retry_length[nCurrentFrame] = length + 10;

        ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, retry_buffer[nCurrentFrame], retry_length[nCurrentFrame], &transferred, 1000);

        if (nCurrentFrame < (totalframe - 1))
        {
            wait_time = 200;
        }
        else
        {
            wait_time = 2000;
            emit signalupdateTextUi("waiting for the ACK... about 10 secound ");
        }

        for (j = 0; j < wait_time; j++)
        {
            Sleep(40);
            r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, pkg_ret, 512, &transferred, 100);
            if ((r < 0) && (nCurrentFrame == 0))
            {
                ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, buffer, length + 10, &transferred, 1000);
            }

            if (transferred > 0)
            {
                qDebug() << "pkg_ret[10] "<< pkg_ret[10] << endl;
                //pkg okg
                if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x01)
                {
                    nCurrentFrame++;
                    if (nCurrentFrame == totalframe)
                    {
                        QString temp;
                        temp.sprintf("progress : %d / %d ", nCurrentFrame, totalframe);
                        emit signalupdateTextUi(temp);
                        emit signalupdateTextUi("upgrade successed");

                        update_remote_app_flag = false;
                        emit signalreboot();

                        return 0;
                    }
                    else
                        break;
                }
                //pkg failed
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x00)
                {
                    //last pkg failed
                    if (wait_time == 2000)
                    {
                        emit signalupdateTextUi("========================================================");
                        emit signalupdateTextUi("upgrade failed ,Please restart the module and update it");
                        emit signalupdateTextUi("========================================================");

                        return 0;
                    }
                    //common pkg failed
                    else
                    {
                        j = wait_time;
                        break;
                    }
                }
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x02) // lost data
                {
                    lost_length = (pkg_ret[13] << 8 | pkg_ret[12]);
                    lost_num = (pkg_ret[15] << 8 | pkg_ret[14]);

                    for (int n = 0; n < lost_length; n++)
                    {
                        qDebug() << "lost_num" << lost_length << " = " << (pkg_ret[15 + 2 * n] << 8 | pkg_ret[14 + 2 * n]) << endl;
                        lost_num = (pkg_ret[15 + 2 * n] << 8 | pkg_ret[14 + 2 * n]);

                        QString temp3;
                        temp3.sprintf("resend lost pkg : %d / %d", lost_num, lost_length);
                        emit signalupdateTextUi(temp3);
                        ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, retry_buffer[lost_num], retry_length[lost_num], &transferred, 1000);
                        Sleep(40);
                    }

                    ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, retry_buffer[totalframe - 1], retry_length[totalframe - 1], &transferred, 1000);
                    emit signalupdateTextUi("waiting for the ACK... about 10 secound ");

                    goto retry_send;
                }
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 5) // end
                {
                    emit signalupdateTextUi("upgrade successed");

                    update_remote_app_flag = false;
                    emit signalreboot();
                }
                //pkg no ack
                else
                    continue;
            }
        }

        if (j == wait_time)
        {
            need_retry++;
            if (need_retry > 5)
            {

                emit signalupdateTextUi("========================================================");
                emit signalupdateTextUi("upgrade failed ,Please restart the module and update it");
                emit signalupdateTextUi("========================================================");
                update_remote_app_flag = false;
                return 0;
            }
        }
        else
            need_retry = 0;

        if ((nCurrentFrame % 10 == 0) || (nCurrentFrame == (totalframe - 1)))
        {
            QString temp2;
            temp2.sprintf("progress : %d / %d", nCurrentFrame, totalframe);
            emit signalupdateTextUi(temp2);
        }

    }

    return 0;



retry_send:

    while(m_isCanRun)
    {
        wait_time = 2000;
        
        for (j = 0; j < wait_time; j++)
        {
            Sleep(40);
            r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, pkg_ret, 512, &transferred, 100);

            if (transferred > 0)
            {
                qDebug() << "pkg_ret2[10] " << pkg_ret[10] << endl;
                //pkg okg
                if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x01)
                {
                    break;
                }
                //pkg failed
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x00)
                {
                    //last pkg failed
                    if (wait_time == 2000)
                    {
                        emit signalupdateTextUi("========================================================");
                        emit signalupdateTextUi("upgrade failed ,Please restart the module and update it");
                        emit signalupdateTextUi("========================================================");

                        return 0;
                    }
                    //common pkg failed
                    else
                    {
                        j = wait_time;
                        break;
                    }
                }
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 0x02) // lost data
                {
                    lost_length = (pkg_ret[13] << 8 | pkg_ret[12]);
                    lost_num = (pkg_ret[15] << 8 | pkg_ret[14]) ;

                    for (int n = 0; n < lost_length; n++)
                    {
                        qDebug() << "lost_num" << lost_length << " = "<< (pkg_ret[15+2*n] << 8 | pkg_ret[14+2*n]) << endl;
                        lost_num = (pkg_ret[15 + 2 * n] << 8 | pkg_ret[14 + 2 * n]);

                        QString temp3;
                        temp3.sprintf("resend lost pkg : %d / %d", lost_num, lost_length);
                        emit signalupdateTextUi(temp3);
                        ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, retry_buffer[lost_num], retry_length[lost_num], &transferred, 1000);
                        Sleep(40);
                    }
         
                    ret = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, retry_buffer[totalframe - 1], retry_length[totalframe - 1], &transferred, 1000);
                    emit signalupdateTextUi("waiting for the ACK... about 10 secound ");
                }
                else if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a
                    && pkg_ret[2] == 0x01 && pkg_ret[3] == 0x00 && pkg_ret[10] == 5) // end
                {
                    emit signalupdateTextUi("upgrade successed");

                    update_remote_app_flag = false;
                    emit signalreboot();

                    return 0;
                }
                //pkg no ack
                else
                    continue;
            }
        }

        QString temp2;
        temp2.sprintf("resend lost pkg : %d / %d", lost_num, lost_length);
        emit signalupdateTextUi(temp2);

    }



}

