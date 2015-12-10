/**
    DxSensor

    Sensor common interface

    @file       DxSensor.c
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/


#include "Sensor.h"
#include "DxSensor.h"
#include "sif.h"
#include "pll.h"
#include "iocfg.h"
#include "Delay.h"
#include "SensorDrv.h"
#include "LVDS.h"
#define __MODULE__          DxDrv
#define __DBGLVL__          1 // 0=OFF, 1=ERROR, 2=TRACE
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "DebugModule.h"

#if  (_SENSORLIB_ == _SENSORLIB_CMOS_IMX083CQK_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_IMX083CQK();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_MN34110_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_MN34110();
}
#elif (_SENSORLIB_ == _SENSORLIB_DUMMY_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_Dummy();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_AR0330P_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_AR0330();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_AR0330M_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_AR0330M();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_MI5100_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_MI5100();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_OV2710_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_OV2710();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX322LQJ_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_IMX322LQJ();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_IMX206CQC();
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_OV2710M_)
SENSOR_DRVTAB* DrvSensor_GetTab1st(void)
{
    return Sensor_GetDrvTab_OV2710M();
}
#else
#endif

#if  (_SENSORLIB_ == _SENSORLIB_CMOS_IMX083CQK_)
/*
Sensor pin      LVDS pin
0   A      ->   0
1   B      ->   1
2   C      ->   2
3   D      ->   3
4   E      ->   4
5   F      ->   5
6   G      ->   6
7   H      ->   7
8   I      ->   8
9   J      ->   9
*/

UINT32 Sen2Lvds_tab[LVDS_DATLANE_ID_MAX] =
{0, 1, 2, 3, 4, 5, 6, 7, 8 ,9};

SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_SIF;
    InitObj.CmdInfo.INFO.SIF.busclk = 24000000;
    InitObj.CmdInfo.INFO.SIF.chanel = SIF_CH0;
    InitObj.CmdInfo.INFO.SIF.sen_d_s = 1;
    InitObj.CmdInfo.INFO.SIF.sen_h = 1;
    InitObj.CmdInfo.INFO.SIF.DMA_en = DISABLE;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_480;
    InitObj.PclkInfo.uiMClkFreq = 72000000;

    InitObj.Sen2LVDSPinMap = Sen2Lvds_tab;
    return InitObj;
}


#elif (_SENSORLIB_ == _SENSORLIB_CMOS_MN34110_)
/*
Sensor pin                   LVDS pin
0   ch 1 port 0  SDOD0 ->    0
1   ch 1 port 1  SDOD1 ->    1
2   ch 1 port 2  SDOD3 ->    2
3   ch 1 port 3  SDOD4 ->    3
4   ch 2 port 3  SDOD6 ->    4
5   ch 2 port 2  SDOD7 ->    5
6   ch 2 port 1  SDOD9 ->    6
7   ch 2 port 0  SDODA ->    7
*/

UINT32 Sen2Lvds_tab[LVDS_DATLANE_ID_MAX] =
{ 0, 1, 2, 3, 4, 5, 6, 7, SEN_IGNORE, SEN_IGNORE};

SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_SIF;
    InitObj.CmdInfo.INFO.SIF.busclk = 24000000;
    InitObj.CmdInfo.INFO.SIF.chanel = SIF_CH0;
    InitObj.CmdInfo.INFO.SIF.sen_d_s = 1;
    InitObj.CmdInfo.INFO.SIF.sen_h = 1;
    InitObj.CmdInfo.INFO.SIF.DMA_en = DISABLE;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    //InitObj.PclkInfo.uiMClkSrc = PLL_CLKSEL_SIE_MCLKSRC_480;
    InitObj.PclkInfo.uiMClkFreq = 54000000;

    InitObj.Sen2LVDSPinMap = Sen2Lvds_tab;


    //power-
    gpio_setPin(GPIO_SENSOR_PWM1);  //SN1.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM0);  //SN1.2
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM2);  //SN3.3
    Delay_DelayMs(10);

    return InitObj;
}


#elif (_SENSORLIB_ == _SENSORLIB_CMOS_MI5130_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x21;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x20;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 24000000;
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_MI5100_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};

    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x91;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x90;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 6000000;
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_AR0330P_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x21;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x20;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 24000000;

    //power-
    gpio_setPin(GPIO_SENSOR_PWM1);  //SN2.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM0);  //SN1.8
    Delay_DelayMs(10);

    //reset
    gpio_clearPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    gpio_setPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_OV2710_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x6d;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x6c;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 24000000;

    //power-
    gpio_setPin(GPIO_SENSOR_PWM1);  //SN2.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM0);  //SN1.8
    Delay_DelayMs(10);

    //reset
    gpio_clearPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    gpio_setPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_AR0330M_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x21;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x20;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 24000000;

 //   InitObj.Option=SENSOR_OPTION_NONE;

//power-
    gpio_setPin(GPIO_SENSOR_PWM2);  //SN2.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM1);  //SN1.8
 //   gpio_setPin(GPIO_SENSOR_PWM0);  //SN1.8
    Delay_DelayMs(10);

    //reset
    gpio_clearPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    gpio_setPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    //gpio_clearPin(GPIO_SENSOR2_PDN);
    //Delay_DelayMs(1);
    //gpio_setPin(GPIO_SENSOR2_PDN);	
    //Delay_DelayMs(1);
	
    //gpio_clearPin(GPIO_SENSOR2_RESET);
    //Delay_DelayMs(100);
    //gpio_setPin(GPIO_SENSOR2_RESET);
    //Delay_DelayMs(10);	
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_OV2710M_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x6d;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x6c;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 24000000;
    debug_msg("OV2710M DrvSensor_GetObj1st..\r\n");	
    gpio_clearPin(GPIO_SENSOR_PDN);  //SN2.8
    Delay_DelayMs(20);
    //reset
    
    gpio_clearPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);
    gpio_setPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(100);

    //gpio_clearPin(GPIO_SENSOR2_PDN);
    //Delay_DelayMs(1);
    //gpio_setPin(GPIO_SENSOR2_PDN);	
    //Delay_DelayMs(1);
	
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX322LQJ_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = ENABLE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0x35;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0x34;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 37125000;
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)

/*
Sensor pin    LVDS pin
0   DO0 ->    0
1   DO1 ->    1
2   DO2 ->    2
3   DO3 ->    3
*/

UINT32 Sen2Lvds_tab[LVDS_DATLANE_ID_MAX] =
{ 0, 1, 2, 3, SEN_IGNORE, SEN_IGNORE, SEN_IGNORE, SEN_IGNORE, SEN_IGNORE, SEN_IGNORE};

SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_SIF;
    InitObj.CmdInfo.INFO.SIF.busclk = 24000000;
    InitObj.CmdInfo.INFO.SIF.chanel = SIF_CH0;
    InitObj.CmdInfo.INFO.SIF.sen_d_s = 1;
    InitObj.CmdInfo.INFO.SIF.sen_h = 1;
    InitObj.CmdInfo.INFO.SIF.DMA_en = DISABLE;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 72000000;

    InitObj.Sen2LVDSPinMap = Sen2Lvds_tab;
    return InitObj;
}
#elif (_SENSORLIB_ == _SENSORLIB_DUMMY_)
SENSOR_INIT_OBJ DrvSensor_GetObj1st(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_SIF;
    InitObj.CmdInfo.INFO.SIF.busclk = 4800000;
    InitObj.CmdInfo.INFO.SIF.chanel = SIF_CH0;
    InitObj.CmdInfo.INFO.SIF.sen_d_s = 1;
    InitObj.CmdInfo.INFO.SIF.sen_h = 1;
    InitObj.CmdInfo.INFO.SIF.DMA_en = DISABLE;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 13500000;
    return InitObj;
}
#else
#endif

#if (_SENSORLIB2_ == _SENSORLIB2_CMOS_TVP5150_)
SENSOR_DRVTAB* DrvSensor_GetTab2nd(void)
{
    debug_msg("Z900 -- >> Sensor_GetDrvTab_TVP5150 \r\n");
    return Sensor_GetDrvTab_TVP5150();
}

SENSOR_INIT_OBJ DrvSensor_GetObj2nd(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    InitObj.CmdInfo.CmdType = SENSOR_CMD_I2C;
    InitObj.CmdInfo.INFO.I2C.I2c = SENSOR_I2C1_2ND;
    InitObj.CmdInfo.INFO.I2C.extSlaveAddrEnable = FALSE;
    InitObj.CmdInfo.INFO.I2C.extSlaveRAddr = 0xB9;
    InitObj.CmdInfo.INFO.I2C.extSlaveWAddr = 0xB8;

    InitObj.PclkInfo.bSieMClkEn = ENABLE;
    InitObj.PclkInfo.uiMClkSrc = PLL_CLKSRC_PLL5;
    InitObj.PclkInfo.uiMClkFreq = 27000000;

	
 //   gpio_clearPin(GPIO_SENSOR2_RESET);
 //   Delay_DelayMs(100);
 //   gpio_setPin(GPIO_SENSOR2_RESET);
 //   Delay_DelayMs(100);
	
    return InitObj;
}
#else
SENSOR_DRVTAB* DrvSensor_GetTab2nd(void)
{
    return NULL;
}

SENSOR_INIT_OBJ DrvSensor_GetObj2nd(void)
{
    SENSOR_INIT_OBJ InitObj = {0};
    return InitObj;
}
#endif

#if (_SENSORLIB_ == _SENSORLIB_CMOS_MN34110_)
void DrvSensor_TurnOnPower(void)
{
    //UINT32 bl;
    Delay_DelayMs(1);
    gpio_setDir(GPIO_SENSOR_STANDBY, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_STANDBY);

    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_RESET);
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_RESET);
}
/*
void DrvSensor_TurnOffPower(void)
{
    //UINT32 bl;
    gpio_clearPin(GPIO_SENSOR_RESET);
    gpio_clearPin(GPIO_SENSOR_STANDBY);
}

void DrvSensor_PowerSaveOff(void)
{
    //pll_enableClock(SIE_MCLK);
    gpio_setDir(GPIO_SENSOR_STANDBY, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_STANDBY);

    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_RESET);
}


void DrvSensor_PowerSaveOn(void)
{
    gpio_clearPin(GPIO_SENSOR_RESET);
    gpio_clearPin(GPIO_SENSOR_STANDBY);
}
*/
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX322LQJ_)
void DrvSensor_TurnOnPower(void)
{
    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_RESET);

    pll_setPLLEn(PLL_ID_5, DISABLE);
    //pll_setPLL(PLL_ID_5, 0x80000);//37.125 MHz
    pll_setPLL(PLL_ID_5, 0x63000);//37.125 MHz
    pll_setPLLEn(PLL_ID_5, ENABLE);
    pll_setClkEn(PLL_CLK_SIEMCLK, FALSE);
    pll_selectClkSrc(PLL_CLK_SIEMCLK, PLL_CLKSRC_PLL5);
    pll_setClkFreq(PLL_CLK_SIEMCLK, 37125000);
    //pll_setClkFreq(PLL_CLK_SIEMCLK, 24000000);
    pll_setClkEn(PLL_CLK_SIEMCLK, TRUE);

    Delay_DelayMs(100);
    gpio_setPin(GPIO_SENSOR_RESET);
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)
void DrvSensor_TurnOnPower(void)
{
    //power-
    gpio_clearPin(GPIO_SENSOR_RESET);  //reset
    Delay_DelayMs(10);


    gpio_setPin(GPIO_SENSOR_PWM0);  //SN1.2
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM1);  //SN1.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_PWM2);  //SN2.8
    Delay_DelayMs(10);
    gpio_setPin(GPIO_SENSOR_RESET);  //reset
    Delay_DelayMs(10);
}
#elif(_SENSORLIB2_ == _SENSORLIB2_CMOS_TVP5150_)
void DrvSensor_TurnOnPower(void)
{
    
 //   gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
 //   gpio_setPin(GPIO_SENSOR_RESET);

 //   gpio_setDir(GPIO_SENSOR2_RESET, GPIO_DIR_OUTPUT);
 //   gpio_setPin(GPIO_SENSOR2_RESET);	
    gpio_setPin(GPIO_SENSOR2_POWER_EN);
    Delay_DelayMs(1);
    gpio_clearPin(GPIO_SENSOR2_PDN);
    Delay_DelayMs(1);
    gpio_setPin(GPIO_SENSOR2_PDN);
    Delay_DelayMs(1);
    gpio_clearPin(GPIO_SENSOR2_RESET);
    Delay_DelayMs(1);
    gpio_setPin(GPIO_SENSOR2_RESET);

    DBG_ERR("------------PDN=%d\r\n",gpio_getPin(GPIO_SENSOR2_PDN));
    //gpio_setDir(GPIO_SENSOR2_PDN, GPIO_DIR_OUTPUT);
    //gpio_clearPin(GPIO_SENSOR2_PDN);
	
   // debug_msg("------------PDN=%d\r\n",gpio_getPin(GPIO_SENSOR2_PDN));
}
#else
void DrvSensor_TurnOnPower(void)
{
    gpio_setDir(GPIO_SENSOR_STANDBY, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_STANDBY);

    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_RESET);
}
#endif

#if (_SENSORLIB_ == _SENSORLIB_CMOS_IMX208PQH_)
void DrvSensor_TurnOffPower(void)
{
    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_RESET);

    gpio_setDir(GPIO_SENSOR_PW1, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_PW1);
    gpio_setDir(GPIO_SENSOR_PW2, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_PW2);
    gpio_setDir(GPIO_SENSOR_PW3, GPIO_DIR_OUTPUT);
    gpio_clearPin(GPIO_SENSOR_PW3);
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)
void DrvSensor_TurnOffPower(void)
{
    //power-
    gpio_clearPin(GPIO_SENSOR_RESET);  //reset
    Delay_DelayMs(10);
    gpio_clearPin(GPIO_SENSOR_PWM2);  //SN2.8
    Delay_DelayMs(10);
    gpio_clearPin(GPIO_SENSOR_PWM1);  //SN1.8
    Delay_DelayMs(10);
    gpio_clearPin(GPIO_SENSOR_PWM0);  //SN1.2
    Delay_DelayMs(10);
}
#else
void DrvSensor_TurnOffPower(void)
{
    DBG_IND("[PWRIC] - sensor close\r\n");
    gpio_clearPin(GPIO_SENSOR_RESET);
  //  gpio_clearPin(GPIO_SENSOR_STANDBY);
}
#endif

#if (_SENSORLIB_ == _SENSORLIB_CMOS_IMX208PQH_)
void DrvSensor_PowerSaveOff(void)
{
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)
void DrvSensor_PowerSaveOff(void)
{
}
#else
void DrvSensor_PowerSaveOff(void)
{
    //pll_enableClock(SIE_MCLK);
//    gpio_setDir(GPIO_SENSOR_STANDBY, GPIO_DIR_OUTPUT);
//    gpio_setPin(GPIO_SENSOR_STANDBY);

    gpio_setDir(GPIO_SENSOR_RESET, GPIO_DIR_OUTPUT);
    gpio_setPin(GPIO_SENSOR_RESET);
}
#endif

#if (_SENSORLIB_ == _SENSORLIB_CMOS_IMX208PQH_)
void DrvSensor_PowerSaveOn(void)
{
}
#elif (_SENSORLIB_ == _SENSORLIB_CMOS_IMX206CQC_)
void DrvSensor_PowerSaveOn(void)
{
}
#else
void DrvSensor_PowerSaveOn(void)
{
    gpio_clearPin(GPIO_SENSOR_RESET);
    gpio_clearPin(GPIO_SENSOR_STANDBY);
}
#endif

#if(_SENSORLIB2_ == _SENSORLIB2_CMOS_TVP5150_)
BOOL DrvSensor_Det2ndSensor(void)
{
    return TRUE;//return gpio_getPin(GPIO_SENSOR2_DET)?FALSE:TRUE;
}
#else
BOOL DrvSensor_Det2ndSensor(void)
{
    return FALSE;
}
#endif
