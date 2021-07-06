#pragma once

#include "stdafx.h"

#include <QMetaType>

//设定为4字节对齐


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

typedef struct {
    uint64_t timestamp;             // update time in us
    float temperature;              // temperature in degrees celsius
    float pressure;                 // barometric pressure, already temp. comp.
    float altitude;                 // altitude, already temp. comp.
} baro_t;

typedef struct {
    uint64_t timestamp;       // update time in ms

    float x;                // pitch control    -1..1
    float y;                // roll control     -1..1
    float z;                // throttle control  0..1
    float r;                // yaw control      -1..1
    float scroll[2];        // scroll control   -1..1
    int8_t offset[2];       // roll-pitch offset @rad
    uint16_t button;        // button state     @bit
    uint8_t gear;           // control gear     1..3
    uint8_t rssi;           // signal strength  0..100
    uint8_t regained;       // signal regained
} manual_t;






typedef struct {
    uint64_t timestamp;             // update time in us
    float temperature;              // temperature in degrees celsius
    float x, y, z;                  // body frame acceleration in m/s^s
    float x_raw, y_raw, z_raw;      // raw sensor value of acceleration
} accel_t;

typedef struct {
    uint64_t timestamp;             // update time in us
    float temperature;              // temperature in degrees celsius
    float x, y, z;                  // body frame angular rate in rad/s
    float x_raw, y_raw, z_raw;      // raw sensor value of angular rate
} gyro_t;

typedef struct {
    uint64_t timestamp;             // update time in us
    float temperature;              // temperature in degrees celsius
    float x, y, z;                  // body frame magetic field in Guass
    float x_raw, y_raw, z_raw;      // raw sensor value of magnetic field
} mag_t;

typedef struct {
    uint64_t timestamp;             // update time in us
    uint64_t timestamp_time_relative;
    uint64_t time_utc_usec;         // timestamp (ms, UTC), this is the timestamp which comes from th gps module, It might be unvailable right after cold start, indicated by a value of 0
    int32_t lat;                    // latitude in 1e-7 degrees
    int32_t lon;                    // longitude in 1e-7 degrees
    int32_t alt;                    // altitude in 1e-3 meters above MSL (millimeters)
    int32_t alt_ellipsoid;          // altitude in 1e-3 meters blove Ellipsoid (millimeters)
    float s_variance_m_s;           // gps speed accuracy estimate (m/s)
    float c_variance_rad;           // gps course accuracy estimate (radians)
    uint8_t fix_type;               // 0-1: no fix   2: 2D fix   3: 3D fix   4: RTCM code diffenrential   5: RTK float   6: RTK fixed
    float eph;                      // gps horizontal position accuracy (meters)
    float epv;                      // gps vertical position accuracy (meters)
    float hdop;                     // horizontal dilution of precision
    float vdop;                     // vertical dilution of precision
    int32_t noise_per_ms;           // gps noise per millisecond
    int32_t jamming_indicator;      // indicates jamming is occurring
    float vel_m_s;                  // gps ground speed (m/s)
    float vel_n_m_s;                // gps north velocity (m/s)
    float vel_e_m_s;                // gps east velocity (m/s)
    float vel_d_m_s;                // gps down velocity (m/s)
    float cog_rad;                  // course over ground (not heading, but direction of movement), -PI..PI (radians)
    uint8_t vel_ned_valid;             // true if NED velocity is valid
    uint8_t satellites_used;        // number of satellites used
} gps_t;

//#pragma pack(4)

typedef  struct {
    uint32_t magichead;
    baro_t  baro;
    accel_t accel;
    gyro_t  gyro;
    mag_t 	mag;
    gps_t 	gps;
    manual_t manual0;
    uint32_t end;
} flight_data_t ; 

//#pragma pack()

typedef struct {
    uint64_t timestamp;     // time since system start in us
    double lat;             // latitude in degrees
    double lon;             // longitude in degrees
    float alt;              // altitude in meters (AMSL)

    float x;                // x coordinate in meters
    float y;                // y coordinate in meters
    float z;                // z coordinate in meters

    float yaw;              // yaw angle in radians

    uint8_t valid_alt;      // true when the altitude has been set
    uint8_t valid_hpos;
    uint8_t manual_home;		// true when home position was set manually
} home_t;

typedef struct {
    float picth;		/* 俯仰角度 单位: 度 */
    float roll;     	/* 横滚角度 单位: 度 */
    float yaw;      	/* 航向角 单位: 度	*/
    float flyspeed; 	/* 水平速度 单位: m/s */

    //-------------
    float vspeed; 		/* 垂直速度 单位: m/s */
    float altitude; 	/* 高度 单位: m */
    float battary_voltage; 	/* 电压 单位: v */
    float in_gas;          	/* 无需关心的数据   */

    //-------------
    float rx_rssi;        	/* 接收机信号强度 范围 : 0-10 */
    float gps_num;        	/* 飞行器的GPS卫星数量       */
    float hdop; 	/* GPS的精度 范围:!!! 0-250, 小于150为优秀,大于150为不好 */
    float vdop; 	/* GPS的精度 范围:!!! 0-250, 小于150为优秀,大于150为不好 */

    //-------------
    uint32_t lon; 	/* 飞行器的经纬度 the lag\lng           */
    uint32_t lat; 	/* 飞行器的经纬度 the lag\lng           */

    uint8_t flymode;	/* 0:手动,1:姿态,2:高度,3:GPS,4:巡航,5:AOC无头,6:环绕,7:RTH返航 8: 跟随 9: 任务模式 */
    uint8_t is_moto_unlock;  	/* 马达是否启动 */
    uint8_t is_acc_cali_req;  	/* 水平校准请求 */
    uint8_t is_imu_cali_req;  	/* 陀螺仪 校准请求 */

    uint8_t tip_no_gps;   		/* 提示无GPS			*/
    uint8_t is_mag_cali_req;	/* 指南针校准请求 */
    uint8_t is_low_voltage;  	/* 低电压报警 */
    uint8_t mag_cali_state;  	/* 指南针校准方向 1:水平校准 , 2:垂直校准        */

    //-------------

    uint8_t rc_is_failed;  	/* 遥控信号丢失          */
    uint8_t is_ahrc_rst;  	/* AHRS 正在初始化           */
    uint8_t is_alt_failed;  /* 飞行器高度失控            */
    uint8_t nav_state;  	/* 导航状态 3:跟随 2:没有导航数据 1:暂停 0:航点飞行中 */

    float   gps_head;        /* GPS飞行方向单位 度  */
    float   home_distance;   /* 飞行距离单位: 米    */
    float   homd_head;       /* 回航角度 单位:度,范围+-180 */

    //-------------

    
    uint32_t fly_time_sec;   /* 飞行时间 单位: 秒          */

    uint8_t gyro_failed;   	/*  陀螺仪错误 	*/
    uint8_t baro_failed;   	/*  气压计错误  */
    uint8_t mag_failed;   	/*  地磁错误    */
    uint8_t optflow_failed; /*  光流错误    */

    uint8_t gps_failed;   	/*  GPS错误    */
    uint8_t is_landing;   	/*  正在降落   	*/
    uint8_t is_uping;   	/*  正在上升   	*/
    uint8_t is_return;   	/*  返航中      */

    uint8_t is_circle_fly;  /*  环绕中      */
    uint8_t is_low_vtg2;   	/* 二级低电压报警	*/
    uint8_t reserve1;   	/*       	*/
    uint8_t reserve2;   	/*       	*/

    //-------------

    home_t home;

} fly_state_v3_t;

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



#define CF_PRO_MSGID_APP_CRTL   0x07

#define CF_PRO_MSGID_APP_MISSION    0x08

#define CF_MSGID_PARAMS1   0x09
#define CF_MSGID_PARAMS2   0x0A



#define CF_PRO_MSGID_CAMERA      0x28
#define     MSGID_CAMERA_TAKE_PIC       0x01
#define     MSGID_CAMERA_RECORD_START   0x02
#define     MSGID_CAMERA_RECORD_STOP    0x00


#define  MSGID_SKY2GND_FLYSTATE_V3   0x82    // new





typedef struct {
    uint64_t timestamp;             // update time in us
    float x;                        // pitch control    -1..1  俯仰控制
    float y;                        // roll control     -1..1   横滚控制
    float z;                        // throttle control  0..1   油门控制
    float r;                        // yaw control      -1..1   偏航控制

    float offset_x;                 // pitch offset -1..1   俯仰偏移
    float offset_y;                 // roll offset -1..1    横滚偏移
    float offset_r;                 // yaw offset -1..1     偏航偏移
    uint8_t app_stick_valid;        // 虚拟摇杆有效标志  1则有效，0则无效。

    uint8_t cmd_arm;                // 解锁命令
    uint8_t cmd_stop;               // 急停命令
    uint8_t cmd_takeoff;            // 起飞命令
    uint8_t cmd_land;               // 着陆命令
    uint8_t cmd_rtl;                // 返航命令
    uint8_t cmd_hdg;                // 航线模式命令
    uint8_t cmd_follow;             // 跟随命令
    uint8_t cmd_mission;            // 任务命令
    uint8_t cmd_circle;             // 环绕命令

    uint8_t cmd_light;              // 开关灯命令
    uint8_t cmd_gimbal;             //  未知待研究

    uint8_t cali_level;             //水平校准命令
    uint8_t cali_mag;               // 磁力计校准命令
  

    int32_t lat;                    // latitude in 1e-7 degrees
    int32_t lon;                    // longitude in 1e-7 degrees

    float circle_radius;           // 环绕半径
    float gps_acc;                  // gps的加速度
    float yaw;                      // 偏航角
} app_t;



typedef struct {
    uint64_t timestamp;         // update time (us)
    uint16_t count;             // mission numbers
    uint16_t index;             // current mission index
    int32_t lat[10];            // latitude in 1e-7 degrees
    int32_t lon[10];            // longitude in 1e-7 degrees

    float distance;             // mission distance (m)
} mission_t;


typedef struct
{
    uint8_t magic_header[2]; // 固定头
    uint16_t msg_id; // 
    uint8_t packet_num; // 消息包总数
    uint8_t packet_cur; // 当前消息包序号
    uint16_t msg_len; // payload 的总长度
    uint16_t msg_chksum; // payload 校验值
    uint8_t valid; //
    uint8_t link_status; // 天地链接状态指示安卓APP通信及功能整理.md 2020/11/27
    uint8_t vedio_status; // 天地链接状态指示安卓APP通信及功能整理.md 2020/11/27
    uint8_t gnd_sig_quality; // 图传信号质量
    uint8_t sky_sig_quality; // 遥控信号质量
    uint8_t gnd_rssia;
    uint8_t gnd_rssib;
    uint8_t sky_rssia;
    uint8_t sky_rssib;
    uint8_t sky_error_score; // sky error rate percent
    uint8_t gnd_error_score; //grd error rate percent
} STRU_RC_STATUS;





typedef struct {
    uint32_t magic_head;
    // attitude control

    float att_p[3];

    float rate_p[3];
    float rate_i[3];
    float rate_d[3];
    float rate_ff[3];
    float rate_int_lim[3];
    float auto_rate_max[3];
    float mc_rate_max[3];

    // position control

    float thr_hover;
    float thr_min;
    float thr_max;

    float tilt_max_air;
    float tilt_max_land;

    float tko_speed;
    float land_speed;

    // takeoff altitude
    float tko_alt;

    // low battery throttle
    float battery_low1;
    float battery_low2;
    float battery_critical;

    float pos_p[3];

    float vel_p[3];
    float vel_i[3];
    float vel_d[3];
    float vel_ff[3];
    float vel_i_max[3];

} param1_t;

typedef struct {
    float dec_hor_max;
    float acc_hor_max;
    float acc_z_p;
    float acc_z_i;
    float acc_z_d;
    float acc_up_max;
    float acc_down_max;


    float vel_hor_max;
    float vel_up_max;
    float vel_down_max;

    float man_vel_xy_max;
    float man_tilt_max;
    float man_yaw_max;


    float global_yaw_max;

    // sensor calibration

    float gyro_offset[3];
    float gyro_scale[3];
    float accel_offset[3];
    float accel_scale[3];
    float mag_offset[3];
    float mag_scale[3];

    float coe;

    // vehicle offset
    float roll_offset;
    float pitch_offset;

    // limits
    float limit_altitude;
    float limit_altitude_rtl;
    float limit_distance;
    int limit_newer;

    // armed counter
    int armed_counter;

    // manual control scale
    float man_tilt_max_scale[3];
    float man_yaw_max_scale[3];

    float vel_hor_max_scale[3];
    float vel_up_max_scale[3];
    float vel_down_max_scale[3];


    uint32_t magic_end;
} param2_t;


typedef struct {

    uint32_t magic_head;
    // attitude control

    float att_p[3];

    float rate_p[3];
    float rate_i[3];
    float rate_d[3];
    float rate_ff[3];
    float rate_int_lim[3];
    float auto_rate_max[3];
    float mc_rate_max[3];

    // position control

    float thr_hover;
    float thr_min;
    float thr_max;

    float tilt_max_air;
    float tilt_max_land;

    float tko_speed;
    float land_speed;

    // takeoff altitude
    float tko_alt;

    // low battery throttle
    float battery_low1;
    float battery_low2;
    float battery_critical;

    float pos_p[3];

    float vel_p[3];
    float vel_i[3];
    float vel_d[3];
    float vel_ff[3];
    float vel_i_max[3];

    float dec_hor_max;
    float acc_hor_max;
    float acc_z_p;
    float acc_z_i;
    float acc_z_d;
    float acc_up_max;
    float acc_down_max;


    float vel_hor_max;
    float vel_up_max;
    float vel_down_max;

    float man_vel_xy_max;
    float man_tilt_max;
    float man_yaw_max;


    float global_yaw_max;

    // sensor calibration

    float gyro_offset[3];
    float gyro_scale[3];
    float accel_offset[3];
    float accel_scale[3];
    float mag_offset[3];
    float mag_scale[3];

    float coe;

    // vehicle offset
    float roll_offset;
    float pitch_offset;

    // limits
    float limit_altitude;
    float limit_altitude_rtl;
    float limit_distance;
    int limit_newer;

    // armed counter
    int armed_counter;

    // manual control scale
    float man_tilt_max_scale[3];
    float man_yaw_max_scale[3];

    float vel_hor_max_scale[3];
    float vel_up_max_scale[3];
    float vel_down_max_scale[3];

    uint32_t magic_end;

} param_t;

typedef struct
{
    QStringList top_labelList;

    //item list
    QList<QTreeWidgetItem*> items;

    // item
    QTreeWidgetItem att_p;
} TAB_PARAMS_SET;