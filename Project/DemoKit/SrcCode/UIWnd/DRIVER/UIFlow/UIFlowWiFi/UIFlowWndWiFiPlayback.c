//This source code is generated by UI Designer Studio.

#include "UIFramework.h"
#include "UIFrameworkExt.h"
#include "UIGraphics.h"
#include "NVTToolCommand.h"
#include "UIFlowWndWiFiPlaybackRes.c"
#include "UIFlowWndWiFiPlayback.h"
#include "PrjCfg.h"

//---------------------UIFlowWndWiFiPlaybackCtrl Debug Definition -----------------------------
#define _UIFLOWWNDWIFIPLAYBACK_ERROR_MSG        1
#define _UIFLOWWNDWIFIPLAYBACK_TRACE_MSG        0

#if (_UIFLOWWNDWIFIPLAYBACK_ERROR_MSG&(PRJ_DBG_LVL>=PRJ_DBG_LVL_ERR))
#define UIFlowWndWiFiPlaybackErrMsg(...)            debug_msg ("^R UIFlowWndWiFiPlayback: "__VA_ARGS__)
#else
#define UIFlowWndWiFiPlaybackErrMsg(...)
#endif

#if (_UIFLOWWNDWIFIPLAYBACK_TRACE_MSG&(PRJ_DBG_LVL>=PRJ_DBG_LVL_TRC))
#define UIFlowWndWiFiPlaybackTraceMsg(...)          debug_msg ("^B UIFlowWndWiFiPlayback: "__VA_ARGS__)
#else
#define UIFlowWndWiFiPlaybackTraceMsg(...)
#endif

//---------------------UIFlowWndWiFiPlaybackCtrl Global Variables -----------------------------

//---------------------UIFlowWndWiFiPlaybackCtrl Prototype Declaration  -----------------------

//---------------------UIFlowWndWiFiPlaybackCtrl Public API  ----------------------------------

//---------------------UIFlowWndWiFiPlaybackCtrl Private API  ---------------------------------
//---------------------UIFlowWndWiFiPlaybackCtrl Control List---------------------------
CTRL_LIST_BEGIN(UIFlowWndWiFiPlayback)
CTRL_LIST_END

//----------------------UIFlowWndWiFiPlaybackCtrl Event---------------------------
INT32 UIFlowWndWiFiPlayback_OnOpen(VControl *, UINT32, UINT32 *);
INT32 UIFlowWndWiFiPlayback_OnClose(VControl *, UINT32, UINT32 *);
EVENT_BEGIN(UIFlowWndWiFiPlayback)
EVENT_ITEM(NVTEVT_OPEN_WINDOW,UIFlowWndWiFiPlayback_OnOpen)
EVENT_ITEM(NVTEVT_CLOSE_WINDOW,UIFlowWndWiFiPlayback_OnClose)
EVENT_END

INT32 UIFlowWndWiFiPlayback_OnOpen(VControl *pCtrl, UINT32 paramNum, UINT32 *paramArray)
{
    Ux_DefaultEvent(pCtrl,NVTEVT_OPEN_WINDOW,paramNum,paramArray);
    return NVTEVT_CONSUME;
}
INT32 UIFlowWndWiFiPlayback_OnClose(VControl *pCtrl, UINT32 paramNum, UINT32 *paramArray)
{
    Ux_DefaultEvent(pCtrl,NVTEVT_CLOSE_WINDOW,paramNum,paramArray);
    return NVTEVT_CONSUME;
}
