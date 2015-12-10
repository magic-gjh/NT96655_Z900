
#ifndef __Gsensor_H
#define __Gsensor_H

#include "i2c.h"

typedef enum
{
    GSENSOR_I2C_REGISTER_1BYTE = 1,
    GSENSOR_I2C_REGISTER_2BYTE = 2,
    ENUM_DUMMY4WORD(GSENSOR_I2C_REGISTER_BYTES)
} GSENSOR_I2C_REGISTER_BYTES, *PGSENSOR_I2C_REGISTER_BYTES;


typedef struct
{
    I2C_SECTION I2C_Channel;
    GSENSOR_I2C_REGISTER_BYTES  I2C_RegBytes;
    I2C_PINMUX      I2C_PinMux;
    I2C_BUS_CLOCK   I2C_BusClock;
    UINT32          I2C_Slave_WAddr;
    UINT32          I2C_Slave_RAddr;
} GSENSOR_INFO,*PGSENSOR_INFO;

// for axis data
typedef enum
{
    GSENSOR_AXIS_X,
    GSENSOR_AXIS_Y,
    GSENSOR_AXIS_Z,
    ENUM_DUMMY4WORD(GSENSOR_AXIS)
} GSENSOR_AXIS;

// for get status
typedef enum
{
    GSENSOR_ORIENT_0,
    GSENSOR_ORIENT_90,
    GSENSOR_ORIENT_180,
    GSENSOR_ORIENT_270,
    ENUM_DUMMY4WORD(GSENSOR_ORIENT)
} GSENSOR_ORIENT;

typedef enum
{
    GSENSOR_SHAKE_LEFT,
    GSENSOR_SHAKE_RIGHT,
    ENUM_DUMMY4WORD(GSENSOR_SHAKE)
} GSENSOR_SHAKE;

typedef enum
{
    GSENSOR_SENSITIVITY_OFF,
    GSENSOR_SENSITIVITY_LOW,
    GSENSOR_SENSITIVITY_MED,
    GSENSOR_SENSITIVITY_HIGH,
    ENUM_DUMMY4WORD(GSENSOR_SENSITIVITY)
}GSENSOR_SENSITIVITY;

typedef struct
{
   UINT32      Xacc;
   UINT32      Yacc;
   UINT32      Zacc;
} AXIS_DATA;

typedef struct {
    AXIS_DATA   Axis;
    GSENSOR_ORIENT      Ori;
    GSENSOR_SHAKE       SH;
    UINT32       Dev;
} Gsensor_Data,*pGsensor_Data;

extern BOOL   GSensor_open(void);
extern BOOL   GSensor_close(void);
extern BOOL   GSensor_Init(void);
extern BOOL   GSensor_GetData(Gsensor_Data *GS_Data);
extern void   GSensor_GetStatus(UINT32 status,UINT32 *uiData);
extern BOOL   GSensor_ParkingMode(void);
extern BOOL   GSensor_CrashMode(void);
extern void   GSensor_SetSensitivity(GSENSOR_SENSITIVITY GSensorSensitivity);


extern BOOL   GSensor_I2C_Init(GSENSOR_INFO GSensorInfo);
extern void   GSensor_I2C_WriteReg(UINT32 uiAddr, UINT32 uiValue);
extern UINT32 GSensor_I2C_ReadReg(UINT32 uiAddr);


#endif  //__Gsensor_H
