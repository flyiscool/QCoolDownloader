
#include <QFileDialog>
#include <QDateTime>
#include <QThread>
#include <windows.h>
#include <stdio.h>
#include <QDebug>
#include <QMutex>
#include <QMetaType>
#include "cpUpdataApp.h"



#define VID_COOLFLY		0xAAAA
#define PID_CP_F1_GND   0xAA97
#define IFGROUND	0

typedef struct UPGRADE {

    unsigned char flag;
    unsigned char H_id;
    unsigned char L_id;
    unsigned char state1;
    unsigned char state2;

}UPGRADE;

typedef struct PARSE_PKG {

    unsigned char head[8];
    unsigned int head_len;

    char* usr_data;
    int  usr_data_len;

}PARSE_PKG;

typedef struct
{
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
}MD5_CTX;


void CPThreadUsbMonitor::setfilename(QString fileNames)
{
    file = fileNames;
}

void CPThreadUsbMonitor::threadCPUsbMonitor_main(void)
{
    emit signalupdateTextUi("begin download...");
    ctx = NULL;
    devGround = NULL;
    int r = libusb_init(&ctx); //initialize a library session
    if (r)
    {
        emit signalupdateTextUi("libusb init libusb Error！！！");
        return;
    }

    libusb_set_debug(ctx, 3);  //set verbosity level to 3, as suggested in the documentation

    int transferred;

    int cnt_dev2 = libusb_get_device_list(NULL, &devsList); //get the list of devices


    int cnt_dev = libusb_get_device_list(NULL, &devsList); //get the list of devices
    if (cnt_dev < 0) {
        emit signalupdateTextUi("Error in enumerating devices !");
        return;
    }

    devGround = libusb_open_device_with_vid_pid(ctx, VID_COOLFLY, PID_CP_F1_GND);
    if (devGround == NULL) {
        emit signalupdateTextUi("Can't open the coolfly device ...");
        libusb_free_device_list(devsList, 1);
        return;
    }
    else
    {
        r = libusb_claim_interface(devGround, IFGROUND); //claim interface 0 (the first) of device (mine had jsut 1)
        if (r < 0) {
            emit signalupdateTextUi("Cannot Claim Interface");
            libusb_free_device_list(devsList, 1);
            devGround = NULL;

            return;
        }

        Cmd_Upgrade(devGround, &file);

    }

    if (devGround != NULL)
    {
        r = libusb_release_interface(devGround, IFGROUND); //release the claimed interface
        if (r != 0) {
            emit signalupdateTextUi("Cannot Release Interface");
        }
    }

    if (devGround != NULL)
    {
        libusb_close(devGround);
    }

    if (ctx != NULL)
    {
        //libusb_exit(ctx); //close the sessio
    }
    ctx = NULL;
}


void CPThreadUsbMonitor::run()
{
    m_isCanRun = true;
 
    threadCPUsbMonitor_main();
}


int CPThreadUsbMonitor::Cmd_Upgrade(libusb_device_handle* dev, QString* fileNames)
{
    unsigned char state = 0;
    unsigned char buffer[512];
    unsigned char pkg_ret[512];

    QDateTime current_date_time;
    QString current_date;
    
    if (*fileNames == NULL)
    {
        current_date_time = QDateTime::currentDateTime();
        current_date = current_date_time.toString("hh:mm:ss");
        emit signalupdateTextUi(current_date + " no upgrade file");
        return -1;
    }

    //start upgrade

   if (Get_Upgrade_Version(dev, pkg_ret) < 0)
    {
        qDebug() << "Cmd_Upgrade_V1 need" << endl;
        return 0;
    }
    /*
    else if(pkg_ret[0] == '1' && pkg_ret[1] == '.' && pkg_ret[2] == '0' && pkg_ret[3] == '1')
    {
        Cmd_Upgrade_V2(port,dev,upgrade_file);
    }
    */
    //version 1.0.1 ande version 1.0.2
    else if (pkg_ret[0] == '1' && pkg_ret[1] == '.' && pkg_ret[2] == '0' && (pkg_ret[3] == '2' || pkg_ret[3] == '1'))
    {
       qDebug() << "Cmd_Upgrade_V3 need-----------" << endl;
       Cmd_Upgrade_V3(dev, fileNames);
    }
    else
    {
       QString temp;
       temp.sprintf("version err : %02x %02x %02x %02x", pkg_ret[0] , pkg_ret[1], pkg_ret[2], pkg_ret[3]);
       emit signalupdateTextUi(temp);
    }

   return -1;
}


#define ENDPOINT_CTRL_IN	0x84 
#define ENDPOINT_CTRL_OUT	0x01 

int CPThreadUsbMonitor::Get_Upgrade_Version(libusb_device_handle* dev, unsigned char* buf)
{
    int ret;
    int i;
    unsigned char buffer[512];
    unsigned char pkg_ret[512];
    int r;

    unsigned char tbuff[512];
    int transferred= 1;

    do {
        r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, tbuff, 512, &transferred, 1000);
    } while (transferred != 0);

    //check protocol
    buffer[0] = 0xff;
    buffer[1] = 0x5a;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
    buffer[4] = 0x01;
    buffer[5] = 0x00;
    buffer[6] = 0x00;
    buffer[7] = 0x00;
    buffer[8] = 0x00;
    buffer[9] = 0x00;

    r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_OUT, buffer, 10, &transferred, 1000);
    if (transferred != 10)
    {
        emit signalupdateTextUi("usb send err!!!");
        return -1;
    }

    Sleep(100);

    r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, pkg_ret, 512, &transferred, 1000);
    
    if (transferred > 0)
    {
        if (pkg_ret[0] == 0xff && pkg_ret[1] == 0x5a && pkg_ret[2] == 0x00 && pkg_ret[3] == 0x00)
        {
            memcpy(buf, &pkg_ret[11], 4);
            return 0;
        }
    }

    return -1;

}



typedef struct IMAGE_HEADER
{
    char data[8];
    char version[4];
    char bootmode[2];
    char appsize[4];
    char MD5[16];
    char reserve[222];
}IMAGE_HEADER;


typedef struct UPGRADE_HEADER
{
    char image_size[4];
    char EfuseMD5[16];
    char MD5[16];
}UPGRADE_HEADER;


typedef struct FilePart
{
    char* file_data;
    unsigned int size;
}FilePart;

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

static int Get_File_Size(QString* filename)
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


int CPThreadUsbMonitor::Cmd_Upgrade_V3(libusb_device_handle* dev, QString* upgrade_file)
{
    int fd;
    int rret;
    int i;
    int ret;
    int total_part = 0;
    unsigned int length;
    unsigned int fsize;
    unsigned int file_len = 0;
    unsigned int file_len_total = 0;
    unsigned int sky_len;
    unsigned int grd_len;
    unsigned char pkg_ret[512];
    unsigned char upd_map[] = { 0x02,0x03,0x04,0x00,0x05 };

    MD5_CTX ctx;
    char bret;

    IMAGE_HEADER image_upd_header;
    IMAGE_HEADER image_updbak_header;
    IMAGE_HEADER image_sky_header;
    IMAGE_HEADER image_grd_header;
    IMAGE_HEADER image_appbak_header;

    UPGRADE_HEADER upgrade_upd_header;
    UPGRADE_HEADER upgrade_updbak_header;
    UPGRADE_HEADER upgrade_app_header;
    UPGRADE_HEADER upgrade_appbak_header;


    unsigned char tbuff[512];
    int transferred = 1;
    int r;

    qDebug() << "Cmd_Upgrade_V3----------------------"<< endl;
    do {
        r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, tbuff, 512, &transferred, 1000);
    } while (transferred != 0);


    //boot not in total file
    total_part++;

    //upgrade send data
    fsize = Get_File_Size(upgrade_file);
 
    

    
    //boot upgrade
    //if (fsize < 0xa000)
    //    return Cmd_Upgrade_Boot(port, dev, upgrade_file);
    
    QString temp;
    temp.sprintf("get file size = %d \r\n", fsize);
    emit signalupdateTextUi(temp);

    //app upgrade
    if (fsize < 0x300000)
    {
        qDebug() << "fsize < 0x300000" << endl;
        return Cmd_Upgrade_V2(dev, upgrade_file);
    }

    return -1;
}


int CPThreadUsbMonitor::Cmd_Upgrade_V2(libusb_device_handle* dev, QString* upgrade_file)
{
    int i, j;
    int fsize;
    int fsize2 = 0;
    FILE*  fd;
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

    qDebug() << "Cmd_Upgrade_V2--------" << endl;

    do {
        r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, tbuff, 512, &transferred, 1000);
    } while (transferred != 0);

    //upgrade send data
    fsize = Get_File_Size(upgrade_file);

    qDebug() << "get file size: " << fsize << endl;

    totalframe = (fsize + 495) / 496;

    std::string str = upgrade_file->toStdString();
    const char* file = str.c_str();
    
    UTF8ToUnicode(file, strUnicode);
    fd = _wfopen(strUnicode, L"rb");
    
    if (fd < 0)
    {
        qDebug() << "upgrade file not exist " << fsize << endl;

        return -1;
    }

    while (1)
    {
        if (need_retry == 0)
        {
            length = fread(buffer + 16, 1, 496, fd);

            if (length <= 0)
            {
                qDebug() << "length <= 0" << endl;
                fclose(fd);
                return -1;
            }
            //fsize2 += (unsigned int)length;
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
            emit signalupdateTextUi("waiting for the ACK...");
        }
            
        for (j = 0; j < wait_time; j++)
        {
            Sleep(10);
            
            qDebug() << "nCurrentFrame = " << nCurrentFrame <<endl;
            
            r = libusb_interrupt_transfer(dev, ENDPOINT_CTRL_IN, pkg_ret, 512, &transferred, 100);
            
            qDebug() << "r = " << r << endl;

            if ((r < 0)&&(nCurrentFrame == 0))
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
                        temp.sprintf("progress : %d / %d\n", nCurrentFrame, totalframe);
                        emit signalupdateTextUi(temp);
                        emit signalupdateTextUi("upgrade successed");

                        m_isCanRun = false;
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

                        return -1;
                    }
                    //common pkg failed
                    else
                    {
                        j = wait_time;
                        break;
                    }
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
                return -1;
            }
        }
        else
            need_retry = 0;

        QString temp2;
        temp2.sprintf("progress : %d / %d\n", nCurrentFrame, totalframe);
        emit signalupdateTextUi(temp2);

    }

    return 0;

}
