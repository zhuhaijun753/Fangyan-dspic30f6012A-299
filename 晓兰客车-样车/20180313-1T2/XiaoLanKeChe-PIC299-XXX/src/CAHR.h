#ifndef APP_CHAR_H
#define	APP_CHAR_H

#include "FONT.h"

/*******************************主界面显示*************************************/

//单体电压
__prog__ const unsigned char * DTDY[] = {BMP_Dan, BMP_Ti, BMP_DY, 0};
//单体温度
__prog__ const unsigned char * DTWD[] = {BMP_Dan, BMP_Ti, BMP_Wendu, 0};

/******************************************************************************/

/********************************菜单选项**************************************/
//仪表系统信息
__prog__ const unsigned char * IC_STSTEM[] = {BMP_IC, BMP_STSTEM, BMP_Message, 0};
//电池信息
__prog__ const unsigned char * Battery_STSTEM[] = {BMP_Battery, BMP_Message, 0};
//电机系统信息
__prog__ const unsigned char * MOTO_STSTEM[] = {BMP_MOTO, BMP_STSTEM, BMP_Message, 0};
//气泵系统信息
__prog__ const unsigned char * AIR_STSTEM[] = {BMP_DMQ, BMP_STSTEM, BMP_Message, 0};
//油泵系统信息
__prog__ const unsigned char * OIL_STSTEM[] = {BMP_DMY, BMP_STSTEM, BMP_Message, 0};
//电池管理系统信息
__prog__ const unsigned char * BMS_STSTEM[] = {BMP_Battery, BMP_Manage, BMP_STSTEM, BMP_Message, 0};
//空调系统信息
//__prog__ const unsigned char * OIL_STSTEM[] = {BMP_Air, BMP_STSTEM, BMP_Message, 0};

/******************************************************************************/

/******************************CAN报警*****************************************/
//电池电压低
__prog__ const unsigned char * DCDYD[] = {BMP_DC, BMP_DY, BMP_Di, 0};
//SOC
__prog__ const unsigned char * SOC[] = {BMP_s, BMP_o, BMP_c, 0};
//CAN
__prog__ const unsigned char * CHAR_CAN[] = {BMP_c, BMP_a, BMP_n, 0};
//充电通讯异常
__prog__ const unsigned char * CDTXYC[] = {BMP_Chongdian, BMP_Tongxun, BMP_Yichang, 0};
//前部模块离线
__prog__ const unsigned char * QBMKLX[] = {BMP_Qianbu, BMP_Mokuai, BMP_Lixian, 0};
//顶部模块离线
__prog__ const unsigned char * DBMKLX[] = {BMP_Dingbu, BMP_Mokuai, BMP_Lixian, 0};
//后部模块离线
__prog__ const unsigned char * HBMKLX[] = {BMP_Houbu, BMP_Mokuai, BMP_Lixian, 0};
//电机系统故障
__prog__ const unsigned char * DJXTGZ[] = {BMP_DJ, BMP_System, BMP_Guzhang, 0};
//电池故障
__prog__ const unsigned char * DCGZ[] = {BMP_DC, BMP_Guzhang, 0};
//蓄电池电压低
__prog__ const unsigned char * XDCDYD[] = {BMP_Xu, BMP_DC, BMP_DY, BMP_Di, 0};
//正在充电禁止行车
__prog__ const unsigned char * CHAGRE[] = {BMP_Charging,BMP_Che, 0};
//绝缘故障
__prog__ const unsigned char * JYGZ[] = {BMP_Jueyuan, BMP_Guzhang, 0};
//电池高温报警
__prog__ const unsigned char * DCWDGBJ[] = {BMP_DC, BMP_Wendu, BMP_Gao, BMP_Baojing, 0};
//电池过压报警
__prog__ const unsigned char * DCGYBJ[] = {BMP_DC, BMP_Guo, BMP_Ya, BMP_Baojing, 0};
//电池欠压报警
__prog__ const unsigned char * DCQYBJ[] = {BMP_DC, BMP_Qian, BMP_Ya, BMP_Baojing, 0};
//SOC 低报警
__prog__ const unsigned char * SOCDBJ[] = {BMP_s,BMP_o,BMP_c, BMP_Di, BMP_Baojing, 0};
//单体电池过压报警
__prog__ const unsigned char * DTDCGYBJ[] = {BMP_Dan,BMP_Ti,BMP_DC, BMP_Guo, BMP_Ya, BMP_Baojing, 0};
//单体电池欠压报警
__prog__ const unsigned char * DTDCQYBJ[] = {BMP_Dan,BMP_Ti,BMP_DC, BMP_Qian, BMP_Ya, BMP_Baojing, 0};
//SOC 过高报警
__prog__ const unsigned char * SOCGGBJ[] = {BMP_s,BMP_o,BMP_c, BMP_Guo,BMP_Gao, BMP_Baojing, 0};
//SOC 跳变报警
__prog__ const unsigned char * SOCTBBJ[] = {BMP_s,BMP_o,BMP_c, BMP_Tiaobian, BMP_Baojing, 0};
//气泵系统故障
__prog__ const unsigned char * QBXTGZ[] = {BMP_Qibeng, BMP_System,BMP_Guzhang, 0};
//油泵系统故障
__prog__ const unsigned char * YBXTGZ[] = {BMP_Youbeng, BMP_System,BMP_Guzhang, 0};
/******************************************************************************/

/*******************************开关量采集*************************************/

//前雾灯开关
__prog__ const unsigned char * QWDKG[] = {BMP_IC_Qian, BMP_IC_Wu, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//后雾灯开关
__prog__ const unsigned char * HWDKG[] = {BMP_IC_Hou, BMP_IC_Wu, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//小灯开关
__prog__ const unsigned char * XDKG[] = {BMP_Xiao, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//前门开状态
__prog__ const unsigned char * QMKZT[] = {BMP_IC_Qian, BMP_Men, BMP_Kai, BMP_Zhuangtai, 0};
//中门开状态
__prog__ const unsigned char * ZMKZT[] = {BMP_Zhong, BMP_Men, BMP_Kai, BMP_Zhuangtai, 0};
//前门应急阀报警
//远光灯开关
__prog__ const unsigned char * YGDKG[] = {BMP_IC_Yuan, BMP_IC_Guang, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//近光灯开关
__prog__ const unsigned char * JGDKG[] = {BMP_IC_Jin, BMP_IC_Guang, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//ASR
__prog__ const unsigned char * ASR[] = {BMP_a, BMP_s, BMP_r, 0};
//ABS
__prog__ const unsigned char * ABS[] = {BMP_a, BMP_b, BMP_s, 0};
//危险报警信号
__prog__ const unsigned char * WXBJKG[] = {BMP_Weixian, BMP_Baojing, BMP_IC_Kaiguan, 0};

/******************************************************************************/

/******************************模块输出状态************************************/

//左前雾灯输出
__prog__ const unsigned char * ZQWDSC[] = {BMP_IC_Zuo, BMP_IC_Qian, BMP_IC_Wu, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//右前雾灯输出
__prog__ const unsigned char * YQWDSC[] = {BMP_IC_You, BMP_IC_Qian, BMP_IC_Wu, BMP_IC_Deng, BMP_IC_Kaiguan, 0};
//前门开输出
__prog__ const unsigned char * QMKSC[] = {BMP_IC_Qian, BMP_Men, BMP_Kai, BMP_Output, 0};
//中门开输出
__prog__ const unsigned char * ZMKSC[] = {BMP_Zhong, BMP_Men, BMP_Kai, BMP_Output, 0};
//左远光灯输出
__prog__ const unsigned char * ZYGDSC[] = {BMP_IC_Zuo, BMP_IC_Yuan, BMP_IC_Guang, BMP_IC_Deng, 0, 0};
//右远光灯输出
__prog__ const unsigned char * YYGDSC[] = {BMP_IC_You, BMP_IC_Yuan, BMP_IC_Guang, BMP_IC_Deng, 0, 0};
//左近光灯输出
__prog__ const unsigned char * ZJGDSC[] = {BMP_IC_Zuo, BMP_IC_Jin, BMP_IC_Guang, BMP_IC_Deng, 0, 0};
//右近光灯输出
__prog__ const unsigned char * YJGDSC[] = {BMP_IC_You, BMP_IC_Jin, BMP_IC_Guang, BMP_IC_Deng, 0, 0};
//后雾灯输出
__prog__ const unsigned char * HWDSC[] = {BMP_IC_Hou, BMP_IC_Wu, BMP_IC_Deng, 0};

/******************************************************************************/

/******************************************************************************/
#endif	/* APP_CHAR_H */