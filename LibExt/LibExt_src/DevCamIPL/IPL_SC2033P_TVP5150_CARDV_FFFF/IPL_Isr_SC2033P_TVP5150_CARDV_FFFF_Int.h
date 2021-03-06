#ifndef IPL_ISR_OV9712_TVP5150_CARDV_FFFF_INT_H_
#define IPL_ISR_OV9712_TVP5150_CARDV_FFFF_INT_H_
/**
    IPL_Isr_OV9712_TVP5150_CARDV_FFFF_EVB_FF_Int.h


    @file       IPL_Isr_OV9712_TVP5150_CARDV_FFFF_EVB_FF_Int.h
    @ingroup    mISYSAlg
    @note       Nothing (or anything need to be mentioned).

    Copyright   Novatek Microelectronics Corp. 2011.  All rights reserved.
*/

extern void IPL_IsrSIECB(IPL_PROC_ID Id, UINT32 IntStatus,UINT32 idx);
extern void IPL_IsrIFECB(IPL_PROC_ID Id, UINT32 IntStatus,UINT32 idx);
extern void IPL_IsrIPECB(IPL_PROC_ID Id, UINT32 IntStatus,UINT32 idx);
extern void IPL_IsrIMECB(IPL_PROC_ID Id, UINT32 IntStatus,UINT32 idx);

#endif //IPL_ISR_OV9712_TVP5150_CARDV_FFFF_H_