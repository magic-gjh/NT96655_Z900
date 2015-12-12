/**
    IPL dzoom table OV9712_TVP5150_CARDV_FFFF.

    @file       IPL_dzoomTableOV9712_TVP5150_CARDV_FFFF.c
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

#include "IPL_SC2033P_TVP5150_CARDV_FFFF_Int.h"

const UINT32 VDOZOOM_TVP5150_INFOR_MODE_TABLE[1][DZOOM_ITEM_MAX] =
{
    //sie       sie
    //crop out  ch0 out    pre in
    {1440, 240, 1440, 240, 1440, 240}
    //{720, 120, 720, 120, 720, 120}
};
const UINT32 VDOZOOM_INFOR_MODE_1_TABLE[31][DZOOM_ITEM_MAX] =
{
    //sie         sie
    //crop out    ch0 out      pre in
//{ 1920, 1080, 1920, 1080, 1920, 1080},
{ 1888, 1064, 1888, 1064, 1888, 1064},
{ 1824, 1024, 1824, 1024, 1824, 1024},
{ 1776,  996, 1776,  996, 1776,  996},
{ 1728,  972, 1728,  972, 1728,  972},
{ 1680,  944, 1680,  944, 1680,  944},
{ 1632,  916, 1632,  916, 1632,  916},
{ 1584,  888, 1584,  888, 1584,  888},
{ 1536,  864, 1536,  864, 1536,  864},
{ 1488,  836, 1488,  836, 1488,  836},
{ 1440,  808, 1440,  808, 1440,  808},
{ 1392,  780, 1392,  780, 1392,  780},
{ 1344,  756, 1344,  756, 1344,  756},
{ 1296,  728, 1296,  728, 1296,  728},
{ 1248,  700, 1248,  700, 1248,  700},
{ 1216,  684, 1216,  684, 1216,  684},
{ 1168,  656, 1168,  656, 1168,  656},
{ 1120,  628, 1120,  628, 1120,  628},
{ 1072,  600, 1072,  600, 1072,  600},
{ 1024,  576, 1024,  576, 1024,  576},
{  976,  548,  976,  548,  976,  548},
{  928,  520,  928,  520,  928,  520},
{  880,  492,  880,  492,  880,  492},
{  832,  468,  832,  468,  832,  468},
{  784,  440,  784,  440,  784,  440},
{  736,  412,  736,  412,  736,  412},
{  688,  384,  688,  384,  688,  384},
{  640,  360,  640,  360,  640,  360},
{  608,  340,  608,  340,  608,  340},
{  560,  312,  560,  312,  560,  312},
{  512,  288,  512,  288,  512,  288},
{  464,  260,  464,  260,  464,  260},
};


const UINT32 VDOZOOM_INFOR_MODE_2_TABLE[31][DZOOM_ITEM_MAX] =
{
    //sie         sie
    //crop out    ch0 out      pre in
{ 1280, 720, 1280, 720, 1280, 720},
{ 1248, 700, 1248, 700, 1248,  700},
{ 1216, 684, 1216, 684, 1216,  684},
{ 1184, 664, 1184, 664, 1184,  664},
{ 1152, 648, 1152, 648, 1152,  648},
{ 1120, 628, 1120, 628, 1120,  628},
{ 1088, 612, 1088, 612, 1088,  612},
{ 1056, 592, 1056, 592, 1056,  592},
{ 1024, 576, 1024, 576, 1024,  576},
{  992, 556,  992, 556,  992,  556},
{  960, 540,  960, 540,  960,  540},
{  928, 520,  928, 520,  928,  520},
{  896, 504,  896, 504,  896,  504},
{  864, 484,  864, 484,  864,  484},
{  832, 468,  832, 468,  832,  468},
{  800, 448,  800, 448,  800,  448},
{  768, 432,  768, 432,  768,  432},
{  736, 412,  736, 412,  736,  412},
{  704, 396,  704, 396,  704,  396},
{  672, 376,  672, 376,  672,  376},
{  640, 360,  640, 360,  640,  360},
{  608, 340,  608, 340,  608,  340},
{  576, 324,  576, 324,  576,  324},
{  544, 304,  544, 304,  544,  304},
{  512, 288,  512, 288,  512,  288},
{  480, 268,  480, 268,  480,  268},
{  448, 252,  448, 252,  448,  252},
{  416, 232,  416, 232,  416,  232},
{  384, 216,  384, 216,  384,  216},
{  352, 196,  352, 196,  352,  196},
{  320, 180,  320, 180,  320,  180},
};


UINT32* SenMode2Tbl(IPL_PROC_ID Id, UINT32 SenMode, UINT32 *DzMaxidx)
{
    UINT32 *Ptr = NULL;
	debug_msg("^GKENPHY SENMODE:%d   %d\r\n",Id, SenMode);
	
	//SenMode = SENSOR_MODE_1;
    if (Id == IPL_ID_1)
    {
        switch(SenMode)
        {
	        case SENSOR_MODE_1:
	            Ptr = (UINT32*)&VDOZOOM_INFOR_MODE_1_TABLE[0][0];
	            *DzMaxidx = (sizeof(VDOZOOM_INFOR_MODE_1_TABLE) / 4 / DZOOM_ITEM_MAX) - 1;
	            break;

	        case SENSOR_MODE_2:
	            Ptr = (UINT32*)&VDOZOOM_INFOR_MODE_2_TABLE[0][0];
	            *DzMaxidx = (sizeof(VDOZOOM_INFOR_MODE_2_TABLE) / 4 / DZOOM_ITEM_MAX) - 1;
	            break;

            default:
                DBG_ERR("Wrong Dzoom Mode\r\n");
                Ptr = NULL;
                *DzMaxidx = 0;
                break;
        }
    }
    else if (Id == IPL_ID_2)
    {
        switch(SenMode)
        {
            case SENSOR_MODE_1:
                Ptr = (UINT32*)&VDOZOOM_TVP5150_INFOR_MODE_TABLE[0][0];
                *DzMaxidx = (sizeof(VDOZOOM_TVP5150_INFOR_MODE_TABLE) / 4 / DZOOM_ITEM_MAX) - 1;
                break;

            default:
                DBG_ERR("Wrong Dzoom Mode\r\n");
                Ptr = NULL;
                *DzMaxidx = 0;
                break;
        }
    }
    return Ptr;
}
