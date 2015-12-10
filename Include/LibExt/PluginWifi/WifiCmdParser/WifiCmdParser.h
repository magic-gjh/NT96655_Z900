#ifndef _WIFICMDPARSER_H_
#define _WIFICMDPARSER_H_




#ifdef __ECOS


#include "Type.h"
extern void WifiCmd_InstallID(void) _SECTION(".kercfg_text");


typedef struct _WIFI_CMD_ENTRY
{
    UINT32                cmd;
    UINT32                event;
    UINT32                usrCB;
    FLGPTN                Waitflag;
    UINT32                UIflag;
}
WIFI_CMD_ENTRY;

typedef void WifiCmd_DefCB(UINT32 cmd,UINT32 ret,cyg_uint32 bufAddr, cyg_uint32* bufSize, char* mimeType);
typedef void WifiCmd_EventHandle(UINT32 evt, UINT32 paramNum, ...);

#define WIFI_CMD_BEGIN(tbl) WIFI_CMD_ENTRY (Cmd_##tbl)[]={ ///< begin a command table
#define WIFI_CMD_ITEM(cmd,event,func,wait,UIflag)  {(cmd), (event),(func),(wait),(UIflag)}, ///< insert a command item in command table
#define WIFI_CMD_END()    {0,0,0,0}}; ///< end a command table

#define CMD_STR     "custom=1&cmd="
#define PAR_STR     "&par="
#define PARS_STR    "&str="
#define HTML_PATH   "A:\\test.htm"

#define WIFI_CMD_TERMINATE    (-255)
#define WIFI_CMD_NOT_FOUND    (-256)

#define WIFI_PAR_STR_LEN        (64)

extern void   WifiCmd_Done(FLGPTN flag,UINT32 result);
extern void   WifiCmd_SetExecTable(WIFI_CMD_ENTRY *pAppCmdTbl);
extern WIFI_CMD_ENTRY *  WifiCmd_GetExecTable(void);
extern int    WifiCmd_GetData(char* path, char* argument, cyg_uint32 bufAddr, cyg_uint32* bufSize, char* mimeType, cyg_uint32 segmentCount);
extern void   WifiCmd_SetDefautCB(UINT32 defaultCB);
extern void   WifiCmd_SetEventHandle(UINT32 eventHandle);
extern void   WifiCmd_ReceiveCmd(UINT32 enable);
extern void   WifiCmd_UnlockString(char *string);
extern char   *WifiCmd_LockString(void);
#endif
#endif //_WIFICMDPARSER_H_



