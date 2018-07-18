#include "APP_System.h"

unsigned char delay_cnt;

DATA_BIT gKEY[5];

unsigned int single_miles = 0;
unsigned long total_miles = 0;

unsigned int SPEED; //外部CAN换算车速
unsigned int RPM;   //外部CAN换算转速

unsigned char dm_hcu_display[32]; //故障类型
unsigned char dm_tcu_display[32];

//系统初始化函数
void System_Init(void) {
    ADPCFG = 0xffff; //关闭AD口     
    LED_Clear(); //消除开机瞬间LED和蜂鸣器闪烁
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
    LED_Clear();
    LED_Out();
    POUT_Clear();               
    CAN1_Init();
    CAN2_Init();
    while (1) {
        ClrWdt();
        LCD_BackLight(0x00);
        
        if (F50ms) {
            F50ms = 0;           
            SW_Input();
        }
        
        if (WAKE_Danger) {//如果危险报警唤醒
            PWR_CTRL_DR = 0;
            PWR_CTRL = 1; 
            LED_Danger();
        } else {
            POUT_Clear();
            LED_Clear();
        }               
              
        LED_Out();

        if (WAKE_ON || WAKE_ACC || WAKE_Charge || WAKE_Start) break; // 如果是ON唤醒则正常退出
        
        if (!WAKE_Danger) {
            BCAN_SendCtl(); //中断里的BCAN发送50ms发送一次，可能来不及发最后的状态，程序已经结束
            __delay_us(500);
            BCAN_SendCtl();
            __delay_us(500);
            BCAN_SendCtl();
            __delay_us(500);
            BCAN_SendCtl();
            __delay_us(500);
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
    TIM5_Init(); //频率输出初始化
    EEROM_ReadSingleMiles();
    EEROM_ReadTotalMiles();
    EEROM_ReadHMPulse();
    EEROM_ReadBackLight();
    single_miles = e_single_miles;
    total_miles = e_total_miles;
    OC1RS = e_backlight; //更新背光值
    ADC_Init();
    LCD_Init();
    LED_Enable(); //蜂鸣器开启
    LED_Out();
    while (delay_cnt > 1); //延时1s
    OC1RS = 0;
    PWR_CTRL_DR = 0; //电源控制接口
    PWR_CTRL = 1; //开启MCU电源控制
    LED_Clear();
    LED_Out();
    SW_Input();
    LCD_BackLight(7);
    DS3231_ReadTime(&timer); //读取时间 
    Count_ADR();//AD采集
}

//电机转动逻辑
void SM_Logic(void) {
    static unsigned char task = 0;
    static unsigned int  temp = 0;
    
    if (CAN_0x239.MCU_MtrSpd > 12000)
        RPM = CAN_0x239.MCU_MtrSpd - 12000;
    else RPM = 12000 - CAN_0x239.MCU_MtrSpd;
    //RPM = MCU_RPM;
    
    //SPEED = (unsigned int) (RPM * 0.446 * 2 * 60 * 3.14159 / 1000 / 5.571  * e_hm_pulse / 100); //单位1Km/h
    SPEED = CAN_0x141.VCU_VehSpd * 0.05625 * 1.03;
    
    sm_new_value[0] = (unsigned int) (SPEED * 132 / 7);
    if (sm_new_value[0] > 2640)
        sm_new_value[0] = 2640;
    SM_Update_0(); //

    sm_new_value[5] = (unsigned int) (RPM * 0.33);
    if (sm_new_value[5] > 2640)
        sm_new_value[5] = 2640;
    SM_Update_5(); //2640

    if (task == 0) {
        task = 1;
        temp = 0;
        if (temp < 500)
            sm_new_value[4] = 0;
        else if (temp > 3000)
            sm_new_value[4] = 1152;
        else sm_new_value[4] = (unsigned int) ((temp - 500) * 0.4608);
        SM_Update_4();  //气压1
    } else if (task == 1) {
        task = 2;
        temp = CAN_0x213.BMS_HVBatSOC;
        if (temp <= 0) {
            sm_new_value[3] = 0;
        } else if (temp > 100) {
            sm_new_value[3] = 1152;
        } else {
            sm_new_value[3] = (unsigned int) (11.52 * temp);
        }
        SM_Update_3(); //SOC
    } else if (task == 2) {
        task = 3;
        temp = 0;
        if (temp < 500)
            sm_new_value[2] = 0;
        else if (temp > 3000)
            sm_new_value[2] = 1152;
        else sm_new_value[2] = (unsigned int) ((temp - 500) * 0.4608);
        SM_Update_2();  //气压2
    } else if (task == 3) {
        task = 0;
        temp = ADV[0];
        if (temp < 160) {
            sm_new_value[1] = 0;
        } else if ((temp >= 160)&&(temp < 320)) {
            sm_new_value[1] = (unsigned int) (7.2 * (temp - 160));
        } else {
            sm_new_value[1] = 1152;
        }
        /*if (temp < 180) { 
            sm_new_value[1] = 0;
        } else if ((temp >= 180)&&(temp < 240)) {
            sm_new_value[1] = (unsigned int) (6.667 * (temp - 180));
        } else if ((temp >= 240)&&(temp <= 320)) {
            sm_new_value[1] = (unsigned int) (10 * (temp - 240)) + 400;
        } else {
            sm_new_value[1] = 1200;
        }*/
        SM_Update_1();//蓄电池电压
    } else {
        task = 0;
    }
}

//电机全部归位状态 函数为0时表示电机全部归位
int SM_Zero_State(void) {
    unsigned char task = 6;
    
    if (sm_old_value[0] == 0) task--;
    if (sm_old_value[1] == 0) task--;
    if (sm_old_value[2] == 0) task--;
    if (sm_old_value[3] == 0) task--;
    if (sm_old_value[4] == 0) task--;
    if (sm_old_value[5] == 0) task--;
    
    return task;
}

//定时启动条件
char Count_Time(unsigned char condition, //启动条件
        unsigned int  out_time,           //条件满足，工作时间
        CYCLE_EN cycle_en,                //是否循环执行
        unsigned int  stop_time           //循环执行时，间歇时间
        ) { 
    static unsigned int temp = 0;
    if (condition) {
        temp = temp > 60000 ? 60000 : temp + 1; //计数上限，防止溢出
    } else {
        temp = 0;
    }

    if (temp > 0 && temp < out_time * 100) {
        return 1;
    } else if (!cycle_en) {
        return 0;
    } else if (temp < (out_time + stop_time) * 100) {
        return 0;
    } else {
        temp = 0;
        return 0;     
    }
}

/**************************************************************************************
    * FunctionName   : KeyProcess()
    * Description    : 按钮处理
    * EntryParameter : None
    * ReturnValue    : None
    **************************************************************************************/
void KeyProcess(void){
    static unsigned int lpcnt2 = 0, lpcnt1 = 0, lpcount = 0;
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
            lpcnt2 = 0;
            bkey_set = 1; //长按时取消单按键操作
            key_long_set = 1;
        } else lpcnt2++;
    } else {
        lpcnt2 = 0;
    }

    if (KEY_AV && KEY_UP) {
        if (lpcnt1 >= 300) {//组合按钮，长按AV和UP按钮
            lpcnt1 = 0;
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
            save_flag = 1;
            bkey_AV = 1; //长按时取消单按键操作
            lpcount = 100;
        } else lpcount++;
    } else if (lpcount > 0) lpcount--;
}
/**************************************************************************************
 * FunctionName   : TaskRemarks()
 * Description    : 任务标志处理
 * EntryParameter : None
 * ReturnValue    : None
 **************************************************************************************/
void TaskRemarks(void) {
    unsigned char i;
    for (i = 0; TaskComps[i].TaskHook != NULL; i++) // 逐个任务时间处理
    {
        if (TaskComps[i].Timer) // 时间不为0
        {
            TaskComps[i].Timer--; // 减去一个节拍
        }
        if (TaskComps[i].Timer == 0) // 时间减完了
        {
            if (TaskComps[i].Run == 0) {//前次任务执行完成后再计时，保证间隔准确
                TaskComps[i].Timer = TaskComps[i].ItvTime; // 恢复计时器值，从新下一次
                TaskComps[i].Run = 1; // 任务可以运行
            }
        }
    }
}

/**************************************************************************************
 * FunctionName   : TaskProcess()
 * Description    : 任务处理
 * EntryParameter : None
 * ReturnValue    : None
 **************************************************************************************/
void TaskProcess(void) {
    unsigned char i = 0;
    static unsigned char Flag = 0, poll_time = 0;
    //任务处理,两个任务间隔取决于poll_time * 运行时间间隔
    for (i = 0; TaskComps[i].TaskHook != NULL; i++) { // 逐个任务时间处理
        if (TaskComps[i].Run && Flag) // 时间不为0
        {
            TaskComps[i].TaskHook(); // 运行任务
            TaskComps[i].Run = 0; // 标志清0 
            Flag = 0;
        }
    }

    if (poll_time >= 4) {
        poll_time = 0;
        Flag = 1;
    } else if (!Flag) {
        poll_time++;
    }

    //任务处理，每次循环只执行一个任务
    /*if (TaskComps[i].TaskHook != NULL) // 逐个任务时间处理
    {
        if (TaskComps[i].Run) // 时间不为0
        {
            TaskComps[i].TaskHook(); // 运行任务
            TaskComps[i].Run = 0; // 标志清0 
        }
        i++;
    }
    if(TaskComps[i].TaskHook == NULL){
        i = 0;
    }*/
}