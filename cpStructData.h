#pragma once

#include "stdafx.h"

#include <QMetaType>

//#pragma pack(4)//设定为4字节对齐


#define MAX_USB_BULK_SIZE  4*1024     // 32Kbyte 
//#define MAX_USB_BULK_SIZE  1024*1024    // 1Mbyte 
//#define MAX_USB_BULK_SIZE  512*1024     // 512Kbyte 


enum State_LED {
    idle,
    normal,
    error,
    end,
};
Q_DECLARE_METATYPE(State_LED)

// data strcut
enum RxTxMode {
    SkyTxMode,
    GndRxMode,
};


enum UsbStatus {
    NoDeviceFind,
    ConnectSuccess,
    ErrorInOpen,
    ErrorIncommunication,
};
Q_DECLARE_METATYPE(UsbStatus)



struct CMDBuffPackage
{
    int packageID;
    int length;
    uint8_t data[512];
};

struct DebugBuffPackage
{
    int packageID;
    int length;
    uint8_t data[4096];
};


struct ImgPackage
{
    int packageID;
    double timeStamp;
    QImage img;
};


#define MAX_FRAME_IN_LIST_TO_SHOW	200

#define     MAX_CH_COUNT        (40)

typedef struct
{
    uint16_t        magic_header;
    uint16_t        msg_id;
    uint8_t         packet_num;
    uint8_t         packet_cur;
    uint16_t        msg_len;
    uint16_t        chk_sum;
} STRU_WIRELESS_MSG_HEADER;


typedef struct
{
    QString    hardware_ver;
    QString    sdk_ver;
    QString    sdk_build_time;
    QString    boot_ver;
    QString    boot_build_time;
}STRU_VER_INFO;



typedef struct
{
    uint32_t nodeId;
    uint32_t nodeElemCnt;
    uint32_t nodeDataSize;
}STRU_cfgNode;

typedef struct
{
    uint8_t u8_upLinkSwitch;        //0:disable 1:enable
    uint8_t u8_upLinkKeyArray[32];
    uint8_t u8_downLinkSwitch;      //0:disable 1:enable
    uint8_t u8_downLinkKeyArray[32];
    uint8_t reserve[2];
} STRU_BB_AES;

typedef struct
{
    uint32_t                        u32_rfChCount;                  //valid channel count
    int16_t                         u16_rfChFrqList[MAX_CH_COUNT];     //channel frq(MHz)
} STRU_RF_CHANNEL;

typedef enum 
{
    RF_POWER_OPEN = 0,    //open mode
    RF_POWER_CLOSE = 1     //close mode
} ENUM_RF_POWER_MODE;

typedef enum
{
    FCC = 0,
    CE = 1,
    SRRC = 2,
    OTHER = 3
} ENUM_RF_POWER_WORK_MODE;

typedef struct
{
    uint8_t mode;            //mode:  open or close
    uint8_t powerAutoCtrl;              //Enable or disable open power control
    uint8_t tssi_update_time;

    uint8_t power_work_mode;

    uint8_t rcPowerFcc[2];           //close power FCC  for band0, band1
    uint8_t rcPowerCe[2];            //close power CE   for band0, band1
    uint8_t rcPowerSrrc[2];          //close power srrc for band0, band1
    uint8_t rcPowerOther[2];          //close power srrc for band0, band1
    uint8_t vtPowerFcc[2];           //close power FCC  for band0, band1
    uint8_t vtPowerCe[2];            //close power CE   for band0, band1
    uint8_t vtPowerSrrc[2];          //close power srrc for band0, band1
    uint8_t vtPowerOther[2];          //close power srrc for band0, band1
} STRU_RF_POWER_CTRL;

typedef enum
{
    AGC_GEAR_2,
    AGC_GEAR_1,
} ENUM_AGC_GEAR;

typedef struct
{
    uint8_t e_gear;
    uint8_t u8_agcSwitchThresh;
    uint8_t reserve[2];
}STRU_AGC_SET;



typedef struct
{
    uint16_t freq;
    uint16_t real_power;//power 19.5,you should input 195
}STRU_OPEN_POWER_MEAS_VALUE;

#define MAX_OPEN_POWER_REF_VALUE (10)

typedef struct
{
    uint8_t u8_powerOpenMakeupEnable;
    uint8_t u8_bbPwr[2];            //pathA: u8_bbPwr[0]  pathB: u8_bbPwr[1]
    uint8_t u8_2p4g_rfPwr[2]; //2a/2e
    uint8_t u8_5p8g_rfPwr[2]; //66/67
    uint8_t u8_2p4g_ref_power;
    uint8_t u8_5p8g_ref_power;
    uint8_t u8_2p4g_ref_point_num_a;
    STRU_OPEN_POWER_MEAS_VALUE st_2p4g_real_power_value_a[MAX_OPEN_POWER_REF_VALUE];
    uint8_t u8_2p4g_ref_point_num_b;
    STRU_OPEN_POWER_MEAS_VALUE st_2p4g_real_power_value_b[MAX_OPEN_POWER_REF_VALUE];
    uint8_t u8_5p8g_ref_point_num_a;
    STRU_OPEN_POWER_MEAS_VALUE st_5p8g_real_power_value_a[MAX_OPEN_POWER_REF_VALUE];
    uint8_t u8_5p8g_ref_point_num_b;
    STRU_OPEN_POWER_MEAS_VALUE st_5p8g_real_power_value_b[MAX_OPEN_POWER_REF_VALUE];
} STRU_BB_CH_OPEN_POWER_REF_VALUE;


typedef struct
{
    uint8_t u8_centralGain; //BB 
    uint16_t u16_regPwr;
    uint16_t real_power;
} STRU_BB_POWER_CLOSE_MEAS_VALUE;

typedef struct
{
    uint16_t u16_2g_ref_frq;
    STRU_BB_POWER_CLOSE_MEAS_VALUE st_2g_chPowerClose[40]; //for power setting 0~30dbm
    uint16_t u16_5g_ref_frq;
    STRU_BB_POWER_CLOSE_MEAS_VALUE st_5g_chPowerClose[40]; //for power setting 0~30dbm
} STRU_BB_POWER_CLOSE;

typedef enum
{
    PWR_LEVEL0 = 0,
    PWR_LEVEL1 = 1,
    PWR_LEVEL2 = 2,
    PWR_LEVEL3 = 3,
    TEST_0x10 = 0x10,
    TEST_0x11 = 0x11,
    TEST_0x12 = 0x12,
    TEST_0x13 = 0x13,
    TEST_0x14 = 0x14,
    TEST_0x20 = 0x20,
    TEST_0x21 = 0x21,
    TEST_0x22 = 0x22,
    TEST_0x23 = 0x23,
} ENUM_PWR_CTRL;

typedef struct
{
    uint16_t mod_status_update_time;
    uint8_t mod_status_update_enable;
    uint8_t pwr_ctrl;
}STRU_PWR_CTRL;

typedef enum _ENUM_RF_BAND
{
    RF_2G = 0x01,     //support only 2G 
    RF_5G = 0x02,     //support only 5G 
    RF_2G_5G = (RF_2G | RF_5G),
    RF_600M = 0x04
}ENUM_RF_BAND;

typedef enum _ENUM_CH_BW
{
    BW_10M = 0x02,
    BW_20M = 0x00,
}ENUM_CH_BW;

typedef enum
{
    AUTO,
    MANUAL
}ENUM_RUN_MODE;

typedef enum
{
    FULL_BAND,
    SUB_BAND,
}ENUM_BAND_MODE;

typedef struct
{
    uint8_t                 e_bandsupport;
    uint8_t                      u8_bbStartMcs10M;
    uint8_t                      u8_bbStartMcs20M;
    uint8_t                   e_bandwidth;
    uint8_t                e_rfbandMode;
    uint8_t                      bypass_ch_mode;
    uint8_t               frq_band_mode;
    uint8_t                      rsv[1];
} STRU_RF_BAND_MCS_OPT;

typedef enum
{
    MIMO_1T1R = 0,
    MIMO_1T2R = 1,
    MIMO_2T2R = 2
} ENUM_MIMO_MODE;

typedef enum
{
    LNA_OPEN = 0,
    LNA_BYPASS = 1,
    LNA_AUTO = 2
}ENUM_LNA_MODE;

typedef struct
{
    uint8_t st_skyMimoMode;
    uint8_t st_grdMimoMode;
    uint8_t  enum_lna_mode;
    uint8_t spi_num;
}STRU_MIMO_MODE;


typedef enum
{
    HAL_UART_BAUDR_9600 = 0,    // 0
    HAL_UART_BAUDR_19200,       // 1
    HAL_UART_BAUDR_38400,       // 2
    HAL_UART_BAUDR_57600,       // 3
    HAL_UART_BAUDR_115200,      // 4
    HAL_UART_BAUDR_230400,      // 5
    HAL_UART_BAUDR_256000,      // 6
    HAL_UART_BAUDR_380400,      // 7
    HAL_UART_BAUDR_460800       // 8
} ENUM_HAL_UART_BAUDR;

typedef struct
{
    uint8_t st_uartBaudr[4];
} STRU_UART_BAUDR;



typedef struct

{
    //AES
    STRU_cfgNode                    st_aesNode;
    STRU_BB_AES                     st_aesData;                                 //AES: must enable

    //RF power setting
    STRU_cfgNode                    st_rfPowerNode;
    STRU_RF_POWER_CTRL              st_rfPowerData;

    //rc & 10M vt channel, max 40 channel
    STRU_cfgNode                    st_band0_rcChNode;      //2.4G or 600 ~ 800MHz rc channel setting for 10M 20M
    STRU_RF_CHANNEL                 st_band0_rcChData;
    STRU_cfgNode                    st_band1_10M_rcChNode;  //5.8G rc channel setting for 10M 20M
    STRU_RF_CHANNEL                 st_band1_10M_rcChData;
    STRU_cfgNode                    st_band0_10M_vtChNode;  //2.4G 10M vt channel setting
    STRU_RF_CHANNEL                 st_band0_10M_vtChData;
    STRU_cfgNode                    st_band1_10M_vtChNode;  //5.8G 10M it channel setting
    STRU_RF_CHANNEL                 st_band1_10M_vtChData;

    //20M vt channel
    STRU_cfgNode                    st_band0_20M_vtChNode;  //2.4G 20M vt channel setting
    STRU_RF_CHANNEL                 st_band0_20M_vtChData;
    STRU_cfgNode                    st_band1_20M_vtChNode;  //5.8G 20M vt channel setting
    STRU_RF_CHANNEL                 st_band1_20M_vtChData;

    //agc switch
    STRU_cfgNode                    st_agcSetNode;
    STRU_AGC_SET                    st_agcSetData;

    //open power
    STRU_cfgNode                    st_powerOpen_vtNode;
    STRU_BB_CH_OPEN_POWER_REF_VALUE             st_powerOpen_vtData;
    STRU_cfgNode                    st_powerOpen_rcNode;
    STRU_BB_CH_OPEN_POWER_REF_VALUE             st_powerOpen_rcData;

    //close power
    STRU_cfgNode                    st_powerClose_vtNode;
    STRU_BB_POWER_CLOSE                st_powerClose_vtData;
    STRU_cfgNode                    st_powerClose_rcNode;
    STRU_BB_POWER_CLOSE             st_powerClose_rcData;

    //pwr ctrl
    STRU_cfgNode                    st_pwrCtrlNode;
    STRU_PWR_CTRL                   st_pwrCtrlData;

    //band Mcs option
    STRU_cfgNode                    st_rfBandMcsNode;
    STRU_RF_BAND_MCS_OPT            st_rfBandMcsData;
    STRU_cfgNode                    st_mimoModeNode;
    STRU_MIMO_MODE                  st_mimoModeData;
    STRU_cfgNode                    st_uartBaundNode;
    STRU_UART_BAUDR                 st_uartBaudData;
} STRU_FACTORY_SETTING_DATA;


typedef struct
{
    STRU_cfgNode                    st_factoryNode;
    STRU_FACTORY_SETTING_DATA       st_factorySetting;
} STRU_FACTORY_SETTING;


typedef struct
{
    uint8_t         skyGround;
    uint8_t         band;
    uint8_t         bandWidth;
    uint8_t         itHopMode;
    uint8_t         rcHopping;
    uint8_t         adapterBitrate;
    uint8_t         channel1_on;
    uint8_t         channel2_on;
    uint8_t         isDebug;
    uint8_t         itQAM;
    uint8_t         itCodeRate;
    uint8_t         rcQAM;
    uint8_t         rcCodeRate;
    uint8_t         ch1Bitrates;
    uint8_t         ch2Bitrates;
    uint8_t         reserved[1];//define to display power value
    uint8_t         u8_itRegs[4];
    uint8_t         u8_rcRegs[4];
    uint8_t         rvc : 4;
    uint8_t         sleep_level : 2;
    uint8_t         switch_mode_2g_5g : 1;
    uint8_t         pure_vt_valid : 1;
    uint8_t         rcId[5];
    uint8_t         chipId[5];

    uint8_t         vtid[2];
    uint8_t         en_realtime;
    uint8_t         inSearching;
    //lna_status define
    //bit[0] : 0,lna open , 1 lna close ( lna bypass).   
    //bit[2:1] : 0,always lna open mode, 1, alway lna close mode, 2 lna auto mode
    //bit[6:3] : bandedge lower power value; range value [0,15]
    //bit[7] : bandedge enable flag, 1 enable, 0 disable 
    uint8_t         lna_status;
    uint8_t         u8_startWrite;  //u8_startWrite: cpu2 start update the flag
    uint8_t         u8_endWrite;    //u8_endWrite:   cpu2 end update the flag
} STRU_DEVICE_INFO;

typedef struct
{
    QStringList top_labelList;

    //item list
    QList<QTreeWidgetItem*> items;

    // item
    QTreeWidgetItem f_uart3_baudrate;
    QStringList l_uart3_baudrate;

    QTreeWidgetItem f_uart3_bypassmode;
    QStringList l_uart3_bypassmode;

   
} TAB_FACTORY_SET;


//////////////////////////////////////////////////////

typedef	union {
    float data[9];
    struct {
        float acc_x;
        float acc_y;
        float acc_z;
        float gyro_x;
        float gyro_y;
        float gyro_z;
        float mag_x;
        float mag_y;
        float mag_z;
    };
} imu9_t;

typedef	union {
    float att[3];
    struct {
        float yaw;
        float pitch;
        float roll;
    };
} attitude3_t;


typedef	union {
    int16_t ch[5];
    struct {
        int16_t roll;
        int16_t pitch;
        int16_t throttle;
        int16_t yaw;
        int16_t mode;
    };
} rc_ch_t;


typedef	union {
    int16_t data[4];
    struct {
        int16_t roll;
        int16_t pitch;
        int16_t throttle;
        int16_t yaw;
    };
} pid_out_t;


typedef	union {
    int16_t data[4];
    struct {
        int16_t motor_back_right;
        int16_t motor_front_right;
        int16_t motor_back_left;
        int16_t motor_front_left;
    };
} motor_t;


typedef struct {
    uint8_t hw_ok;
    uint8_t init_ok;
    float temperature;
    float baro_pressure;
    float height;
    float height_comp;  //cm
} spl0601_s;


typedef struct {
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
} imu6_t;

typedef union {
    struct {
        float x;
        float y;
        float z;
    };
    float axis[3];
} Axis3f;


typedef struct {
    uint64_t timestamp; 		// required for logger
    double time_utc_usec;		// utc_usec
    double lat;				// gps latitude
    double lon;				// gps longitude
    double alt;				// gps altitude
    double alt_ellipsoid;		//	大地水准面差距
    double gps_yaw;
    double lat_err;
    double lon_err;
    double alt_err;
    double pdop;
    double hdop;
    double vdop;
    double vel_m_s;
    double cog_rad;
    double gps_yaw_real;
    double gps_yaw_mag;
    double gnd_speed_j;
    double g_speed_kmh;
    int32_t timestamp_time_relative;
    uint8_t fix_type;
    uint8_t vel_ned_valid;
    uint8_t satellites_used;
} gps_pos_s;

typedef struct {
    uint32_t ch[16];
    uint64_t timestamp;
} sky_rc_s;


typedef struct {
    imu6_t	imu6;
    Axis3f 	mag;
    spl0601_s baro;
    gps_pos_s gps_pos;
    sky_rc_s	sky_rc;
} sensorData_s;



typedef struct {
    float roll;
    float pitch;
    float yaw;
} attitude_s;

typedef struct {
    float _00;
    float _01;
    float _02;
    float _10;
    float _11;
    float _12;
    float _20;
    float _21;
    float _22;
} dcm_s;

typedef struct {
    float setHeading;
    float uAltitude;
    float u_roll;
    float u_pitch;
    float u_yaw;
} ctrl_pid_s;

typedef struct {
    double lat;
    double lon;
    double alt;
} gps_local_s;


typedef struct {
    float cm_per_deg_lat;
    float cm_per_deg_lon;
} gps_cal_s;


typedef  struct {
    double x1_hat;
    double y1_hat;
    float z1_hat;
    float z2_hat;
    float z3_hat;
    float z1_hat2;
    float z2_hat2;
    float z3_acc;
    double vx1_hat;
    double vy1_hat;
    float vx2_hat;
    float vx3_hat;
    float vy2_hat;
    float vy3_hat;
} kalman_output_s;

typedef struct {
    float CH_THR;
    float CH_AIL;
    float CH_ELE;
    float CH_RUD;
    float AUX_1;
    float AUX_2;
    float AUX_3;
    float AUX_4;
    uint64_t timestamp;
} rcf_s;


typedef  struct {
    uint32_t magichead;
    uint32_t cnt;
    attitude_s ypr;
    dcm_s dcm;
    float heading;
    ctrl_pid_s pidout;
    imu6_t e_imu;
    gps_local_s gps_home;
    gps_cal_s gps_cal;
    kalman_output_s kalman;
    rcf_s rcf;
    rcf_s rcf_cal;
    uint32_t flymode;
    motor_t motor;
    uint32_t end;
} flight_t;

typedef  struct {
    uint32_t magichead;
    uint32_t cnt;
    float ax;
    float ay;
    float az;
    float gx;
    float gy;
    float gz;
    float mx;
    float my;
    float mz;
    float yaw;
    float pitch;
    float roll;
    float u_yaw;
    float u_pitch;
    float u_roll;
    uint32_t end;
} flight_data_t;



typedef struct {
    int8_t       ATTPicth;       /* 俯仰角度 单位: 度 */
    int8_t       ATTRoll;        /* 横滚角度 单位: 度 */
    uint8_t      FlySpeed;       /* 水平速度 单位: 0.1m/s */
    uint8_t      Voltage;        /* 电压 单位: 0.1v */
    uint8_t      InGas;          /* 无需关心的数据  */
    int8_t       VSpeed;         /* 垂直速度 单位: 0.1m/s */
    uint8_t      VDOP;           /* GPS的精度 范围: 0-250, 小于150为优秀,大于150为不好 */
    union {
        uint8_t SysState;
        struct {
            uint8_t rxRssi : 4;  /* 接收机信号强度 范围 : 0-10 */
            uint8_t GpsNum : 4;  /* 飞行器的GPS卫星数量       */
        };
    } SysState2;

    union {
        uint16_t SysState;
        struct {
            uint8_t FlyMode : 4;  /* 0:手动,1:姿态,2:高度,3:GPS,4:巡航,5:AOC无头,6:环绕,7:RTH返航 */
            uint8_t IsMotoUnlock : 1;  /* 马达是否启动 */
            uint8_t IsImuCalReq : 1;  /* 陀螺仪 校准请求            */
            uint8_t IsMagCalReq : 1;  /* 指南针校准请求        */
            uint8_t IsLowVoltage : 1;  /* 低电压报警                      */
            uint8_t MagCalState : 2;  /* 指南针校准方向 1:水平校准 , 2:垂直校准        */
            uint8_t RcIsFailed : 1;  /* 遥控信号丢失          */
            uint8_t IsAhrsRst : 1;  /* AHRS 正在初始化              */
            uint8_t IsAltFailed : 1;  /* 飞行器高度失控                */
            uint8_t navState : 2;  /* 导航状态 3:跟随 2:没有导航数据 1:暂停 0:航点飞行中 */
            uint8_t IsAccCalReq : 1;  /* 水平校准请求 */
        };
    } SysState1;

    int16_t      Altitude;       /* 高度 单位: 0.1m    	                */
    int16_t      GpsHead;        /* GPS飞行方向单位 度     		          */
    int16_t      ATTYaw;         /* 航向角 单位: 度    	                */
    uint16_t     HomeDistance;   /* 飞行距离单位: 米                     */
    int16_t      HomeHead;       /* 回航角度 单位:度,范围+-180           */
    uint16_t     FlyTime_Sec;    /* 飞行时间 单位: 秒                    */
    uint8_t      VoltageHigh;    /* 飞行器电压的高8位                    */
    uint8_t      reserve2;       /*                                      */
    int32_t      Lon, Lat;       /* 飞行器的经纬度 the lag\lng           */
    //2020.06.12
    union {
        uint16_t SysState;
        struct {
            uint8_t GyroFailed : 1;   /*  陀螺仪错误     */
            uint8_t BarometerFailed : 1;   /*  气压计错误     */
            uint8_t MagFailed : 1;   /*  地磁错误       */
            uint8_t OptFlowFailed : 1;   /*  光流错误       */
            uint8_t GpsFailed : 1;   /*  GPS错误        */
            uint8_t isLanding : 1;   /*  正在降落       */
            uint8_t isUping : 1;   /*  正在上升       */
            uint8_t isReturn : 1;   /*  返航中         */
            uint8_t isCircleFly : 1;   /*  环绕中         */
            uint8_t IsLowVtg2 : 1;   /* 二级低电压报警   */
            //2020.08.28
            uint8_t Tip_NOGPS : 1;   /* 提示无GPS  */
        };
    } SysState3;
    uint8_t reserve3;            /*                                      */
    uint8_t reserve4;            /*                                      */
    /*-------------------------------------------------------------------*/
} tFlyStateData_V2;


typedef struct {
    float ATTPicth;       /* 俯仰角度 单位: 度 */
    float ATTRoll;        /* 横滚角度 单位: 度 */
    float ATTYaw;         /* 航向角 单位: 度    	                */
    float FlySpeed;       /* 水平速度 单位: 0.1m/s */
    float VSpeed;         /* 垂直速度 单位: 0.1m/s */
    float Voltage;        /* 电压 单位: 0.1v */
    float VDOP;           /* GPS的精度 范围: 0-250, 小于150为优秀,大于150为不好 */
    float Altitude;       /* 高度 单位: 0.1m    	                */
    float GpsHead;        /* GPS飞行方向单位 度     		          */
    float HomeDistance;   /* 飞行距离单位: 米                     */
    float HomeHead;       /* 回航角度 单位:度,范围+-180           */
    float FlyTime_Sec;    /* 飞行时间 单位: 秒                    */
    float Lon;       	  /* 飞行器的经纬度 the lag\lng           */
    float Lat;       	  /* 飞行器的经纬度 the lag\lng           */
    uint32_t rxRssi;	  /* 接收机信号强度 范围 : 0-10 */
    uint32_t GpsNum;	  /* 飞行器的GPS卫星数量       */
    uint32_t FlyMode;     /* 0:手动,1:姿态,2:高度,3:GPS,4:巡航,5:AOC无头,6:环绕,7:RTH返航 */
    union {
        uint32_t SysFlag;
        struct {
            uint8_t IsMotoUnlock : 1;  /* 马达是否启动 */
            uint8_t IsImuCalReq : 1;  /* 陀螺仪 校准请求            */
            uint8_t IsMagCalReq : 1;  /* 指南针校准请求        */
            uint8_t IsLowVoltage : 1;  /* 低电压报警                      */
            uint8_t MagCalState : 2;  /* 指南针校准方向 1:水平校准 , 2:垂直校准        */
            uint8_t RcIsFailed : 1;  /* 遥控信号丢失          */
            uint8_t IsAhrsRst : 1;  /* AHRS 正在初始化              */
            uint8_t IsAltFailed : 1;  /* 飞行器高度失控                */
            uint8_t navState : 2;  /* 导航状态 3:跟随 2:没有导航数据 1:暂停 0:航点飞行中 */
            uint8_t IsAccCalReq : 1;  /* 水平校准请求 */
            uint8_t GyroFailed : 1;   /*  陀螺仪错误     */
            uint8_t BarometerFailed : 1;   /*  气压计错误     */
            uint8_t MagFailed : 1;   /*  地磁错误       */
            uint8_t OptFlowFailed : 1;   /*  光流错误       */
            uint8_t GpsFailed : 1;   /*  GPS错误        */
            uint8_t isLanding : 1;   /*  正在降落       */
            uint8_t isUping : 1;   /*  正在上升       */
            uint8_t isReturn : 1;   /*  返航中         */
            uint8_t isCircleFly : 1;   /*  环绕中         */
            uint8_t IsLowVtg2 : 1;   /*  二级低电压报警  	*/
            uint8_t Tip_NOGPS : 1;   /*  提示无GPS  	  	*/
        };
    } SysFlag_u;

} cf_fly_state_s;



#define  CMD_DEVICE_INFO    0x0019
#define  CMD_CF_PROTOCOL_TX 0x0094
#define  CMD_CF_PROTOCOL_RX 0x0093



#define CF_PRO_MSGID_SYSTEM      0x01
#define     MSGID_REBOOT            0x01

#define CF_PRO_MSGID_CTRL           0x02
#define     MSGID_SET_GPS_HOME          0x01
#define     MSGID_SET_ARMED             0x02
#define     MSGID_SET_DISARMED          0x03

#define CF_PRO_MSGID_CALIBRATE   0x06
#define     MSGID_CALIBRATE_MAG     0x02
#define     MSGID_CALIBRATE_IMU     0x03
#define     MSGID_CALIBRATE_RC     0x04


#define CF_PRO_MSGID_CAMERA      0x28
#define     MSGID_CAMERA_TAKE_PIC       0x01
#define     MSGID_CAMERA_RECORD_START   0x02
#define     MSGID_CAMERA_RECORD_STOP    0x00


#define CF_PRO_MSGID_FLYSTATEDATA   0x1D

#define CF_PRO_MSGID_FLYSTATE       0x1E    // new
