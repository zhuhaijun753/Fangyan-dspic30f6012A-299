#include "SYSTEM.h"
#include "LOGO.h"
unsigned char delay_cnt;
DATA_BIT gKEY[5];
DATA_BIT gCTL[8]; //仪表输出控制信息

unsigned int single_miles = 0;
unsigned long total_miles = 0;


unsigned int Fcan_count = 0; //前部模块CAN
unsigned int Rcan_count = 0; //顶部模块CAN
unsigned int Mcan_count = 0; //中部模块CAN
unsigned int Ecan_count = 0; //发动机ECU模块CAN
unsigned int Vcan_count = 0; //整车控制器VCU模块CAN
unsigned int Bcan_count = 0; //电池管理器BMS模块CAN
unsigned int Dcan_count = 0; //转向助力DC/AC模块CAN
unsigned int Acan_count = 0; //空调AC模块CAN

unsigned int ADR[8]; //模拟量采集   10/15


DBCM_KEY fKEY; //前模块功率开关状态
DBCM_KEY mKEY; //中（顶）模块功率开关状态
DBCM_KEY rKEY; //后模块功率开关状态

unsigned int fFreq; //前模块频率
unsigned int mFreq; //中（顶）模块频率
unsigned int rFreq; //后模块频率

unsigned int fSpeed; //前模块折算车速
unsigned int mSpeed; //中（顶）模块折算车速
unsigned int rSpeed; //后模块折算车速
unsigned int pSpeed; //外部CAN这算车速

unsigned int fADR[2]; //前模块电阻值
unsigned int mADR[2]; //中（顶）模块电阻值
unsigned int rADR[2]; //后模块电阻值

unsigned int ADR[8]; //仪表模拟信号采集值    2016/10/10

DBCM_POUT fPOUT; //前模块功率输出状态
DBCM_POUT mPOUT; //中（顶）模块功率输出状态
DBCM_POUT rPOUT; //后模块功率输出状态

unsigned char fPF[16]; //前模块功率输出故障
unsigned char mPF[16]; //中（顶）模块功率故障
unsigned char rPF[16]; //后模块功率输出故障

unsigned int fpcur[15];//前模块功率输出电流
unsigned int mpcur[15];//中模块功率输出电流
unsigned int rpcur[15];//后模块功率输出电流

unsigned int fSingle_miles;
unsigned int mSingle_miles;
unsigned int rSingle_miles;

unsigned char ccvs_eng_req; //启动请求

unsigned char dm_hcu_display[32]; //故障类型
unsigned char dm_tcu_display[32];


//系统初始化函数

void System_Init(void) {
    ADPCFG = 0xffff; //关闭AD口     
    alarm_led[0].byte = 0xff; //消除开机瞬间LED和蜂鸣器闪烁
    alarm_led[1].byte = 0x00;
    alarm_led[2].byte = 0x00;
    alarm_led[3].byte = 0x00;
    LED_Out();
    /*按键控制接口 */
    KEY_UP_DR = 1; //   
    KEY_SET_DR = 1; // 
    KEY_AV_DR = 1;

    PWR_CTRL_DR = 1; //供电电压端口初始化
    PWR_CTRL = 0;

    LCD_Init();
    TIMER1_Init();
    delay_cnt = 10; //延时0.1s
    while (delay_cnt > 1) {
        ClrWdt();
        SW_Input(); //开关读取
    }
    alarm_led[0].byte = 0xff;
    alarm_led[1].byte = 0x00;
    alarm_led[2].byte = 0x00;
    alarm_led[3].byte = 0x00;
    LED_Out();
    gCTL[0].byte = 0x00;
    gCTL[1].byte = 0x00;
    gCTL[2].byte = 0x00;
    gCTL[3].byte = 0x00;
    gCTL[4].byte = 0x00;
    gCTL[5].byte = 0x00;
    gCTL[6].byte = 0x00;
    gCTL[7].byte = 0x00;
    CAN1_Init();
    CAN2_Init();
    while (1) {
        ClrWdt();
        LCD_BackLight(0x00);
        if (wake_up1) {//如果危险报警唤醒
            F_PO8 = FLASH;
            F_PO6 = FLASH;
            R_PO11 = FLASH;
            R_PO12 = FLASH;
            LED4 = FLASH; //左转向灯
            LED8 = FLASH; //右转向灯 
        } else {
            F_PO6 = 0;
            F_PO8 = 0;
            R_PO11 = 0;
            R_PO12 = 0;
            LED4 = 0; //左转向灯
            LED8 = 0; //右转向灯 
        }

        if (F50ms) {
            F50ms = 0;
            LED_Out();
            SW_Input();
            BCAN_SendCtl();
        }

        if (M_ON || wake_up2) break; // 如果是ON唤醒则正常退出

        if (wake_up1 == 0) {
            F_PO8 = 0;
            F_PO6 = 0;
            R_PO11 = 0;
            R_PO12 = 0;
            BCAN_SendCtl();
            __delay_us(100);
            BCAN_SendCtl();
            __delay_us(100);
            BCAN_SendCtl();
            __delay_us(100);
            BCAN_SendCtl();
            __delay_us(100);
            PWR_CTRL_DR = 1; //关闭电源
            PWR_CTRL = 0; //关闭电源 
            PWR_CTRL_DR = 1; //关闭电源
            PWR_CTRL = 0; //关闭电源 
            PWR_CTRL_DR = 1; //关闭电源
            PWR_CTRL = 0; //关闭电源 
        }
    }

    delay_cnt = 100; //设置1s定时 
    SM_Init();
    SM_Reset();
    BL_Init(); //仪表小灯背光初始化
    INT1_Init();
    INT2_Init();
    TIMER3_Init();
    EEROM_ReadSingleMiles();
    EEROM_ReadTotalMiles();
    EEROM_ReadHMPulse();
    EEROM_ReadBackLight();
    single_miles = e_single_miles;
    total_miles = e_total_miles;
    OC1RS = e_backlight; //更新背光值
    ADC_Init();
    LCD_Init();
    alarm_led[0].byte = 0xC0; //蜂鸣器开启
    alarm_led[1].byte = 0xff;
    alarm_led[2].byte = 0xff;
    alarm_led[3].byte = 0xff;
    LED_Out();
    while (delay_cnt > 1); //延时1s
    OC1RS = 0;
    PWR_CTRL_DR = 0; //电源控制接口
    PWR_CTRL = 1; //开启MCU电源控制
    alarm_led[0].byte = 0xff;
    alarm_led[1].byte = 0x00;
    alarm_led[2].byte = 0x00;
    alarm_led[3].byte = 0x00;
    LED_Out();
    SW_Input();
    LCD_BackLight(7);
    DS3231_ReadTime(&timer); //读取时间   
}

void SYSTME_Logic(void) {
    // F_PO1 = (IN16 && M_ON); //雨刮快档
    // F_PO2 = (IN14 && M_ON); //雨刮慢档
    // F_PO3 = (IN21 && M_ON); //喷水电机
    F_PO4 = (IN19 && IN9 && (IN4 || M_ON)); //左前雾灯
    F_PO5 = (IN20 && (pSpeed < 2) && (IN4 || M_ON)); //前面开电磁阀
    F_PO6 = (FLASH && (wake_up1 || (IN18 && (IN4 || M_ON)))); //右转向灯
    F_PO7 = ((IN28 || (pSpeed > 2)) && (IN4 || M_ON)); //前门关电磁阀
    F_PO8 = (FLASH && (wake_up1 || (IN17 && (IN4 || M_ON)))); //左转向灯
    F_PO9 = (IN19 && (IN4 || M_ON)); //位置灯
    F_PO10 = ((Current_Gear == 124)&& (IN4 || M_ON)); //倒车监视器电源
    F_PO11 = (IN23 && (IN4 || M_ON)); //右近光灯
    F_PO12 = (IN23 && (IN4 || M_ON)); //左近光灯
    F_PO13 = (IN24 && (IN4 || M_ON)); //左远光
    F_PO14 = (IN24 && (IN4 || M_ON)); //右远光
    F_PO15 = (IN19 && IN2 && (IN4 || M_ON)); //前踏步灯
    /*
        M_PO1 = (IN12 && (IN4 || M_ON)); //广告灯
        M_PO2 = (FLASH && (IN5 || wake_up1) && (IN4 || M_ON)); //右转向灯
        M_PO3 = ((mKH1 || mKL15) && (IN4 || M_ON)); //车内指示灯电源
        M_PO4 = ((fKL6 || rKL6) && (IN4 || M_ON)); //制动灯
        M_PO5 = (IN20 && (moto_speed<50) && (IN4 || M_ON)); //前门开电磁阀
        M_PO6 = (IN19 && (IN4 || M_ON)); //前路牌
        M_PO7 = (IN28 && (IN4 || M_ON)); //前门关电磁阀
        M_PO8 = (IN19 && (IN4 || M_ON)); //侧路牌
        M_PO9 = (IN27 && (moto_speed<50) && (IN4 || M_ON)); //中门开电磁阀
        M_PO10 = (IN19 && (IN4 || M_ON)); //后路牌
        M_PO11 = (IN26 && (IN4 || M_ON)); //中门关电磁阀
        M_PO12 = (IN19 && (IN4 || M_ON)); //滚动电子
        M_PO13 = (IN6 && (IN4 || M_ON)); //小灯
        M_PO14 = (IN19 && (IN4 || M_ON)); //后广告屏
        M_PO15 = (FLASH && (IN1 || wake_up1) && (IN4 || M_ON)); //左转向灯
     */
    R_PO1 = IN4; //干燥器电源
    R_PO2 = (IN27 && (pSpeed < 2) && (IN4 || M_ON)); //后门开电磁阀
    R_PO3 = ((IN26 || (pSpeed > 2)) && (IN4 || M_ON)); //后门关电磁阀     
    R_PO4 = (IN19 && IN3 && (IN4 || M_ON)); //后踏步灯
    R_PO5 = (IN11 && (IN4 || M_ON)); //室内灯1
    R_PO6 = (IN19 && IN9 && IN10); //后雾灯
    R_PO7 = ((Current_Gear == 124) && (IN4 || M_ON)); //倒车灯
    R_PO8 = ((pSpeed < 5) && (IN4 || M_ON)); //门泵电源
    R_PO9 = ((Brake_Pedal || rKL1) && (IN4 || M_ON)); //制动灯
    if ((rADR[0] < 1908) || IN13) { //风扇
        R_PO10 = 1;
    } else if ((rADR[0] > 2202) || !IN13)
        R_PO10 = 0;
    R_PO11 = (FLASH && (wake_up1 || (IN17 && (IN4 || M_ON)))); //左转向灯
    R_PO12 = (FLASH && (wake_up1 || (IN18 && (IN4 || M_ON)))); //右转向灯
    R_PO13 = (IN19 && (IN4 || M_ON)); //位置灯
    R_PO14 = (IN12 && (IN4 || M_ON)); //室内灯2
    R_PO15 = (((IN1 && (tcu_start_allow == 1) && (hcu_starter_forbid != 1)) || (ENG_start_req == 21415) || (rKL15 && (Current_Gear == 125))) && (IN4 || M_ON)); //前启动
    R_PO16 = 1;//发电机励磁输出

}

void Count_ADR(void) {
    float ad = 4096.0;
    //电压型传感器采集  ADR[]是实际的电压*1000
    //ADR[1] = (unsigned int) (read_ADR(1) / ad * 5000);
    //ADR[2] = (unsigned int) (read_ADR(2) / ad * 5000);
    //ADR[3] = (unsigned int) (read_ADR(3) / ad * 5000);
    //ADR[4] = (unsigned int) (read_ADR(4) / ad * 5000);
    ADR[5] = (read_ADR(5) / ad * 5000);
    if (ADR[5] < 500)//0.5V偏移
        ADR[5] = 0;
    else ADR[5] = ADR[5] - 500;
    
    ADR[6] = (read_ADR(6) / ad * 5000);
    if (ADR[6] < 500)//0.5V偏移
        ADR[6] = 0;
    else ADR[6] = ADR[6] - 500;
    //蓄电池电压    
    ADR[0] = (unsigned int) (550.0 * read_VOL() / ad); //实际电压放大10倍   //(read_VOL() - 1300)*10 / 76 + 180;
    //电阻型传感器采集  ADR[]是实际的阻值
    //ADR[1] = (unsigned int) (100.0*read_ADR(1)/(ad-read_ADR(1)));//((read_ADR(1) / ad * 100) / (1 - read_ADR(1) / ad));
    //ADR[2] = (unsigned int) (100.0*read_ADR(2)/(ad-read_ADR(2)));
    //ADR[3] = (unsigned int) (100.0*read_ADR(3)/(ad-read_ADR(3)));
    //ADR[4] = (unsigned int) (100.0*read_ADR(4)/(ad-read_ADR(4)));
    //ADR[5] = (unsigned int) (100.0*read_ADR(5)/(ad-read_ADR(5)));
    //ADR[6] = (unsigned int) (100.0*read_ADR(6)/(ad-read_ADR(6)));
    //if (ADR[1] > 1000)
    //    ADR[1] = 0;
    //if (ADR[2] > 1000)
    //    ADR[2] = 0;
    //if (ADR[3] > 1000)
    //    ADR[3] = 0;
    //if (ADR[4] > 1000)
    //    ADR[4] = 0;
    //if (ADR[5] > 1000)
    //    ADR[5] = 0;
    //if (ADR[6] > 1000)
    //    ADR[6] = 0;
    
}

void LED_Logic(void) {

    //LED1 = ((ADR[5] < 15) && M_ON); //油量过低报警

    //LED2 = 0;空档
    //LED3 = (IN16 && IN29 && M_ON); //排气制动

    //左转向
    if ((IN17 && M_ON) || wake_up1)
        LED4 = FLASH;
    else LED4 = 0;

    //LED5 = (IN11 && M_ON); //进气预热

    //机油压力 
    if (!Engine_wait) {
        if (IN33 && M_ON)//机油报警
            LED6 = 1;
        else if ((Engine_oil_press * 4 <= 70) && M_ON)
            LED6 = FLASH_1s;
        else LED6 = 0;
    } else LED6 = 0;

    if ((water_temp > 138) && M_ON)//水温过高报警
        LED7 = 1;
    else LED7 = 0;
    //右转向灯
    if ((IN18 && M_ON) || wake_up1)
        LED8 = FLASH;
    else LED8 = 0;

    if ((rKL3 || (ADR[5]/40 < 60)) && M_ON)//气压1低信号
        LED9 = 1;
    else
        LED9 = 0;

    LED10 = wake_up2; //总电源指示   

    if ((rKL4 || (ADR[6]/40 < 60)) && M_ON) //气压2
        LED11 = 1;
    else
        LED11 = 0;

    //发动机故障指示
    /*
    if (Engine_warn == 1 && M_ON)
        LED12 = 1;
    if (Engine_warn == 0)
        LED12 = 0;

    if (Maintain_warn == 1 && M_ON) {//维护报警
        LED13 = 1;
    } else if (Maintain_warn == 0)LED13 = 0;

    //STOP停机报警
    if (Stop_warn == 1 && M_ON) {
        LED14 = 1;
    } else if (Stop_warn == 0)LED14 = 0;
    //等待启动//没有确定信号源  
    if (Engine_wait == 1 && M_ON) {
        LED15 = 1;
    } else if (Engine_wait == 0) LED15 = 0;
*/
    LED16 = (!rKH1 && M_ON); //蓄电池充电

    LED17 = (IN24 && M_ON); //远光灯

    LED18 = (rKL2 && M_ON); //手刹

    LED19 = (IN9 && M_ON); //前雾灯

    LED20 = (IN10 && M_ON); //后雾灯

    LED21 = ((Current_Gear == 124) && M_ON); //倒车灯

    LED22 = (IN23 && M_ON); //近光灯

    //LED23 = (IN14 && IN31 && M_ON);//缓速器

    LED24 = ((Brake_Pedal || rKL1 || rKL12) && M_ON); //刹车指示


    //蜂鸣器 为0时响，为1时不响   
    if (IN22)
        BUZZ = 1;
    else if ((mil_buzz_request) && !IN22)
        BUZZ = 0;
    else if((IN17 || IN18) && !IN22)
        BUZZ = !FLASH;
    else if((!rKL5 || rKL6 || rKL8 || rKL9 || !rKL10 || rKL11) && !IN22)
        BUZZ = !kFLASH;
    else if((rKL3 || rKL4) && !IN22)
        BUZZ = !FLASH_1s;
    else
        BUZZ = 1;
    //

    if (IN2) {
        PO2 = 0;
    } else {
        PO2 = 1;
    }

    if (IN19) {
        PO1 = 0;
    } else {
        PO1 = 1;
    }

    if (IN19)
        OC1RS = e_backlight;
    else OC1RS = 0;
    
    //启动 
    /*
    if (((tcu_start_allow == 1)&&(hcu_starter_forbid != 1)) || (ENG_start_req == 21415))
        PO1 = 0;
    else if (hcu_starter_forbid == 1) PO1 = 1;
    else PO1 = 1;
     */
    //if (Brake_Pedal == 0) PO2 = 1;
    //else if (Brake_Pedal == 1) PO2 = 0;



    mil_lamp_request = (CAN_18FECAEF.bData[0] >> 2)&0x03;
    fail_mode = (CAN_18FECAEF.bData[0] >> 4)&0x03;
    mil_buzz_request = (CAN_18FECAEF.bData[0] >> 6)&0x03;

    Lamp_status_YELLOW = (CAN_18FECA03.bData[0] >> 2)&0x03;
    Lamp_status_RED = (CAN_18FECA03.bData[0] >> 4)&0x03;

}

void BCAN_SendCtl(void) {
    CAN_MESSAGE message_POUT;
    message_POUT.id = 0x6a4;
    message_POUT.data[0] = gCTL[0].byte;
    message_POUT.data[1] = gCTL[1].byte;
    message_POUT.data[2] = gCTL[2].byte;
    message_POUT.data[3] = gCTL[3].byte;
    message_POUT.data[4] = gCTL[4].byte;
    message_POUT.data[5] = gCTL[5].byte;
    message_POUT.data[6] = ((M_ON && wake_up3) || IN4) + (IN14 << 1)+(IN15 << 2)+(IN16 << 3)+(IN21 << 4); //模块雨刮控制
    message_POUT.data[7] = 0;
    message_POUT.len = 8;
    message_POUT.type = 1;
    CAN1_SendMessage(&message_POUT);
}

void BCAN_send_mile(void) {
    CAN_MESSAGE msg;
    msg.data[0] = (unsigned char) (e_total_miles); //低八位
    msg.data[1] = (unsigned char) (e_total_miles >> 8); //二级八位
    msg.data[2] = (unsigned char) (e_total_miles >> 16); //三级八位
    msg.data[3] = (unsigned char) (e_total_miles >> 24); //高八位
    msg.data[4] = 0;
    msg.data[5] = 0;
    msg.data[6] = 0;
    msg.data[7] = 0;
    msg.id = 0x454;
    msg.len = 0x08;
    msg.type = 0x01;
    CAN1_SendMessage(&msg);

}

void PCAN_CCVS(void) {
    CAN_MESSAGE message_CCVS;
    DATA_BIT CAN_Send;
    message_CCVS.id = 0x18FEF117;
    CAN_Send.bit1 = 0;
    CAN_Send.bit2 = 0;
    CAN_Send.bit3 = rKL2;//手刹
    CAN_Send.bit4 = 0;
    CAN_Send.bit5 = 1;//空调
    CAN_Send.bit6 = 1;//空调
    CAN_Send.bit7 = 0;
    CAN_Send.bit8 = 0;
    message_CCVS.data[0] = CAN_Send.byte;
    
    message_CCVS.data[1] = (unsigned char) (pSpeed * 256);
    message_CCVS.data[2] = (unsigned char) (pSpeed * 256 >> 8);

    CAN_Send.bit1 = 1; //发动机禁止启动
    CAN_Send.bit2 = 1; //发动机禁止启动
    CAN_Send.bit3 = rKL7; //后舱门 
    CAN_Send.bit4 = 0; //后舱门
    CAN_Send.bit5 = rKL1; //脚刹
    CAN_Send.bit6 = 0; //脚刹
    CAN_Send.bit7 = 0;
    CAN_Send.bit8 = 0;
    message_CCVS.data[3] = CAN_Send.byte;

    CAN_Send.bit1 = 0; //限速70
    CAN_Send.bit2 = 0; //限速70
    CAN_Send.bit3 = rKL6; //充电口盖 
    CAN_Send.bit4 = 0; //充电口盖 
    CAN_Send.bit5 = 1; //前门
    CAN_Send.bit6 = 1; //前门
    CAN_Send.bit7 = 1; //后门
    CAN_Send.bit8 = 1; //后门
    message_CCVS.data[4] = CAN_Send.byte;
    
    
    message_CCVS.data[5] = (unsigned char) (ADR[5]/40); //前气压
    message_CCVS.data[6] = (unsigned char) (ADR[6]/40); //后气压
    message_CCVS.data[7] = 0xff;
    message_CCVS.len = 8;
    message_CCVS.type = 0;
    CAN2_SendMessage(&message_CCVS);
}

void PCAN_send_mile(void) {
    CAN_MESSAGE msg;
    msg.data[0] = (unsigned char) (e_total_miles); //低八位
    msg.data[1] = (unsigned char) (e_total_miles >> 8); //二级八位
    msg.data[2] = (unsigned char) (e_total_miles >> 16); //三级八位
    msg.data[3] = (unsigned char) (e_total_miles >> 24); //高八位
    msg.data[4] = (unsigned char) (e_single_miles); //低八位; 
    msg.data[5] = (unsigned char) (e_single_miles >> 8); //二级八位;
    msg.data[6] = 0x00;
    msg.data[7] = 0x00;
    msg.id = 0x18EBFFD7;
    msg.len = 0x08;
    msg.type = 0x00;
    CAN2_SendMessage(&msg);

}

void DM(unsigned char x, DM1 id) {//解析出id的第x个故障码的spn和fmi
    SPN = id.bData[2 + 4 * (x - 1)]
            +((unsigned int) id.bData[3 + 4 * (x - 1)] << 8)
            +((unsigned long) (id.bData[4 + 4 * (x - 1)]&0xE0) << 11);
    FMI = (id.bData[4 + 4 * (x - 1)] & 0x1F);
}

/*
void DM_HCU(unsigned char x) {//解析出第x个故障码的spn和fmi
    hcu_spn = CAN_18FECAEF.bData[2 + 4 * (x - 1)]
            +((unsigned int) CAN_18FECAEF.bData[3 + 4 * (x - 1)] << 8)
            +((unsigned long) (CAN_18FECAEF.bData[4 + 4 * (x - 1)]&0xE0) << 11);
    hcu_fmi = (CAN_18FECAEF.bData[4 + 4 * (x - 1)] & 0x1F);
    //if(hcu_spn==103) hcu_spn=0;//随州那边103故障码无法消除
}

void DM_TCU(unsigned char x) {//解析出第x个故障码的spn和fmi
    tcu_spn = CAN_18FECA03.bData[2 + 4 * (x - 1)]
            +((unsigned int) CAN_18FECA03.bData[3 + 4 * (x - 1)] << 8)
            +((unsigned long) (CAN_18FECA03.bData[4 + 4 * (x - 1)]&0xE0) << 11);
    tcu_fmi = (CAN_18FECA03.bData[4 + 4 * (x - 1)] & 0x1F);
}

void DM_CCU(unsigned char x) {//解析出第x个故障码的spn和fmi
    ccu_spn = CAN_18FECAF5.bData[2 + 4 * (x - 1)]
            +((unsigned int) CAN_18FECAF5.bData[3 + 4 * (x - 1)] << 8)
            +((unsigned long) (CAN_18FECAF5.bData[4 + 4 * (x - 1)]&0xE0) << 11);
    ccu_fmi = (CAN_18FECAF5.bData[4 + 4 * (x - 1)] & 0x1F);
}

void DM_ECU(unsigned char x) {//解析出第x个故障码的spn和fmi
    ecu_spn = CAN_18FECA00.bData[2 + 4 * (x - 1)]
            +((unsigned int) CAN_18FECA00.bData[3 + 4 * (x - 1)] << 8)
            +((unsigned long) (CAN_18FECA00.bData[4 + 4 * (x - 1)]&0xE0) << 11);
    ecu_fmi = (CAN_18FECA00.bData[4 + 4 * (x - 1)] & 0x1F);
}
 */
void __attribute__((__interrupt__, auto_psv)) _C1Interrupt(void) {
    unsigned char i = 0, j = 0;
    CAN_MESSAGE message;
    if (C1INTFbits.ERRIF) { //接收错误
        C1INTFbits.ERRIF = 0; // 添加错误处理代码
        CAN1_Init();
        C1RX0CONbits.RXFUL = 0;
    }
    if (C1INTFbits.IVRIF) {
        C1INTFbits.IVRIF = 0; //数据无效
        C1RX0CONbits.RXFUL = 0;
    }
    if (C1INTFbits.WAKIF) {
        C1INTFbits.WAKIF = 0;
    }


    if (C1RX0CONbits.RXFUL) { //数据有效
        CAN1_ReceiveMessage(&message);

        switch (message.id) {
            case 0x681:
                Fcan_count = 0;
                fKEY.BYTES[0] = message.data[0];
                fKEY.BYTES[1] = message.data[1];
                fKEY.BYTES[2] = message.data[2];
                fFreq = message.data[3]+((unsigned int) message.data[4] << 8);
                fSpeed = message.data[5]+((unsigned int) message.data[6] << 8);
                break;
            case 0x682:
                Mcan_count = 0;
                mKEY.BYTES[0] = message.data[0];
                mKEY.BYTES[1] = message.data[1];
                mKEY.BYTES[2] = message.data[2];
                mFreq = message.data[3]+((unsigned int) message.data[4] << 8);
                mSpeed = message.data[5]+((unsigned int) message.data[6] << 8);
                break;
            case 0x683:
                Rcan_count = 0;
                rKEY.BYTES[0] = message.data[0];
                rKEY.BYTES[1] = message.data[1];
                rKEY.BYTES[2] = message.data[2];
                rFreq = message.data[3]+((unsigned int) message.data[4] << 8);
                rSpeed = message.data[5]+((unsigned int) message.data[6] << 8);
                break;
            case 0x671:
                Fcan_count = 0;
                fADR[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                fADR[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                break;
            case 0x672:
                Mcan_count = 0;
                mADR[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                mADR[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                break;
            case 0x673:
                Rcan_count = 0;
                rADR[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                rADR[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                break;
            case 0x621:
                Fcan_count = 0;
                fpcur[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                fpcur[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                fpcur[2] = message.data[4]+((unsigned int) message.data[5] << 8);
                fpcur[3] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x631:
                Fcan_count = 0;
                fpcur[4] = message.data[0]+((unsigned int) message.data[1] << 8);
                fpcur[5] = message.data[2]+((unsigned int) message.data[3] << 8);
                fpcur[6] = message.data[4]+((unsigned int) message.data[5] << 8);
                fpcur[7] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x641:
                Fcan_count = 0;
                fpcur[8] = message.data[0]+((unsigned int) message.data[1] << 8);
                fpcur[9] = message.data[2]+((unsigned int) message.data[3] << 8);
                fpcur[10] = message.data[4]+((unsigned int) message.data[5] << 8);
                fpcur[11] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x651:
                Fcan_count = 0;
                fpcur[12] = message.data[0]+((unsigned int) message.data[1] << 8);
                fpcur[13] = message.data[2]+((unsigned int) message.data[3] << 8);
                fpcur[14] = message.data[4]+((unsigned int) message.data[5] << 8);
                break;
                case 0x622:
                Mcan_count = 0;
                mpcur[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                mpcur[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                mpcur[2] = message.data[4]+((unsigned int) message.data[5] << 8);
                mpcur[3] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x632:
                Mcan_count = 0;
                mpcur[4] = message.data[0]+((unsigned int) message.data[1] << 8);
                mpcur[5] = message.data[2]+((unsigned int) message.data[3] << 8);
                mpcur[6] = message.data[4]+((unsigned int) message.data[5] << 8);
                mpcur[7] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x642:
                Mcan_count = 0;
                mpcur[8] = message.data[0]+((unsigned int) message.data[1] << 8);
                mpcur[9] = message.data[2]+((unsigned int) message.data[3] << 8);
                mpcur[10] = message.data[4]+((unsigned int) message.data[5] << 8);
                mpcur[11] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x652:
                Mcan_count = 0;
                mpcur[12] = message.data[0]+((unsigned int) message.data[1] << 8);
                mpcur[13] = message.data[2]+((unsigned int) message.data[3] << 8);
                mpcur[14] = message.data[4]+((unsigned int) message.data[5] << 8);
                break;
                case 0x623:
                Rcan_count = 0;
                rpcur[0] = message.data[0]+((unsigned int) message.data[1] << 8);
                rpcur[1] = message.data[2]+((unsigned int) message.data[3] << 8);
                rpcur[2] = message.data[4]+((unsigned int) message.data[5] << 8);
                rpcur[3] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x633:
                Rcan_count = 0;
                rpcur[4] = message.data[0]+((unsigned int) message.data[1] << 8);
                rpcur[5] = message.data[2]+((unsigned int) message.data[3] << 8);
                rpcur[6] = message.data[4]+((unsigned int) message.data[5] << 8);
                rpcur[7] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x643:
                Rcan_count = 0;
                rpcur[8] = message.data[0]+((unsigned int) message.data[1] << 8);
                rpcur[9] = message.data[2]+((unsigned int) message.data[3] << 8);
                rpcur[10] = message.data[4]+((unsigned int) message.data[5] << 8);
                rpcur[11] = message.data[6]+((unsigned int) message.data[7] << 8);
                break;
            case 0x653:
                Rcan_count = 0;
                rpcur[12] = message.data[0]+((unsigned int) message.data[1] << 8);
                rpcur[13] = message.data[2]+((unsigned int) message.data[3] << 8);
                rpcur[14] = message.data[4]+((unsigned int) message.data[5] << 8);
                break;
            case 0x561:
                Fcan_count = 0;
                fPOUT.BYTES[0] = message.data[0];
                fPOUT.BYTES[1] = message.data[1];
                for (j = 0; j < 4; j++) {//输出状态采集
                    for (i = 0; i < 4; i++) {
                        fPF[i + 4 * j] = (message.data[j + 2] >> (i * 2))&0x03;
                    }
                }
                break;
            case 0x562:
                Mcan_count = 0;
                mPOUT.BYTES[0] = message.data[0];
                mPOUT.BYTES[1] = message.data[1];
                for (j = 0; j < 4; j++) {
                    for (i = 0; i < 4; i++) {
                        mPF[i + 4 * j] = (message.data[j + 2] >> (i * 2))&0x03;
                    }
                }
                break;
            case 0x563:
                Rcan_count = 0;
                rPOUT.BYTES[0] = message.data[0];
                rPOUT.BYTES[1] = message.data[1];
                for (j = 0; j < 4; j++) {
                    for (i = 0; i < 4; i++) {
                        rPF[i + 4 * j] = (message.data[j + 2] >> (i * 2))&0x03;
                    }
                }
                break;
            case 0x451:
                Fcan_count = 0;
                fSingle_miles = message.data[4]+((unsigned int) message.data[5] << 8);
                break;
            case 0x452:
                Mcan_count = 0;
                mSingle_miles = message.data[4]+((unsigned int) message.data[5] << 8);
                break;
            case 0x453:
                Rcan_count = 0;
                rSingle_miles = message.data[4]+((unsigned int) message.data[5] << 8);
                break;

            default:
                if (message.id == e_address) {
                    if (message.data[0] == 0xC1) {
                        __asm__ volatile ("reset");
                    }
                }
                if (message.id == 0x610) {
                    if (message.data[0] == 0x55) {
                    } else if (message.data[0] == 0x56) {
                        if (message.data[1] == e_address) {
                            e_hm_pulse = message.data[2]+((unsigned int) message.data[3] << 8);
                            EEROM_WriteTotalMiles();
                        }
                    } else if (message.data[0] == 0x59) {
                        if (message.data[1] == e_address) {
                            e_total_miles = (unsigned char) message.data[2]
                                    +((unsigned int) message.data[3] << 8)
                                    +((unsigned long) message.data[4] << 16)
                                    +((unsigned long) message.data[5] << 24);
                            EEROM_WriteTotalMiles();
                        }
                    } else if (message.data[0] == 0x5A) {
                        if (message.data[1] == e_address) {
                            e_single_miles = (unsigned char) message.data[2]
                                    +((unsigned int) message.data[3] << 8);
                            EEROM_WriteSingleMiles();
                        }
                    }
                }
                break;
        }
    }
    C1INTF = 0; /* the individual flag register cleared */
    C1RX0CONbits.RXFUL = 0;
    _C1IF = 0;
}

/*******************分包处理*****************/

DM1 CAN_18FECAEF;
unsigned char CAN_18FECAEF_len;
DM1 CAN_18FECA03;
unsigned char CAN_18FECA03_len;
DM1 CAN_18FECAF5;
unsigned char CAN_18FECAF5_len;
DM1 CAN_18FECA00;
unsigned char CAN_18FECA00_len;
CAN_CACHE m_can;
CAN_CACHE n_can;
CAN_CACHE c_can;
CAN_CACHE e_can;

unsigned long SPN; //从缓存里提取的故障码
unsigned char FMI;
//unsigned char CM;
//unsigned char OC;
/*******************分包处理*****************/

unsigned char moto_voltage; //电机电压
unsigned char moto_current; //电机电流
unsigned char moto_speed; //电动转速
unsigned char battle_t; //电池均温
unsigned char Inverter_t; //逆变器温度
unsigned char battle_t_warn; //电池过温报警
unsigned char battle_jueyuan_warn; //绝缘过低报警
unsigned char battle_higtv1_warn; //高压电池1故障
unsigned char battle_higtv2_warn; //高压电池2故障
unsigned char moto_t_warn; //电机过温
unsigned char moto_ready; //ready信号
unsigned char xudc; //蓄电池异常
unsigned char moto_warn; //电机故障

unsigned char BMS_offline_warn; //电池不在线
unsigned char MCU_offline_warn; //电机不在线
unsigned char TCU_offline_warn; //变速箱不在线
unsigned char Engine_offline_warn; //发动机不在线
unsigned char Engine_wait; //发动机怠速
unsigned char EV; //模式
unsigned char Signal_charge; //充电信号
unsigned char BMS_V; //电池电压
unsigned char BMS_SOC; //电池SOC
unsigned char BMS_A; //电池电流

unsigned char mil_lamp_request; //黄色三级报警
unsigned char fail_mode; //文字报警
unsigned char mil_buzz_request; //蜂鸣器响
unsigned long hcu_spn; //HCU故障码
unsigned char hcu_fmi;

unsigned int Speed_rmp; //输出轴转速
unsigned char Eng_static_test; //发动机稳态转速测试
unsigned char tcu_start_allow; //发动机运行启动

unsigned char Current_Gear; //档位

unsigned char Lamp_status_YELLOW; //变速箱黄灯状态
unsigned char Lamp_status_RED; //变速箱红灯状态
unsigned long tcu_spn; //TCU故障码
unsigned char tcu_fmi;

unsigned long ccu_spn; //CCU故障码
unsigned char ccu_fmi;

unsigned long ecu_spn; //ECU故障码
unsigned char ecu_fmi;

unsigned int Max_Cell_Voltage; //最高单体电压
unsigned char Max_Cell_Voltage_Cell_Number; //最高单体电压位置
unsigned int  Min_Cell_Voltage; //最低单体电压
unsigned char Min_Cell_Voltage_Cell_Number; //最低单体电压位置
unsigned char Brake_percent; //踏板开度

unsigned int  Max_Cell_Temperature; //最高单体温度
unsigned char Max_Cell_Temp_Cell_Number; //最高单体温度位置
unsigned char Motor_Temperature; //电机1温度
unsigned char Motor2_Generatrix_Current; //电机2电流
unsigned char Motor2_Speed; //电机转速
unsigned char Motor2_Inverter_Temperature; //电机2逆变器温度
unsigned char Motor2_Temperature; //电机2温度

unsigned int  ENG_start_req; //仪表控制启动
unsigned long mot_work_time; //电机工作累积时间
unsigned char mot_work_percent; //电机工作时间比
unsigned char DCDC_ctrl; //DCDC状态
unsigned char hcu_starter_forbid; //起动机工作
unsigned char aircompress_enable; //压缩机
unsigned char Brake_Pedal; //刹车信号
unsigned char Speed_lim_mode; //限速模式
unsigned char Accelerator; //油门踏板

DATA_BIT OTS_DI; //OTI DI

unsigned char water_temp; //水温

unsigned int Engine_oil_press; //机油压力
unsigned char water_level; //水位

unsigned char Wait_start; //等待启动
unsigned char Maintain_warn; //维护报警
unsigned char Stop_warn; //停机报警
unsigned char Engine_warn; //发动机故障

unsigned int EngineSpeed; //发动机转速

unsigned char barometric_pressure; //增压压力 0.5Kpa               
unsigned char ambient_air_temperature; //周围空气温度
unsigned char air_pressure; //进气压力 0.5Kpa

unsigned long Trip_fuel; //单次累计燃气消耗

unsigned int Shunshi_fule; //发动机瞬时气耗
unsigned int Average_fule; //发动机平均气耗

unsigned char ABS_lamp; //ABS状态

unsigned long abs_spn; //ABS故障码

unsigned int DC_DC_voltage_in; //DCDC输入电压
unsigned char DC_DC_voltage_out; //DCDC输出电压
unsigned int DC_DC_current_out; //DCDC输出电流
unsigned char DC_DC_t; //DCDC散热器温度
DATA_BIT DC_DC_code; //DCDC故障代码

unsigned int DC_AC_voltage_in; //DCAC输入电压
unsigned int DC_AC_voltage_out; //DCAC输出电压
unsigned int DC_AC_current_out; //DCAC输出电流
unsigned char DC_AC_t; //DCAC散热器温度
DATA_BIT DC_AC_code; //DCAC故障代码

void __attribute__((__interrupt__, auto_psv)) _C2Interrupt(void) {
    unsigned char i = 0;
    CAN_MESSAGE msg;
    if (C2INTFbits.ERRIF) { //接收错误
        C2INTFbits.ERRIF = 0; // 添加错误处理代码
        CAN2_Init();
        C2RX0CONbits.RXFUL = 0;
    }
    if (C2INTFbits.IVRIF) {
        C2INTFbits.IVRIF = 0; //数据无效
        C2RX0CONbits.RXFUL = 0;
    }
    if (C2RX0CONbits.RXFUL) { //数据有效
        CAN2_ReceiveMessage(&msg);
        switch (msg.id) {
            case 0x18EF4AEF://Hybrid_Information_Message
                moto_voltage = msg.data[0];
                moto_current = msg.data[1];
                moto_speed = msg.data[3];
                battle_t = msg.data[4];
                Inverter_t = msg.data[5];
                battle_t_warn = msg.data[6]&0x03;
                battle_jueyuan_warn = (msg.data[6] >> 2)&0x03;
                battle_higtv1_warn = (msg.data[6] >> 4)&0x03;
                battle_higtv2_warn = (msg.data[6] >> 6)&0x03;
                moto_t_warn = msg.data[7]&0x03;
                moto_ready = (msg.data[7] >> 2)&0x03;
                xudc = (msg.data[7] >> 4)&0x03;
                moto_warn = (msg.data[7] >> 6)&0x03;
                break;

            case 0x18EF4BEF://Hybrid_DTC_Message
                BMS_offline_warn = msg.data[0]&0x03;
                MCU_offline_warn = (msg.data[0] >> 2)&0x03;
                TCU_offline_warn = (msg.data[0] >> 4)&0x03;
                Engine_offline_warn = msg.data[1]&0x03;
                Engine_wait = msg.data[2]&0x03;
                EV = (msg.data[2] >> 4)&0x03;
                Signal_charge = (msg.data[2] >> 6)&0x03;
                BMS_V = msg.data[3];
                BMS_SOC = msg.data[4];
                BMS_A = msg.data[7];
                break;

            case 0x18FECAEF:// DM1 HCU
                for (i = 0; i < 6; i++) {
                    CAN_18FECAEF.bData[i] = msg.data[i];
                }
                CAN_18FECAEF_len = 0x06; //m_can.len;
                break;

            case 0x1CECFFEF://TPCM HCU
                if (msg.data[0] == 0x20) {
                    if (msg.data[5] == 0xCA && msg.data[6] == 0xFE) {
                        m_can.len = msg.data[1]+((unsigned int) msg.data[2] << 8);
                        m_can.pack = msg.data[3];
                        m_can.flag = 1; //CAN接收开始标记
                    }
                }
                break;

            case 0x1CEBFFEF://TPDT HCU
                if (m_can.flag == 1) {
                    if (msg.data[0] <= m_can.pack) {
                        unsigned int slen;
                        slen = (msg.data[0] - 1)*7;
                        for (i = 0; i < 7; i++) {
                            m_can.buf[slen + i] = msg.data[i + 1];
                        }
                    }
                    if (msg.data[0] == m_can.pack) {
                        for (i = 0; i < m_can.len; i++) {
                            CAN_18FECAEF.bData[i] = m_can.buf[i];
                        }
                        CAN_18FECAEF_len = m_can.len;
                        m_can.flag = 0; //已完成接收
                    }
                }
                break;
                //////////////////////////////////////////////////////////////////////////////////            
            case 0x0CF00203://ETC1 TCU
                Speed_rmp = ((unsigned char) msg.data[1]
                        +((unsigned int) msg.data[2] << 8))*0.125; //输出轴转速
                Eng_static_test = (msg.data[7] >> 4)&0x03; //发动机稳态转速测试
                tcu_start_allow = (msg.data[7] >> 6)&0x03; //发动机运行启动
                break;


            case 0x18F00503://ETC2 TCU
                Current_Gear = msg.data[3];
                break;

            case 0x18FECA03:// DM1 TCU
                for (i = 0; i < 6; i++) {
                    CAN_18FECA03.bData[i] = msg.data[i];
                }
                CAN_18FECA03_len = 0x06; //n_can.len;
                Lamp_status_YELLOW = (CAN_18FECA03.bData[0] >> 2)&0x03;
                Lamp_status_RED = (CAN_18FECA03.bData[0] >> 4)&0x03;
                break;

            case 0x1CECFF03://TPCM TCU
                if (msg.data[0] == 0x20) {
                    if (msg.data[5] == 0xCA && msg.data[6] == 0xFE) {
                        n_can.len = msg.data[1]+((unsigned int) msg.data[2] << 8);
                        n_can.pack = msg.data[3];
                        n_can.flag = 1; //CAN接收开始标记
                    }
                }
                break;

            case 0x1CEBFF03://TPDT TCU
                if (n_can.flag == 1) {
                    if (msg.data[0] <= n_can.pack) {
                        unsigned int slen;
                        slen = (msg.data[0] - 1)*7;
                        for (i = 0; i < 7; i++) {
                            n_can.buf[slen + i] = msg.data[i + 1];
                        }
                    }
                    if (msg.data[0] == n_can.pack) {
                        for (i = 0; i < n_can.len; i++) {
                            CAN_18FECA03.bData[i] = n_can.buf[i];
                        }
                        CAN_18FECA03_len = n_can.len;
                        n_can.flag = 0; //已完成接收
                    }
                }
                break;

            case 0x18FECAF5:// DM1 CCU
                for (i = 0; i < 6; i++) {
                    CAN_18FECAF5.bData[i] = msg.data[i];
                }
                CAN_18FECAF5_len = 0x06; //i_can.len;
                break;

            case 0x18FECA00:// DM1 ECU
                for (i = 0; i < 6; i++) {
                    CAN_18FECA00.bData[i] = msg.data[i];
                }
                CAN_18FECA00_len = 0x06; //e_can.len;

                break;


                ////////////////////////////////////////////////////////////////////////////////          
            case 0x18EF4CEF://BMS Cell voltage Information Message
                Max_Cell_Voltage = (msg.data[0]+((unsigned int) msg.data[1] << 8)) & 0xfff;
                Max_Cell_Voltage_Cell_Number = msg.data[2];
                Min_Cell_Voltage = (msg.data[3]+((unsigned int) msg.data[4] << 8)) & 0xfff;
                Min_Cell_Voltage_Cell_Number = msg.data[5];
                Brake_percent = msg.data[6];
                break;

            case 0x18EF4DEF://Hybrid Temperature Information Message
                Max_Cell_Temperature = msg.data[0]+((unsigned int) msg.data[1] << 8);
                Max_Cell_Temp_Cell_Number = msg.data[2];
                Motor_Temperature = msg.data[3];
                Motor2_Generatrix_Current = msg.data[4];
                Motor2_Speed = msg.data[5];
                Motor2_Inverter_Temperature = msg.data[6];
                Motor2_Temperature = msg.data[7];
                break;

            case 0x18EF0AEF:// Hybrid_ENG_Control
                ENG_start_req = (unsigned char) msg.data[0]
                        +((unsigned int) msg.data[1] << 8);
                mot_work_time = (unsigned char) msg.data[2]
                        +((unsigned int) msg.data[3] << 8)
                        +((unsigned long) msg.data[4] << 16)
                        +((unsigned long) msg.data[5] << 24);
                mot_work_percent = msg.data[6];
                DCDC_ctrl = msg.data[7]&0x03;
                hcu_starter_forbid = (msg.data[7] >> 2)&0x03;
                aircompress_enable = (msg.data[7] >> 4)&0x03;
                Brake_Pedal = (msg.data[7] >> 6)&0x03;
                break;

            case 0x0CF00300://Electronic Engine Controller 2 (EEC2)
                Accelerator = msg.data[1];
                break;


            case 0x189111EF:// OTS_DI
                Zichong = (msg.data[0] >> 1)&0x01;
                Electric = (msg.data[0] >> 2)&0x01;
                Hybrid_forbid = (msg.data[0] >> 6)&0x01;
                Daisujingzhi = (msg.data[0] >> 7)&0x01;
                break;

            case 0x18FEEE00://水温
                water_temp = (unsigned char) msg.data[0];
                break;

            case 0x18FEEF00://水位和机油压力
                Engine_oil_press = (unsigned int) (msg.data[3]);
                water_level = (unsigned char) (msg.data[7]*0.4);
                break;

            case 0x18FEE400://发动机等待启动
                Wait_start = (unsigned char) (msg.data[3]&0x03);
                //water_level=(unsigned char)(msg.data[7]*0.4);  
                break;

            case 0x18FEF600: //周围环境条件 id==0x18FEF600
                barometric_pressure = msg.data[1]*2; //增压压力 0.5Kpa
                ambient_air_temperature = msg.data[2]; //周围空气温度
                air_pressure = msg.data[3]*2; //进气压力 0.5Kpa
                break;

            case 0x0CF00400:
                EngineSpeed = msg.data[3]+((unsigned int) (msg.data[4]) << 8);
                if (EngineSpeed >= 32000) EngineSpeed = 32000; //最大是4000RPM
                break;

            case 0x1CFEAF00://气体燃料消费
                Trip_fuel = (msg.data[0]
                        +((unsigned int) (msg.data[1]) << 8)
                        +((unsigned long) (msg.data[2]) << 16)
                        +((unsigned long) (msg.data[3]) << 24))*0.5; //单次累计气耗              
                break;

            case 0x1FDE800://气体燃料消费经济性
                Shunshi_fule = (msg.data[4]
                        +((unsigned int) (msg.data[5]) << 8))*0.01; //瞬时气耗
                break;

            case 0x1FDE900://气体燃料消费经济性
                Average_fule = (msg.data[4]
                        +((unsigned int) (msg.data[5]) << 8))*0.01; //平均气耗
                break;

            case 0x18F0010B:
                ABS_lamp = (msg.data[5] >> 4)&0x03; //ABS状态
                break;

            case 0x18FECA0B://ABS故障码
                abs_spn = msg.data[2]
                        +((unsigned int) (msg.data[3] << 8))
                        +((unsigned long) (msg.data[4] << 11));
                break;

            case 0x1801A79A://DC/DC变换器
                DC_DC_voltage_in = msg.data[0]
                        +((unsigned int) (msg.data[1] << 8));
                DC_DC_voltage_out = msg.data[2];
                DC_DC_current_out = msg.data[3]
                        +((unsigned int) (msg.data[4] << 8));
                DC_DC_t = msg.data[5];
                DC_DC_code.byte = msg.data[6];
                break;

            case 0x1801A79B://DC/AC变换器
                DC_AC_voltage_in = msg.data[0]
                        +((unsigned int) (msg.data[1] << 8));
                DC_AC_voltage_out = msg.data[2]
                        +((unsigned int) (msg.data[3] << 8));
                DC_AC_current_out = msg.data[4]
                        +((unsigned int) (msg.data[5] << 8));
                DC_AC_t = msg.data[6];
                DC_AC_code.byte = msg.data[7];
                break;
        }
    }
    C2INTF = 0; /* the individual flag register cleared */
    C2RX0CONbits.RXFUL = 0;
    IFS2bits.C2IF = 0;
}

void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void) {//10ms中断一次
    static unsigned int cnt0 = 0;
    static unsigned int cnt1 = 3;
    static unsigned int cnt2 = 7;
    static unsigned int cnt3 = 9;
    static unsigned int cnt4 = 0;
    static unsigned int cnt5 = 0;
    static unsigned int lpcnt2 = 0, lpcnt1 = 0, lpcount = 0;
    static unsigned int Fcount = 0, Kcount = 0;
    static unsigned int count6 = 0, count8 = 0;
    static float mile = 0;
    //报警延时3s标记

    if (alarm_update_flag1 || alarm_update_flag2) {
        if (count6 >= 300) {//
            count6 = 0;
            alarm_update_flag1 = 0;
            alarm_update_flag2 = 0;
        } else {
            count6++;
        }
    } else {
        count6 = 0;
    }

    if (alarm_update_flag) {
        if (count8 >= 300) {//
            count8 = 0;
            alarm_update_flag = 0;
        } else {
            count8++;
        }
    } else {
        count8 = 0;
    }

    if (delay_cnt > 0) delay_cnt--; //延时计数器 

    if (cnt0 >= 4) {//50ms
        cnt0 = 0;
        BCAN_send_mile();
        BCAN_SendCtl();
        if (M_ON_flag && (wake_up3 || wake_up2)) {//wake_up3置0时，M_ON不会立马置0，因为存在消抖延时
            LED_Logic();
            LED_Out();
            SYSTME_Logic();
        }
        F50ms = 1;
    } else cnt0++;

    if (cnt1 >= 9) {//100ms
        cnt1 = 0;
        F100ms = 1;
        PCAN_CCVS();
        //计算累计里程
        if (pSpeed > 0) {//车速大于0时才累计
            mile = mile + (float) (pSpeed / 3.6 * 0.1);
            if (mile >= 100.0) {
                mile = 0.0;
                save_flag = 1;
                if (single_miles >= 9999) single_miles = 0;
                else single_miles++;
                if (total_miles >= 9999999) total_miles = 0;
                else total_miles++;
            }
        }
        //CAN节点故障计时3s
        if (Fcan_count >= CAN_TIME)Fcan_count = CAN_TIME;
        else Fcan_count++;
        if (Rcan_count >= CAN_TIME)Rcan_count = CAN_TIME;
        else Rcan_count++;
        if (Mcan_count >= CAN_TIME)Mcan_count = CAN_TIME;
        else Mcan_count++;
        if (Ecan_count >= CAN_TIME)Ecan_count = CAN_TIME;
        else Ecan_count++;
        if (Vcan_count >= CAN_TIME)Vcan_count = CAN_TIME;
        else Vcan_count++;
        if (Bcan_count >= CAN_TIME)Bcan_count = CAN_TIME;
        else Bcan_count++;
        if (Dcan_count >= CAN_TIME)Dcan_count = CAN_TIME;
        else Dcan_count++;
        if (Acan_count >= CAN_TIME)Acan_count = CAN_TIME;
        else Acan_count++;

        if (cnt4 >= 9) {//1s
            cnt4 = 0;
            F1000ms = 1;
            FLASH_1s = !FLASH_1s;
            PCAN_send_mile();
        } else cnt4++;
    } else cnt1++;

    if (Fcount >= 39) {//0.4s
        Fcount = 0;
        F400ms = 1;
        FLASH = !FLASH;
    } else Fcount++;

    if (Kcount >= 29) {//300ms
        Kcount = 0;
        F300ms = 1;
        kFLASH = !kFLASH;
    } else Kcount++;

    if (cnt2 >= 49) {//500ms 
        cnt2 = 0;
        F500ms = 1;
        F500ms_LCD = !F500ms_LCD;
    } else cnt2++;

    if (wake_up3) { //ON开关判断//消抖//M_ON在掉电的一瞬间会使上拉电源无效，而M_ON在断电会保持300ms，从而影响开关量采集
        if (cnt3 > 30) M_ON = 1;
        else cnt3++;
    } else {
        if (cnt3 < 1) M_ON = 0;
        else cnt3--;
    }

    if (IN10 && IN6 && (IN7 || IN8) && IN9) { //后雾灯开关是常开开关，消抖
        if (cnt5 > 10) rLED_flag = 1;
        else cnt5++;
    } else if ((IN6 && (IN7 || IN8) && IN9) == 0) {
        rLED_flag = 0;
        cnt5 = 0;
    }

    //bit  key_set;  
    if (!KEY_SET && pkey_set && !bkey_set) {
        key_set = 1;
        bkey_set = pkey_set;
    } else {
        if (!KEY_SET == pkey_set) bkey_set = pkey_set;
    }
    pkey_set = KEY_SET;

    //bit  key_up;
    if (!KEY_UP && pkey_up && !bkey_up) {
        key_up = 1; //向上按键
        bkey_up = pkey_up;
    } else {
        if (!KEY_UP == pkey_up) bkey_up = pkey_up;
    }
    pkey_up = KEY_UP;

    //bit key_AV;
    if (!KEY_AV && pkey_AV && !bkey_AV) {
        key_AV = 1;
        bkey_AV = pkey_AV;
    } else {
        if (!KEY_AV == pkey_AV)bkey_AV = pkey_AV;
    }
    pkey_AV = KEY_AV;


    if (KEY_SET) {
        if (lpcnt2 >= 300) {//长按SET按钮
            //lpcnt2 = 0;
            bkey_set = 1; //长按时取消单按键操作
            key_long_set = 1;
        } else lpcnt2++;
    } else {
        lpcnt2 = 0;
    }

    if (KEY_AV && KEY_UP) {
        if (lpcnt1 >= 300) {//组合按钮，长按AV和UP按钮
            //lpcnt1 = 0;
            bkey_AV = 1; //长按时取消单按键操作
            bkey_up = 1; //长按时取消单按键操作
            key_duble = 1; //隐含功能按键
        } else lpcnt1++;
    } else {
        lpcnt1 = 0;
    }

    if (KEY_AV && (KEY_UP == 0)) { //长按 2S 清零单里程 长按AV
        if (lpcount >= 200) {
            single_miles = 0;
            lpcount = 0;
            save_flag = 1;
            bkey_AV = 1; //长按时取消单按键操作
            lpcount = 100;
        } else lpcount++;
    } else if (lpcount > 0) lpcount--;

    _T1IF = 0;
}
/*
void TIMER4_Init(void) {
    TMR4=0;
    T4CON = 0x0010; // 1：8分频 Fcy/8 16MHZ/8=2MHz
    PR4 = 19999;  //10ms中断一次 
    _T4IF = 0;
    _T4IE = 1;
    T4CONbits.TON=1;
}

void __attribute__((__interrupt__, auto_psv)) _T2Interrupt(void){
    static unsigned int cnt = 0;
    if (cnt >= 5) {//50ms
        cnt = 0;
        fFreq++;
    }else cnt++;
    
    
    
    
    _T4IF = 0;  
}*/


