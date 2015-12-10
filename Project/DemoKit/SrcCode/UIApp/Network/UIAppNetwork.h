#ifndef _UIAPPNET_H_
#define _UIAPPNET_H_

#include <cyg/io/eth/rltk/819x/wlan/ieee802_mib.h>
#include "wifi_utility.h"

#define TXT_WIFI_AP_MODE  "AP mode"
#define TXT_WIFI_ST_MODE  "station mode"
#define TXT_WIFI_CONNECT  "Conntecting"
#if (_NETWORK_ == _NETWORK_SDIO_EVB_WIFI_)
#define TXT_WIFI          "wifi"
#define TXT_WIFI_SET      "wifi Setting"
#define TXT_WIFI_START    "wifi start"
#else
#define TXT_WIFI          "LAN"
#define TXT_WIFI_SET      "LAN Setting"
#define TXT_WIFI_START    "LAN start"

#endif


typedef enum
{
    NET_AP_MODE = 0,
    NET_STATION_MODE,
    NET_P2P_DEV_MODE,
    NET_P2P_GO_MODE,
    NET_MODE_SETTING_MAX
}NET_MODE_SETTING;


typedef enum
{
    NET_AUTH_NONE = 0,
    NET_AUTH_WPA2 ,     //wpa2-psk-aes
    NET_AUTH_WPA,       //
    NET_AUTH_SETTING_MAX
}NET_AUTH_SETTING;


#define WIFI_IFNAME             "wlan0"   //interface name

typedef struct _wifi_settings
{
    int wlan_disable;
    int wps_disable;
    char mode[16];
    char security[32];
    int channel;
    char ssid[WSC_MAX_SSID_LEN+1];
    char passphrase[MAX_NETWORK_KEY_LEN+1];
    char wep_key1[MAX_WEP_KEY_LEN+1];
    char wep_key2[MAX_WEP_KEY_LEN+1];
    char wep_key3[MAX_WEP_KEY_LEN+1];
    char wep_key4[MAX_WEP_KEY_LEN+1];
    int wep_default_key;
    char wsc_pin[PIN_LEN+1];
    int is_configured;
    int config_by_ext_reg;
    int p2pmode;
    char device_name[MAX_DEVICE_NAME_LEN+1];
    int is_dhcp; // static ip or dhcp for client mode
} wifi_settings;


extern void UINet_WifiBack2Dev(void);
extern void UINet_WifiP2PReInit(void);
extern INT32 UINet_WifiInit(UINT32 mode,UINT32 security);
extern INT32 UINet_LanInit(UINT32 mode);
extern INT32 UINet_HFSInit(void);
extern INT32 UINet_WifiUnInit(UINT32 mode);
extern INT32 UINet_LanUnInit(UINT32 mode);
extern INT32 UINet_HFSUnInit(void);
extern char* UINet_GetSSID(void);
extern char* UINet_GetPASSPHRASE(void);
extern char* UINet_GetIP(void);
extern char* UINet_GetMAC(void);
extern char* UINet_GetConnectedMAC(void);
extern void UINet_SetAuthType(NET_AUTH_SETTING authtype);
extern UINT32 UINet_GetAuthType(void);
extern void UINet_SetSSID(char *ssid,UINT32 len);
extern void UINet_SetPASSPHRASE(char *pwd,UINT32 len);
extern void UINet_SetChannel(UINT32 channel);
extern UINT32 UINet_GetChannel(void);
extern void UINet_RemoveUser(void);
extern void UINet_AddACLTable(void);
extern void UINet_ClearACLTable(void);
extern void UINet_SiteSurvey(char *ifname,SS_STATUS_Tp pStatus);
extern void UINet_DumpAllSSID(SS_STATUS_Tp pStatus);
extern UINT32 UINet_DHCP_Start(UINT32 isClient);
extern void UINet_P2PDump(SS_STATUS_Tp pStatus);
extern void UINet_P2PScan(char *ifname,SS_STATUS_Tp pStatus);
extern UINT32 UINet_P2PConnect(SS_STATUS_Tp pStatus,UINT32 idx);

#endif //_UIAPPNET_H_
