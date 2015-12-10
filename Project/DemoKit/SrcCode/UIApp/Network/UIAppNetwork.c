
#include "UIInfo.h"


#if defined(__ECOS) && (_NETWORK_ == _NETWORK_SDIO_EVB_WIFI_)
#include <cyg/io/eth/rltk/819x/wlan/wifi_api.h>
#endif
#ifdef __ECOS
#include "net_api.h"
#include <cyg/fileio/fileio.h>
#include "ecos_FileSys.h"
#include <cyg/fs/nvtfs.h>
#include <cyg/hfs/hfs.h>

#include "DhcpNvt.h"
#include <net/dhcpelios/dhcpelios.h>

#include "wps_api.h"
#include "UIAppNetwork.h"
#include "WifiAppCmd.h"
#include "WifiAppXML.h"
#endif
#include "Timer.h"
#include "NVTToolCommand.h"
#include "UIBackgroundObj.h"
#include "AppCommon.h"

//global debug level: PRJ_DBG_LVL
#include "PrjCfg.h"
//local debug level: THIS_DBGLVL
#define THIS_DBGLVL         2 //0=OFF, 1=ERROR, 2=TRACE
///////////////////////////////////////////////////////////////////////////////
#define __MODULE__          UINet
#define __DBGLVL__          ((THIS_DBGLVL>=PRJ_DBG_LVL)?THIS_DBGLVL:PRJ_DBG_LVL)
#define __DBGFLT__          "*" //*=All, [mark]=CustomClass
#include "DebugModule.h"
///////////////////////////////////////////////////////////////////////////////

#ifdef __ECOS
#define MOUNT_FS_ROOT        "/sdcard"
char gNvtDhcpHostNameCli[DHCP_HOST_NAME_MAX_LEN] = "nvtsys01cli";
char gNvtDhcpHostNameSrv[DHCP_HOST_NAME_MAX_LEN] = "nvtsys01srv";
char gMacAddr[6] = {0,0,0,0x81,0x89,0xe5};
UINT32 gAuthType = NET_AUTH_WPA2;
char gSSID[WSC_MAX_SSID_LEN] = "NVT_CARDV";
char gPASSPHRASE[MAX_WEP_KEY_LEN] = "12345678";
char gCurrIP[NET_IP_MAX_LEN] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
UINT32 gChannel = 0;//default 0:auto
char gLastMacAddr[NET_IP_MAX_LEN] = {0};
char gRemoveMacAddr[NET_IP_MAX_LEN] = {0};
char currentMacAddr[NET_IP_MAX_LEN] = {0};
#define EXAM_NET_IP_ETH0            "192.168.0.3"
#define EXAM_NET_NETMASK_ETH0       "255.255.255.0"
#define EXAM_NET_BRAODCAST_ETH0     "192.168.0.255"
#define EXAM_NET_GATEWAY_ETH0       "192.168.0.1"
#define EXAM_NET_SRVIP_ETH0         "192.168.0.1"
#define EXAM_NET_LEASE_START_ETH0   "192.168.0.12"
#define EXAM_NET_LEASE_END_ETH0     "192.168.0.32"
#define EXAM_NET_LEASE_DNS_ETH0     "0.0.0.0"
#define EXAM_NET_LEASE_WINS_ETH0    "0.0.0.0"

#define EXAM_NET_IP_ETH1            "192.168.1.3"
#define EXAM_NET_NETMASK_ETH1       "255.255.255.0"
#define EXAM_NET_BRAODCAST_ETH1     "192.168.1.255"
#define EXAM_NET_GATEWAY_ETH1       "192.168.1.1"
#define EXAM_NET_SRVIP_ETH1         "192.168.1.1"

#define EXAM_NET_IP_WLAN0            "192.168.1.254"
#define EXAM_NET_NETMASK_WLAN0       "255.255.255.0"
#define EXAM_NET_BRAODCAST_WLAN0     "192.168.1.255"
#define EXAM_NET_GATEWAY_WLAN0       "192.168.1.254"
#define EXAM_NET_SRVIP_WLAN0         "192.168.1.254"
#define EXAM_NET_LEASE_START_WLAN0   "192.168.1.33"
#define EXAM_NET_LEASE_END_WLAN0     "192.168.1.52"
#define EXAM_NET_LEASE_DNS_WLAN0     "0.0.0.0"
#define EXAM_NET_LEASE_WINS_WLAN0    "0.0.0.0"


extern UIMenuStoreInfo  currentInfo;
wifi_settings wifiConfig = {0};
struct wps_config gWpscfg;

#if (WIFI_FTP_FUNC==ENABLE)
void start_ftpserver(void);
void stop_ftpserver(void);
#endif

char gNetIntf[3][6] =
{
    'e','t','h','0',0,0,
    'e','t','h','1',0,0,
    'w','l','a','n','0',0,
};
cyg_uint32 xExamHfs_getCurrTime(void)
{
    return timer_getCurrentCount(timer_getSysTimerID());
}

static void UINet_HFSNotifyStatus(int status)
{
    switch (status)
    {
        // HTTP client has request coming in
        case CYG_HFS_STATUS_CLIENT_REQUEST:
             //DBG_IND("client request %05d ms\r\n",xExamHfs_getCurrTime()/1000);
             break;
        // HTTP server send data start
        case CYG_HFS_STATUS_SERVER_RESPONSE_START:
             //DBG_IND("send data start, time= %05d ms\r\n",xExamHfs_getCurrTime()/1000);
             break;
        // HTTP server send data start
        case CYG_HFS_STATUS_SERVER_RESPONSE_END:
             //DBG_IND("send data end, time= %05d ms\r\n",xExamHfs_getCurrTime()/1000);
             break;
        // HTTP client disconnect
        case CYG_HFS_STATUS_CLIENT_DISCONNECT:
             //DBG_IND("client disconnect, time= %05d ms\r\n",xExamHfs_getCurrTime()/1000);
             break;

    }
}
int UINet_HFSUploadResultCb(int result, cyg_uint32 bufAddr, cyg_uint32* bufSize, char* mimeType)
{
    XML_DefaultFormat(WIFIAPP_CMD_UPLOAD_FILE,result,bufAddr,(cyg_uint32 *)bufSize,mimeType);
    return CYG_HFS_CB_GETDATA_RETURN_OK;
}

int UINet_HFSDeleteResultCb(int result, cyg_uint32 bufAddr, cyg_uint32* bufSize, char* mimeType)
{
    XML_DefaultFormat(WIFIAPP_CMD_DELETE_ONE,result,bufAddr,(cyg_uint32 *)bufSize,mimeType);
    return CYG_HFS_CB_GETDATA_RETURN_OK;
}

void Net_DhcpCliCb(NET_DHCP_CB_TYPE cbType, unsigned int p1, unsigned int p2, unsigned int p3)
{
    DBG_IND("::type=%d,P=%d, %d, %d\r\n",cbType, p1, p2, p3);
    if (NET_DHCP_CB_TYPE_CLI_REQ_CNT == cbType)
    {
        //post an event to UI-task show (cycle,count) that client request an IP
    }
    else if (NET_DHCP_CB_TYPE_CLI_REQ_RESULT == cbType)
    {
        if (p1)
        {
            DBG_DUMP("DHCP Client IP = 0x%x\r\n",p1);
            //post an event to UI-task to call,Dhcp_Client_Start_Ok(p1) for the first time.
            //post an event to UI-task to show Got-IP
        }
        else
        {
            DBG_DUMP("DHCP Client IP Fail\r\n");
            //post an event to UI-task to Close DHCP Client,Dhcp_Client_Stop(), and show Get-IP fail
        }
    }
    else if (NET_DHCP_CB_TYPE_MAX == cbType)
    {
        OS_DumpKernelResStatus();
    }
}

//////////////////////////////////////////////////////////////////////////
char* UINet_GetConnectedMAC(void)
{
    return currentMacAddr;
}
void UINet_SetAuthType(NET_AUTH_SETTING authtype)
{
    gAuthType = authtype;
}
UINT32 UINet_GetAuthType(void)
{
    return gAuthType;
}
char* UINet_GetSSID(void)
{
    return gSSID;
}
char* UINet_GetPASSPHRASE(void)
{
    return gPASSPHRASE;
}
char* UINet_GetIP(void)
{
    return gCurrIP;
}
char* UINet_GetMAC(void)
{
    return gMacAddr;
}
void UINet_SetSSID(char *ssid,UINT32 len)
{
    if(len>WSC_MAX_SSID_LEN)
    {
        DBG_ERR("max len %d\r\n",WSC_MAX_SSID_LEN);
        len = WSC_MAX_SSID_LEN;
    }
    memset(gSSID,'\0',WSC_MAX_SSID_LEN);
    sprintf(gSSID,ssid,len);
    DBG_IND("%s\r\n",gSSID);
    strncpy(currentInfo.strSSID, UINet_GetSSID(), (WSC_MAX_SSID_LEN -1)); // Save Wi-Fi SSID.
    currentInfo.strSSID[strlen(currentInfo.strSSID)] = '\0';
}
void UINet_SetPASSPHRASE(char *pwd,UINT32 len)
{

    if(len>MAX_WEP_KEY_LEN)
    {
        DBG_ERR("max len %d\r\n",MAX_WEP_KEY_LEN);
        len = MAX_WEP_KEY_LEN;
    }
    memset(gPASSPHRASE,'\0',MAX_WEP_KEY_LEN);
    strncpy(gPASSPHRASE,pwd,len);
    DBG_IND("%s\r\n",gPASSPHRASE);

    strncpy(currentInfo.strPASSPHRASE, UINet_GetPASSPHRASE(), (MAX_WEP_KEY_LEN -1)); // Save Wi-Fi PASSWORD.
    currentInfo.strPASSPHRASE[strlen(currentInfo.strPASSPHRASE)] = '\0';
}
//0 for auto channel, 1 -14 for 11b/11g,36 -165 for 11a
void UINet_SetChannel(UINT32 channel)
{
    gChannel=channel;
}
UINT32 UINet_GetChannel(void)
{
    return gChannel;
}

#if(_NETWORK_ == _NETWORK_SDIO_EVB_WIFI_)

static int IS_MCAST(unsigned char *da)
{
    if ((*da) & 0x01)
        return 1;
    else
        return 0;
}

void UINet_RemoveUser(void)
{
    UINT32 bNewUser = FALSE;

    CHKPNT;
    //if 2nd user connect,diconnect 1st user
    if(strncmp(currentMacAddr,gLastMacAddr,strlen(currentMacAddr))!=0 )
    {
        //notify current connected socket
        //new connect not get IP and socket not create
        //cannot disconnet immediate,suggest after 500 ms
        WifiApp_SendCmd(WIFIAPP_CMD_NOTIFY_STATUS, WIFIAPP_RET_REMOVE_BY_USER);
        bNewUser = TRUE;
    }

    Ux_PostEvent(NVTEVT_WIFI_AUTHORIZED_OK, 1,bNewUser);
}
void UINet_AddACLTable(void)
{
    if(strncmp(currentMacAddr,gLastMacAddr,strlen(currentMacAddr))!=0 )
    {
        if(strlen(gRemoveMacAddr)!=0)
        {
            DBG_ERR("%s not remove from ACL !!\r\n",gRemoveMacAddr);
        }

        strncpy(gRemoveMacAddr,gLastMacAddr,strlen(gLastMacAddr));
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "add_acl_table", gRemoveMacAddr, NULL_STR);
        strncpy(gLastMacAddr,currentMacAddr,strlen(currentMacAddr));
        DBG_IND("********** gLastMacAddr    %s\r\n",gLastMacAddr);
        DBG_IND("********** gRemoveMacAddr  %s\r\n",gRemoveMacAddr);

        BKG_PostEvent(NVTEVT_BKW_WIFI_CLEARACL);

    }
}
void UINet_ClearACLTable(void)
{
    if(strlen(gRemoveMacAddr))
    {
        DBG_IND("**********clr gRemoveMacAddr  %s\r\n",gRemoveMacAddr);
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "rm_acl_table", gRemoveMacAddr, NULL_STR);
        memset(gRemoveMacAddr,0,strlen(gRemoveMacAddr));
    }
}


void UINet_StationStatus_CB(char *pIntfName, char *pMacAddr, int status)
{
    //#NT#2013/12/06#KS Hung -begin
    //#NT#Post AUTHORIZED_OK event
    if (status == WIFI_STA_STATUS_ASSOCIATED) {

    DBG_IND("ASSOCIATED %d ms\r\n",Perf_GetCurrent()/1000);

        DBG_IND("%s: A client(%02X:%02X:%02X:%02X:%02X:%02X) is associated\r\n",
            pIntfName, *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));
    }
    else if (status == WIFI_STA_STATUS_REASSOCIATED) {
        DBG_IND("%s: A client(%02X:%02X:%02X:%02X:%02X:%02X) is reassociated\r\n",
            pIntfName, *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));
    }
    else if (status == WIFI_STA_STATUS_DISASSOCIATED) {
        DBG_IND("%s: A client(%02X:%02X:%02X:%02X:%02X:%02X) is disassociated\r\n",
            pIntfName, *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));

        Ux_PostEvent(NVTEVT_WIFI_DEAUTHENTICATED, 0);
    }
    else if (status == WIFI_STA_STATUS_DEAUTHENTICATED) {
        DBG_IND("%s: A client(%02X:%02X:%02X:%02X:%02X:%02X) is deauthenticated\r\n",
            pIntfName, *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));

        Ux_PostEvent(NVTEVT_WIFI_DEAUTHENTICATED, 0);
    }
    else if (status == WIFI_STA_STATUS_PORT_AUTHORIZED) {

        DBG_IND("%s: A client(%02X:%02X:%02X:%02X:%02X:%02X) is port authorized\r\n",
            pIntfName, *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));

        {
            sprintf(currentMacAddr,"%02x%02x%02x%02x%02x%02x",
            *pMacAddr, *(pMacAddr+1), *(pMacAddr+2), *(pMacAddr+3), *(pMacAddr+4), *(pMacAddr+5));
            DBG_IND("********** currentMacAddr  %s\r\n",currentMacAddr);

            if(strlen(gLastMacAddr)==0)
            {
                strncpy(gLastMacAddr,currentMacAddr,strlen(currentMacAddr));
                DBG_IND("********** gLastMacAddr    %s\r\n",gLastMacAddr);
            }

            Ux_PostEvent(NVTEVT_WIFI_AUTHORIZED_OK, 1,0);
        }

    }
    else if (status == WIFI_AP_READY) {
        DBG_IND("AP ready\r\n");
        DBG_ERR("AP ready %d ms\r\n",Perf_GetCurrent()/1000);

    }
    else
        DBG_IND("unknown status %d\r\n",status);
    //#NT#2013/12/06#KS Hung -end
}

void UINet_Link2APStatus_CB(char *pIntfName, int status)
{
    if (status == WIFI_LINK_STATUS_CONNECTED) {
        DBG_IND("%s: Connected with AP\r\n", pIntfName);
        #if 0 // Unused code. Because new Wi-Fi driver has fixed this issue.
        RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", gCurrIP, NULL_STR, NULL_STR);  // Issue ARP Request to router.
        #endif
        if (strcmp(wifiConfig.mode, "sta") == 0){
            Ux_PostEvent(NVTEVT_EXE_WIFI_DHCP_START,1,1);
        }
        Ux_PostEvent(NVTEVT_WIFI_AUTHORIZED_OK, 0);
    }
    else if (status == WIFI_LINK_STATUS_DISCONNECTED) {
        DBG_IND("%s: Disconnected with AP\r\n", pIntfName);
    }
    else if (status == WIFI_LINK_STATUS_DEAUTHENTICATED) {
        DBG_IND("%s: Deauthenticated\r\n", pIntfName);
    }
    else
        DBG_IND("unknown status %d \r\n",status);
}
void UINet_P2PEvent_CB(char *pIntfName,  int status)
{
    if (status==WIFI_P2P_EVENT_BACK2DEV) {
        Ux_PostEvent(NVTEVT_EXE_WIFI_BACK2DEV,0);
    }
    else if (status==WIFI_P2P_EVENT_START_DHCPD) {
        Ux_PostEvent(NVTEVT_EXE_WIFI_DHCP_START,1,0);
    }
    else if(status==WIFI_P2P_EVENT_START_DHCPC) {
        Ux_PostEvent(NVTEVT_EXE_WIFI_DHCP_START,1,1);
    }
}

int UINet_WSCFlashParam_CB(struct wsc_flash_param *param)
{
    wifi_settings *pwifi=&wifiConfig;

    pwifi->is_configured = 1;
    strcpy(pwifi->ssid, param->SSID);
    DBG_IND("new SSID %s \r\n", pwifi->ssid);

    pwifi->config_by_ext_reg = param->WSC_CONFIGBYEXTREG;
    if (param->WEP) {
        if (param->WEP == 1) //WEP64
            strcpy(pwifi->security, "wep64-auto");
        else
            strcpy(pwifi->security, "wep128-auto");

        pwifi->wep_default_key = param->WEP_DEFAULT_KEY;
        strcpy(pwifi->wep_key1, param->WEP_KEY1);
        strcpy(pwifi->wep_key2, param->WEP_KEY1);
        strcpy(pwifi->wep_key3, param->WEP_KEY1);
        strcpy(pwifi->wep_key4, param->WEP_KEY1);
    }
    else {
        if (param->WSC_AUTH==WSC_AUTH_OPEN) {
            if (param->WSC_ENC==WSC_ENCRYPT_NONE) {
                strcpy(pwifi->security, "none");
            }
        }
        else if (param->WSC_AUTH==WSC_AUTH_WPAPSK) {
            if (param->WSC_ENC==WSC_ENCRYPT_TKIP)
                strcpy(pwifi->security, "wpa-psk-tkip");
            else if (param->WSC_ENC==WSC_ENCRYPT_AES)
                strcpy(pwifi->security, "wpa-psk-aes");
            else if (param->WSC_ENC==WSC_ENCRYPT_TKIPAES)
                strcpy(pwifi->security, "wpa-psk-tkipaes");
            strcpy(pwifi->passphrase, param->WSC_PSK);
        }
        else if (param->WSC_AUTH==WSC_AUTH_WPA2PSK) {
            if (param->WSC_ENC==WSC_ENCRYPT_TKIP)
                strcpy(pwifi->security, "wpa2-psk-tkip");
            else if (param->WSC_ENC==WSC_ENCRYPT_AES)
                strcpy(pwifi->security, "wpa2-psk-aes");
            else if (param->WSC_ENC==WSC_ENCRYPT_TKIPAES)
                strcpy(pwifi->security, "wpa2-psk-tkipaes");
            strcpy(pwifi->passphrase, param->WSC_PSK);
        }
        else if (param->WSC_AUTH==WSC_AUTH_WPA2PSKMIXED) {
            if (param->WSC_ENC==WSC_ENCRYPT_TKIP)
                strcpy(pwifi->security, "wpa-auto-psk-tkip");
            else if (param->WSC_ENC==WSC_ENCRYPT_AES)
                strcpy(pwifi->security, "wpa-auto-psk-aes");
            else if (param->WSC_ENC==WSC_ENCRYPT_TKIPAES)
                strcpy(pwifi->security, "wpa-auto-psk-tkipaes");
            strcpy(pwifi->passphrase, param->WSC_PSK);
        }
    }

    if (strcmp(pwifi->mode, "p2pdev") == 0) {
    DBG_IND("change p2pmode  %d\r\n",param->WLAN0_P2P_TYPE);
    pwifi->p2pmode = param->WLAN0_P2P_TYPE;
    }

    return 0;
}


void UINet_WSCEvent_CB(int evt)
{
    DBG_IND("evt %d\r\n",evt);
    if (evt == WPS_REINIT_EVENT) {

        Ux_PostEvent(NVTEVT_EXE_WIFI_REINIT,0);
    }
    else if (evt == WPS_STATUS_CHANGE_EVENT) {
        int status;
        status = get_wscd_status();
        print_wscd_status(status);
    }
    else
        DBG_IND("unknown event\n");
}
void UINet_DhcpCb(DHCPD_CLI_STS cliSts, dhcp_assign_ip_info *ipInfo, dhcpd_lease_tbl_loc *pTbl)
{
    switch(cliSts)
    {
    case DHCPD_CLI_STS_RELEASE_IP:
        break;
    case DHCPD_CLI_STS_REQUEST_IP:
        DBG_IND("REQUEST_IP %d ms\r\n",Perf_GetCurrent()/1000);
        break;
    case DHCPD_CLI_STS_DECLINE_IP:
        break;
    case DHCPD_CLI_STS_SRVSTART:
        break;
    case DHCPD_CLI_STS_SRVREADY:
        break;
    default:
        break;
    }
}

BOOL UINet_SetFixLeaseInfo(UINT32 sec,UINT32 cnt)
{
    NET_DHCP_LEASE_CONF leaseInfo;

    memset((void *)&leaseInfo, 0, sizeof(NET_DHCP_LEASE_CONF));//
    memcpy((void *)leaseInfo.ip_pool_start, EXAM_NET_LEASE_START_WLAN0, strlen(EXAM_NET_LEASE_START_WLAN0));
    memcpy((void *)leaseInfo.ip_pool_end, EXAM_NET_LEASE_END_WLAN0, strlen(EXAM_NET_LEASE_END_WLAN0));
    memcpy((void *)leaseInfo.ip_dns, EXAM_NET_LEASE_DNS_WLAN0, strlen(EXAM_NET_LEASE_DNS_WLAN0));
    memcpy((void *)leaseInfo.ip_wins, EXAM_NET_LEASE_WINS_WLAN0, strlen(EXAM_NET_LEASE_WINS_WLAN0));
    memcpy((void *)leaseInfo.ip_server, EXAM_NET_SRVIP_WLAN0, strlen(EXAM_NET_SRVIP_WLAN0));
    memcpy((void *)leaseInfo.ip_gateway, EXAM_NET_GATEWAY_WLAN0, strlen(EXAM_NET_GATEWAY_WLAN0));
    memcpy((void *)leaseInfo.ip_subnetmask, EXAM_NET_NETMASK_WLAN0, strlen(EXAM_NET_NETMASK_WLAN0));
    memcpy((void *)leaseInfo.ip_broadcast, EXAM_NET_BRAODCAST_WLAN0, strlen(EXAM_NET_BRAODCAST_WLAN0));
    leaseInfo.lease_time = (unsigned int)sec;
    leaseInfo.max_lease_cnt = (unsigned int)cnt;
    Dhcp_Conf_Lease(&leaseInfo);

    return TRUE;
}

UINT32 UINet_DHCP_Start(UINT32 isClient)
{
    UINT32 result = E_OK;

    DBG_IND("isCli %d\r\n",isClient);

    if(isClient)
    {
        unsigned int gotIP = 0;

        //make sure dhcpc has closed
        Dhcp_Client_Stop();
        //make sure dhcpd has closed
        Dhcp_Server_Stop(0);
        //start dhcpc

        wifi_ignore_down_up("wlan0", 1);
        //set DHCP client
        result = Dhcp_Client_Start(gNvtDhcpHostNameCli, Net_DhcpCliCb, TRUE);
        if (NET_DHCP_RET_OK != result )
        {
            DBG_ERR("Dhcp cli fail %d\r\n",result);
        }
        else
        {
            Dhcp_Client_Get_IP(&gotIP);
            sprintf(gCurrIP,"%d.%d.%d.%d",(gotIP&0xFF), (gotIP>>8)&0xFF,(gotIP>>16)&0xFF,(gotIP>>24)&0xFF );
            DBG_IND("ip=0x%x %s\r\n",gotIP,gCurrIP);

            wifi_set_passwd_hash_delay(0); // Reset hash delay time once got IP.
            DBG_IND("^Bhash delay time:%d ms\r\n", wifi_get_passwd_hash_delay());
        }
        wifi_ignore_down_up("wlan0", 0);

    }
    else
    {
        //set DHCP server
        Dhcp_Set_Interface("wlan0");
        dhcpd_reg_cli_attach_cb_func(UINet_DhcpCb);
        result = Dhcp_Server_Start(gNvtDhcpHostNameSrv);
        if (NET_DHCP_RET_OK != result)
        {
            DBG_ERR("Dhcp svr fail\r\n");
        }
    }

    return result;
}
#if 0
void UINet_dump_wifi_settings(void)
{
    printf("wlan_disable=%d\n", wifiConfig.wlan_disable);
    printf("wps_disable=%d\n", wifiConfig.wps_disable);
    printf("mode=%s\n", wifiConfig.mode);
    if (strcmp(wifiConfig.mode, "sta") == 0)
        printf("is_dhcp=%d\n", wifiConfig.is_dhcp);
    printf("security=%s\n", wifiConfig.security);
    printf("channel=%d\n", wifiConfig.channel);
    printf("ssid=%s\n", wifiConfig.ssid);
    printf("passphrase=%s\n", wifiConfig.passphrase);
    printf("wep_default_key=%d\n", wifiConfig.wep_default_key);
    printf("wep_key1=%s\n", wifiConfig.wep_key1);
    printf("wep_key2=%s\n", wifiConfig.wep_key2);
    printf("wep_key3=%s\n", wifiConfig.wep_key3);
    printf("wep_key4=%s\n", wifiConfig.wep_key4);
    printf("wsc_pin=%s\n", wifiConfig.wsc_pin);
    printf("is_configured=%d\n", wifiConfig.is_configured);
    printf("config_by_ext_reg=%d\n", wifiConfig.config_by_ext_reg);
    printf("p2pmode=%d\n", wifiConfig.p2pmode);
}
#endif

INT32 UINet_WifiConfig(wifi_settings *pwifi)
{
    int intVal, is_client=0, is_wep=0;
    char tmpbuf[256];
    struct wps_config *pcfg;
    int auth, enc, mixedmode=0, wsc_wpa_cipher=0, wsc_wpa2_cipher=0;

    DBG_IND("mode %s ,p2pmode %d security %s\r\n",pwifi->mode,pwifi->p2pmode,pwifi->security);

    auth = WSC_AUTH_OPEN;
    enc = WSC_ENCRYPT_NONE;

    if (is_interface_up("wlan0"))
        interface_down("wlan0");

    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "band=11", NULL_STR); //bgn
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "ampdu=1", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "use40M=0", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "shortGI20M=1", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "shortGI40M=1", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "rtsthres=2347", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "fragthres=2346", NULL_STR);
    RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "autoch_ss_to=100", NULL_STR);  //for speed up


    //AP mode
    if (strcmp(pwifi->mode, "ap") == 0) {
        is_client = 0;
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "opmode=16", NULL_STR);
        sprintf(tmpbuf, "ssid=%s", pwifi->ssid);
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        sprintf(tmpbuf, "channel=%d", pwifi->channel);
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);

        if (strcmp(pwifi->security, "none") == 0) {
            // No security
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_NONE;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
        }
        else if (strcmp(pwifi->security, "wep64-auto") == 0) {
            // WEP64
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_WEP;
            //authtype: 0-open, 1-shared, 2-auto
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=1", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
            sprintf(tmpbuf, "wepdkeyid=%d", pwifi->wep_default_key);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey1=%s", pwifi->wep_key1);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey2=%s", pwifi->wep_key2);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey3=%s", pwifi->wep_key3);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey4=%s", pwifi->wep_key4);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wep128-auto") == 0) {
            // WEP128
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_WEP;
            //authtype: 0-open, 1-shared, 2-auto
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=5", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
            sprintf(tmpbuf, "wepdkeyid=%d", pwifi->wep_default_key);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey1=%s", pwifi->wep_key1);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey2=%s", pwifi->wep_key2);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey3=%s", pwifi->wep_key3);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey4=%s", pwifi->wep_key4);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wpa-psk-tkip") == 0) {
            // WPA-PSK, TKIP
            auth = WSC_AUTH_WPAPSK;
            enc = WSC_ENCRYPT_TKIP;
            mixedmode = 0;
            wsc_wpa_cipher = 1;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=1", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa_cipher=2", NULL_STR);
            sprintf(tmpbuf, "passphrase=%s", pwifi->passphrase);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wpa2-psk-aes") == 0) {
            // WPA2-PSK, AES
            auth = WSC_AUTH_WPA2PSK;
            enc = WSC_ENCRYPT_AES;
            mixedmode = 0;
            wsc_wpa2_cipher = 2;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa2_cipher=8", NULL_STR);
            sprintf(tmpbuf, "passphrase=%s", pwifi->passphrase);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wpa-auto-psk-tkipaes") == 0) {
            // WPA/WPA2 mixed PSK, TKIP/AES
            auth = WSC_AUTH_WPA2PSKMIXED;
            enc = WSC_ENCRYPT_TKIPAES;
            mixedmode = 1;
            wsc_wpa_cipher = 3;
            wsc_wpa2_cipher = 3;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=3", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa_cipher=10", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa2_cipher=10", NULL_STR);
            sprintf(tmpbuf, "passphrase=%s", pwifi->passphrase);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
    }
    //STA mode
    else if ((strcmp(pwifi->mode, "sta") == 0)||
             ((strcmp(pwifi->mode, "p2pdev") == 0) && (pwifi->p2pmode==P2P_CLIENT))) {
        is_client = 1;
        if ((strcmp(pwifi->mode, "p2pdev") == 0) && (pwifi->p2pmode==P2P_CLIENT)) {
            sprintf(tmpbuf, "opmode=0x%X", (WIFI_P2P_SUPPORT|WIFI_STATION_STATE));
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "setp2pmode=%d", P2P_CLIENT);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", tmpbuf, NULL_STR);
        }
        else
        {
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "opmode=8", NULL_STR);
        }
        sprintf(tmpbuf, "ssid=%s", pwifi->ssid);
        RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        if (strcmp(pwifi->security, "none") == 0) {
            // No security
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_NONE;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
        }
        else if (strcmp(pwifi->security, "wep64-auto") == 0) {
            // WEP64
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_WEP;
            //authtype: 0-open, 1-shared, 2-auto
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=1", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
            sprintf(tmpbuf, "wepdkeyid=%d", pwifi->wep_default_key);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey1=%s", pwifi->wep_key1);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey2=%s", pwifi->wep_key2);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey3=%s", pwifi->wep_key3);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey4=%s", pwifi->wep_key4);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wep128-auto") == 0) {
            // WEP128
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_WEP;
            //authtype: 0-open, 1-shared, 2-auto
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=5", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
            sprintf(tmpbuf, "wepdkeyid=%d", pwifi->wep_default_key);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey1=%s", pwifi->wep_key1);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey2=%s", pwifi->wep_key2);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey3=%s", pwifi->wep_key3);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            sprintf(tmpbuf, "wepkey4=%s", pwifi->wep_key4);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
        else if (strcmp(pwifi->security, "wpa2-psk-aes") == 0) {
            // WPA2-PSK, AES
            auth = WSC_AUTH_WPA2PSK;
            enc = WSC_ENCRYPT_AES;
            mixedmode = 0;
            wsc_wpa2_cipher = 2;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa2_cipher=8", NULL_STR);
            sprintf(tmpbuf, "passphrase=%s", pwifi->passphrase);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
        }
    }
    //P2P
    else if (strcmp(pwifi->mode, "p2pdev") == 0) {
        if (pwifi->p2pmode==P2P_DEVICE) {
            is_client = 1;

            sprintf(tmpbuf, "opmode=0x%X", (WIFI_P2P_SUPPORT|WIFI_STATION_STATE));
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);

            sprintf(tmpbuf, "setp2pmode=%d", P2P_DEVICE);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", tmpbuf, NULL_STR);

            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "dwellch=1",NULL_STR);   /*listen channel, should be 1 or 6 or11 */
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "intent=6",NULL_STR);   /*intent value (0~15) ,the intent of as GO */
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "opch=1", NULL_STR);   /*operation channel of GO, if use auto channel don't care this parameter*/
            sprintf(tmpbuf, "devname=%s", pwifi->device_name);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", tmpbuf, NULL_STR);/*device name; must same with wps's device name parameter*/
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "setwscmethod=188", NULL_STR); //0x188 Display,PushButton,Keypad

            /*p2p note ; p2p GO's SSID must in the form "DIRECT-xy-anystring"  xy is random*/
            sprintf(tmpbuf, "ssid=%s", pwifi->ssid);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "channel=1", NULL_STR);

            // No security
            auth = WSC_AUTH_OPEN;
            enc = WSC_ENCRYPT_NONE;
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "802_1x=0", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "use40M=0", NULL_STR);
        }
    }
    else if (strcmp(pwifi->mode, "p2pgo") == 0) {
            is_client = 0;

            /*parameter for wscd*/
            auth = WSC_AUTH_WPA2PSK;
            enc = WSC_ENCRYPT_AES;
            mixedmode = 0;
            wsc_wpa2_cipher = 2;
            /*parameter for wscd*/

            sprintf(tmpbuf, "opmode=0x%X", (WIFI_P2P_SUPPORT|WIFI_AP_STATE));
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);

            /*p2p note ; take care!! All p2pcmd is valid after interface's opmode include WIFI_P2P_SUPPORT */
            sprintf(tmpbuf, "setp2pmode=%d", P2P_TMP_GO);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", tmpbuf, NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "dwellch=1",NULL_STR);   /*listen channel, should be 1 or 6 or11 */
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "intent=6",NULL_STR);   /*intent value (0~15) ,the intent of as GO */
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "opch=1", NULL_STR);   /*operation channel of GO, if use auto channel don't care this parameter*/
            sprintf(tmpbuf, "devname=%s", pwifi->device_name);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", tmpbuf, NULL_STR);/*device name; must same with wps's device name parameter*/
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "p2pcmd", "setwscmethod=188", NULL_STR); //0x188 Display,PushButton,Keypad

            /*p2p note ; p2p GO's SSID must in the form "DIRECT-xy-anystring"  xy is random*/
            sprintf(tmpbuf, "ssid=%s", pwifi->ssid);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "channel=1", NULL_STR);

            /*p2p note ; p2p GO/GC  must use WPA2AES security*/
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "authtype=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "encmode=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "psk_enable=2", NULL_STR);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", "wpa2_cipher=8", NULL_STR);
            sprintf(tmpbuf, "passphrase=%s", pwifi->passphrase);
            RunSystemCmd(NULL_FILE, "iwpriv", "wlan0", "set_mib", tmpbuf, NULL_STR);
    }
    else {
        return -1;
    }


    //Do not up "wlan0" here. It would be up by interface_config() later.
    //interface_up("wlan0");

    //WPS
    memset(&gWpscfg, 0, sizeof(struct wps_config));
    pcfg = &gWpscfg;
    //--------------------------------------------------------------------
    strcpy(pcfg->wlan_interface_name, "wlan0");
    //strcpy(pcfg->lan_interface_name, "eth0");
    strcpy(pcfg->lan_interface_name, "wlan0");

    //--------------------------------------------------------------------
    if (is_client != 1) //AP_MODE=0, CLIENT_MODE=1
        pcfg->start = 1;

    //WSC_DEBUG("wlan mode=%d\n",is_client);
    if (is_client == 1) {
        intVal = MODE_CLIENT_UNCONFIG;
    }
    else {
        if (!pwifi->is_configured)
            intVal = MODE_AP_UNCONFIG;
        else
            intVal = MODE_AP_PROXY_REGISTRAR;
    }
    pcfg->mode = intVal;
    //WSC_DEBUG("wsc mode=%d\n",pcfg->mode);

    pcfg->upnp = 0;

    //pcfg->config_method = (CONFIG_METHOD_VIRTUAL_PIN |  CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC );
    pcfg->config_method = 0x188; //(CONFIG_METHOD_KEYPAD | CONFIG_METHOD_DISPLAY | CONFIG_METHOD_PBC);

    /*assigned auth support list 2.0*/
    pcfg->auth_type_flags = (WSC_AUTH_OPEN |   WSC_AUTH_WPA2PSK);

    /*assigned Encry support list 2.0*/
    pcfg->encrypt_type_flags =
        (WSC_ENCRYPT_NONE |  WSC_ENCRYPT_AES );

    //auth_type, MIB_WLAN_WSC_AUTH
    pcfg->auth_type_flash = auth;

    //encrypt_type, MIB_WLAN_WSC_ENC
    pcfg->encrypt_type_flash = enc;
    if (enc == WSC_ENCRYPT_WEP)
        is_wep = 1;

    pcfg->connect_type = 1; //CONNECT_TYPE_BSS
    pcfg->manual_config = 0;

    //--------------------------------------------------------------------
    if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
        pcfg->wep_transmit_key = pwifi->wep_default_key + 1;
        strcpy(pcfg->network_key, pwifi->wep_key1);
        strcpy(pcfg->wep_key2, pwifi->wep_key2);
        strcpy(pcfg->wep_key3, pwifi->wep_key3);
        strcpy(pcfg->wep_key4, pwifi->wep_key4);
    }
    else {
        strcpy(pcfg->network_key, pwifi->passphrase);
    }
    pcfg->network_key_len = strlen(pcfg->network_key);

    strcpy(pcfg->SSID, pwifi->ssid);

    //strcpy(pcfg->pin_code, "12345670");
    strcpy(pcfg->pin_code, pwifi->wsc_pin);

    if (pwifi->channel > 14)
        intVal = 2;
    else
        intVal = 1;
    pcfg->rf_band = intVal;

    pcfg->config_by_ext_reg = pwifi->config_by_ext_reg;
    DBG_IND("pcfg->config_by_ext_reg  %d\r\n",pcfg->config_by_ext_reg);
    /*for detial mixed mode info*/
    //  mixed issue
    intVal=0;
    if (mixedmode){ // mixed mode (WPA+WPA2)
        if(wsc_wpa_cipher==1){
            intVal |= WSC_WPA_TKIP;
        }else if(wsc_wpa_cipher==2){
            intVal |= WSC_WPA_AES;
        }else if(wsc_wpa_cipher==3){
            intVal |= (WSC_WPA_TKIP | WSC_WPA_AES);
        }
        if(wsc_wpa2_cipher==1){
            intVal |= WSC_WPA2_TKIP;
        }else if(wsc_wpa2_cipher==2){
            intVal |= WSC_WPA2_AES;
        }else if(wsc_wpa2_cipher==3){
            intVal |= (WSC_WPA2_TKIP | WSC_WPA2_AES);
        }
    }
    pcfg->mixedmode = intVal;
    /*for detial mixed mode info*/

    //wscd.conf
    strcpy(pcfg->device_name, pwifi->device_name);
    strcpy(pcfg->manufacturer, "Realtek Semiconductor Corp.");
    strcpy(pcfg->manufacturerURL, "http://www.realtek.com/");
    strcpy(pcfg->model_URL, "http://www.realtek.com/");
    strcpy(pcfg->model_name, "RTL8xxx");
    strcpy(pcfg->model_num, "RTK_ECOS");
    strcpy(pcfg->serial_num, "123456789012347");
    strcpy(pcfg->modelDescription, "WLAN Access Point");
    strcpy(pcfg->ssid_prefix, "RTKAP_");

    apply_wps_config_to_wscd(pcfg);

    return 0;
}
cyg_thread_info tinfo;
static int UINet_get_thread_info_by_name(char *name, cyg_thread_info *pinfo)
{
    cyg_handle_t thandle = 0;
    cyg_uint16 id = 0;

    while (cyg_thread_get_next(&thandle, &id)) {
        cyg_thread_get_info(thandle, id, pinfo);
        //printf("ID: %04x name: %10s pri: %d\n",
                //  pinfo->id, pinfo->name ? pinfo->name:"----", pinfo->set_pri);
        if (strcmp(name, pinfo->name)==0)
            return 1;
    }
    return 0;
}


void UINet_WifiSettings(wifi_settings *pwifi,UINT32 mode,UINT32 security)
{
    if (mode == NET_AP_MODE) {
        strcpy(pwifi->mode,"ap");
        if(currentInfo.strSSID[0] == 0)
        {
#if (MAC_APPEN_SSID==ENABLE)
        sprintf(pwifi->ssid,"%s%02x%02x%02x%02x%02x%02x",gSSID, gMacAddr[0], gMacAddr[1], gMacAddr[2],
            gMacAddr[3], gMacAddr[4], gMacAddr[5]);
#else
        sprintf(pwifi->ssid,"%s",gSSID);
#endif
        }
        else
        {
            sprintf(pwifi->ssid,"%s",currentInfo.strSSID);
        }

        strcpy(pwifi->device_name, "Wireless AP");
    }

    else if(mode == NET_STATION_MODE){
        strcpy(pwifi->mode,"sta");
        sprintf(pwifi->ssid,"%s",gSSID);
        strcpy(pwifi->device_name, "Wireless STA");
    }
    else if (mode == NET_P2P_DEV_MODE){
        pwifi->p2pmode = P2P_DEVICE;
        strcpy(pwifi->mode,"p2pdev");
        /*p2p note ; p2p GO's SSID must in the form "DIRECT-xy-anystring"  xy is random*/
        strcpy(pwifi->ssid, "DIRECT-de-12345678");
        strcpy(pwifi->device_name, "p2p-dev");
    }
    else if (mode == NET_P2P_GO_MODE){
        pwifi->p2pmode = P2P_TMP_GO;
        strcpy(pwifi->mode,"p2pgo");
        /*p2p note ; p2p GO's SSID must in the form "DIRECT-xy-anystring"  xy is random*/
        strcpy(pwifi->ssid, "DIRECT-go-12345678");
        pwifi->is_configured = 1;            // let wscd under configured mode
        strcpy(pwifi->device_name, "p2p-go");
    }
    else{
        DBG_ERR("unknown mode %d\r\n",mode);
    }

    if(security == NET_AUTH_NONE)
    {
        strcpy(pwifi->security,"none");
    }
    else if(security == NET_AUTH_WPA2)
    {
        strcpy(pwifi->security,"wpa2-psk-aes");
    }
    else if(security == NET_AUTH_WPA)
    {
        strcpy(pwifi->security,"wpa-psk-tkip");
    }
    else
    {
        DBG_ERR("unknown security %d\r\n",security);
    }

    pwifi->channel = gChannel;

    strcpy(pwifi->passphrase, gPASSPHRASE);

    pwifi->wep_default_key = 0;
    if (strcmp(pwifi->security, "wep64-auto") == 0) {
        strcpy(pwifi->wep_key1, "1234567890");
        strcpy(pwifi->wep_key2, "1234567890");
        strcpy(pwifi->wep_key3, "1234567890");
        strcpy(pwifi->wep_key4, "1234567890");
    }
    else {
        strcpy(pwifi->wep_key1, "12345678901234567890123456");
        strcpy(pwifi->wep_key2, "12345678901234567890123456");
        strcpy(pwifi->wep_key3, "12345678901234567890123456");
        strcpy(pwifi->wep_key4, "12345678901234567890123456");
    }
}

void UINet_WifiP2PReInit(void)
{
    wifi_settings *pwifi = &wifiConfig;

    CHKPNT;
    UINet_WifiConfig(pwifi);

    if (strcmp(pwifi->mode, "p2pgo") == 0) {
        if (pwifi->p2pmode==P2P_TMP_GO) {
            delete_interface_addr("wlan0");
            memcpy((void *)gCurrIP, EXAM_NET_IP_WLAN0, NET_IP_MAX_LEN);
            interface_config("wlan0", gCurrIP, "255.255.255.0");
            DBG_IND(" setip=%s -\r\n", gCurrIP);
            UINet_DHCP_Start(FALSE);
        }
    }
    else if (strcmp(pwifi->mode, "p2pdev") == 0) {
        if (pwifi->p2pmode==P2P_DEVICE) {
            delete_interface_addr("wlan0");
            interface_up("wlan0");
        }
        else if (pwifi->p2pmode==P2P_CLIENT) {
            delete_interface_addr("wlan0");
            interface_up("wlan0");
        }
    }

    if (UINet_get_thread_info_by_name("wscd", &tinfo)) {
        if (!pwifi->wlan_disable && !pwifi->wps_disable)
        {
             wsc_reinit(); //trigger wsc to reinit
             DBG_IND("wsc_reinit\r\n");
        }
        else
        {
            wsc_stop_service(); //trigger wsc to stop service
            DBG_IND("wsc_stop_service\r\n");
        }
    }
    else {
            DBG_IND("wsc_stop_service\r\n");
        if (!pwifi->wlan_disable && !pwifi->wps_disable)
            create_wscd();
    }

}

void UINet_WifiBack2Dev(void)
{
    wifi_settings *pwifi=&wifiConfig;
    CHKPNT;
    UINet_WifiSettings(pwifi,NET_P2P_DEV_MODE,NET_AUTH_NONE);
    Ux_PostEvent(NVTEVT_EXE_WIFI_REINIT,0);

}
INT32 UINet_WifiInit(UINT32 mode,UINT32 security)
{
    UINT32 result=0;
    static BOOL bInit = FALSE;
    wifi_settings *pwifi=&wifiConfig;

    DBG_IND("set wifi %d  %d \r\n",mode,security);

    memset(pwifi,0,sizeof(wifi_settings));

    if(!bInit)
    {
        result = wifi_init_wlan0_netdev(0);
        if (result!=0 )
        {
            DBG_ERR("wifi init fail %d\r\n",result);
            return E_SYS;
        }
        else
        {
            DBG_IND("wifi init succesful \r\n");
        }
        bInit = TRUE;
    }

    //register sta status cb
    register_wifi_sta_status_cb(WIFI_IFNAME, UINet_StationStatus_CB);

    //register link status cb
    register_wifi_link_status_cb(WIFI_IFNAME, UINet_Link2APStatus_CB);
    register_wsc_event_cb(UINet_WSCEvent_CB);
    register_wsc_flash_param_cb(UINet_WSCFlashParam_CB);
#if defined(__ECOS) && (_WIFI_MODULE_ == _WIFI_MODULE_RTL8189_)
    register_p2p_event_indicate_cb("wlan0", UINet_P2PEvent_CB);
#endif

    if (wifi_get_wlan0_efuse_mac(gMacAddr) < 0)
    {
        DBG_IND("wifi_get_wlan0_efuse_mac() fail. Use hardcoded mac.\r\n");
        set_mac_address("wlan0", "\x00\x00\x00\x81\x89\xe5");
    }
    else
    {
        unsigned char zero[] = {0, 0, 0, 0, 0, 0};
        DBG_IND("wlan0 efuse mac [%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
        gMacAddr[0], gMacAddr[1], gMacAddr[2],
        gMacAddr[3], gMacAddr[4], gMacAddr[5]);
        //sanity check
        if (!memcmp(gMacAddr, zero, MACADDRLEN) || IS_MCAST((unsigned char *)gMacAddr))
        {
            DBG_ERR("efuse mac format is invalid. Use hardcoded mac.\r\n");
            set_mac_address("wlan0", "\x00\x00\x00\x81\x89\xe5");
            sprintf(gMacAddr,"%c%c%c%c%c%c",0,0, 0,0x81,0x89,0xe5);
            DBG_ERR("default wlan0 efuse mac [%02x:%02x:%02x:%02x:%02x:%02x]\r\n",
            gMacAddr[0], gMacAddr[1], gMacAddr[2],
            gMacAddr[3], gMacAddr[4], gMacAddr[5]);
        }
        else
        {
            set_mac_address("wlan0", gMacAddr);
        }
    }

    UINet_WifiSettings(pwifi,mode,security);

    generate_pin_code(pwifi->wsc_pin);

    UINet_WifiConfig(pwifi);

    #if 0
    create_wscd();
    DBG_IND("create_wscd \r\n");
    #else
    if (UINet_get_thread_info_by_name("wscd", &tinfo))
    {
        wsc_reinit(); //trigger wsc to reinit
        DBG_IND("wsc_reinit\r\n");
    }
    else
    {
        create_wscd();
        DBG_IND("create_wscd\r\n");
    }
    #endif
    if ((mode == NET_AP_MODE)|| (mode == NET_P2P_GO_MODE))
    {
        memcpy((void *)gCurrIP, EXAM_NET_IP_WLAN0, NET_IP_MAX_LEN);
        interface_config("wlan0", gCurrIP, "255.255.255.0");
        DBG_IND(" setip=%s -\r\n", gCurrIP);
        result = UINet_DHCP_Start(FALSE);
    }
    else if (mode == NET_STATION_MODE)
    {
        // Set loop delay time of hash algorithm.
        wifi_set_passwd_hash_delay(10);
        DBG_IND("^Bhash delay time:%d ms\r\n", wifi_get_passwd_hash_delay());

        delete_interface_addr("wlan0");
        interface_up("wlan0");
    }
    else if (mode == NET_P2P_DEV_MODE)
    {
        delete_interface_addr("wlan0");
        interface_up("wlan0");
    }
    else
    {
       DBG_ERR("mode %d unknown\r\n",mode);
    }

    DBG_IND("Default IP=%s\r\n",gCurrIP);


    if(result==NET_DHCP_RET_OK)
    {
        result=UINet_HFSInit();
    }

    return result;
}

INT32 UINet_WifiUnInit(UINT32 mode)
{
    UINT32 ret = 0;

    delete_interface_addr("wlan0");

    ret = Dhcp_Server_Stop(FALSE);

    ret = Dhcp_Client_Stop();

    wsc_stop_service(); //trigger wsc to stop service

    ret = interface_down("wlan0");

    return ret;
}

void UINet_print_bss_desc(struct bss_desc *pBss)
{
    char ssidbuf[40], tmp2Buf[20], tmp3Buf[20];

    //BSSID
    DBG_DUMP("%02x:%02x:%02x:%02x:%02x:%02x",
        pBss->bssid[0], pBss->bssid[1], pBss->bssid[2],
        pBss->bssid[3], pBss->bssid[4], pBss->bssid[5]);

    //channel
    DBG_DUMP("  %3d", pBss->channel);

    //band
    tmp2Buf[0] = 0;
    if (pBss->network & WIRELESS_11N) {
        strcat(tmp2Buf, "N");
    }
    if (pBss->network & WIRELESS_11G) {
        strcat(tmp2Buf, "G");
    }
    if (pBss->network & WIRELESS_11B) {
        strcat(tmp2Buf, "B");
    }
    DBG_DUMP("%5s", tmp2Buf);

    //rssi
    DBG_DUMP(" %3d", pBss->rssi);

    //signal quality
    DBG_DUMP("   %3d ", pBss->sq);

    //Security
    memset(tmp3Buf,0x00,sizeof(tmp3Buf));

    // refer to get_security_info() for details
    if ((pBss->capability & cPrivacy) == 0) {
        strcpy(tmp2Buf, "");
        strcpy(tmp3Buf, "None");
    }
    else {
        if (pBss->t_stamp[0] == 0) {
            strcpy(tmp2Buf, "");
            strcpy(tmp3Buf, "WEP");
        }
        else {
            int wpa_exist = 0, idx = 0;
            if (pBss->t_stamp[0] & 0x0000ffff) {
                idx = sprintf(tmp2Buf,"%s","WPA");
                if (((pBss->t_stamp[0] & 0x0000f000) >> 12) == 0x4)
                    idx += sprintf(tmp2Buf+idx,"%s","-PSK");
                else if(((pBss->t_stamp[0] & 0x0000f000) >> 12) == 0x2)
                    idx += sprintf(tmp2Buf+idx,"%s","-1X");
                wpa_exist = 1;

                if (((pBss->t_stamp[0] & 0x00000f00) >> 8) == 0x5)
                    sprintf(tmp3Buf,"%s","AES/TKIP");
                else if (((pBss->t_stamp[0] & 0x00000f00) >> 8) == 0x4)
                    sprintf(tmp3Buf,"%s","AES");
                else if (((pBss->t_stamp[0] & 0x00000f00) >> 8) == 0x1)
                    sprintf(tmp3Buf,"%s","TKIP");
            }
            if (pBss->t_stamp[0] & 0xffff0000) {
                if (wpa_exist)
                    idx += sprintf(tmp2Buf+idx,"%s","/");
                idx += sprintf(tmp2Buf+idx,"%s","WPA2");
                if (((pBss->t_stamp[0] & 0xf0000000) >> 28) == 0x4)
                    idx += sprintf(tmp2Buf+idx,"%s","-PSK");
                else if (((pBss->t_stamp[0] & 0xf0000000) >> 28) == 0x2)
                    idx += sprintf(tmp2Buf+idx,"%s","-1X");

                if (((pBss->t_stamp[0] & 0x0f000000) >> 24) == 0x5)
                    sprintf(tmp3Buf,"%s","AES/TKIP");
                else if (((pBss->t_stamp[0] & 0x0f000000) >> 24) == 0x4)
                    sprintf(tmp3Buf,"%s","AES");
                else if (((pBss->t_stamp[0] & 0x0f000000) >> 24) == 0x1)
                    sprintf(tmp3Buf,"%s","TKIP");
            }
        }
    }
    DBG_DUMP("%-10s ", tmp3Buf);
    DBG_DUMP("%-16s ", tmp2Buf);

    //SSID
    memcpy(ssidbuf, pBss->ssid, pBss->ssidlen);
    ssidbuf[pBss->ssidlen] = '\0';
    DBG_DUMP("%s\r\n", ssidbuf);
}

void UINet_DumpAllSSID(SS_STATUS_Tp pStatus)
{
    int i;
    struct bss_desc *pBss;

    if(!pStatus)
    {
        DBG_ERR("no pStatus \r\n");
        return ;
    }

    DBG_DUMP("pStatus->number %d\r\n",pStatus->number);

    //success
    DBG_DUMP(" #      BSSID          ch band rssi  sq  enc        auth             SSID\r\n");
    for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
        pBss = &pStatus->bssdb[i];
        DBG_DUMP("%2d ", i);
        UINet_print_bss_desc(pBss);
    }
    //free(pStatus);
    pStatus = NULL;

}
void UINet_SiteSurvey(char *ifname,SS_STATUS_Tp pStatus)
{
    int status;
    unsigned char res;
    int wait_time;

    //below is copied from formWlSiteSurvey()
    {
        // issue scan request
        wait_time = 0;
        while (1) {
            // ==== modified by GANTOE for site survey 2008/12/26 ====
            switch(getWlSiteSurveyRequest(ifname, &status))
            {
                case -2:
                    DBG_ERR("Auto scan running!!please wait...\n");
                    goto ss_err;
                    break;
                case -1:
                    DBG_ERR("Site-survey request failed!\n");
                    goto ss_err;
                    break;
                default:
                    break;
            }

            if (status != 0) {  // not ready
                if (wait_time++ > 5) {
                    DBG_ERR("scan request timeout!\n");
                    goto ss_err;
                }
                //sleep(1);
                Delay_DelayMs(1000);
            }
            else
                break;
        }

        // wait until scan completely
        wait_time = 0;
        while (1) {
            res = 1;    // only request request status
            if (getWlSiteSurveyResult(ifname, (SS_STATUS_Tp)&res) < 0 ) {
                DBG_ERR("Read site-survey status failed!");
                goto ss_err;
            }
            if (res == 0xff) {   // in progress
                if (wait_time++ > 10)
                {
                    DBG_ERR("scan timeout!");
                    goto ss_err;
                }
                //sleep(1);
                Delay_DelayMs(1000);
            }
            else
                break;
        }
    }

    pStatus->number = 0; // request BSS DB
    if (getWlSiteSurveyResult(ifname, pStatus) < 0 ) {
        DBG_ERR("Read site-survey status failed!\n");
        //free(pStatus);
        pStatus = NULL;
        goto ss_err;
    }

    return;

ss_err:
    //failed
    //free(pStatus);
    pStatus = NULL;
}

void UINet_P2PDump(SS_STATUS_Tp pStatus)
{
    struct bss_desc *pBss;
    int i;

    DBG_DUMP("pStatus->number=%d\r\n",pStatus->number);
    DBG_DUMP(" # Channel Device address    Role WSCMethod Device Name\r\n");
    for (i=0; i<pStatus->number && pStatus->number!=0xff; i++) {
        pBss = &pStatus->bssdb[i];
        DBG_DUMP("%2d %3d     ", i, pBss->channel);
        DBG_DUMP("%02x:%02x:%02x:%02x:%02x:%02x ",
            pBss->p2paddress[0], pBss->p2paddress[1], pBss->p2paddress[2],
            pBss->p2paddress[3], pBss->p2paddress[4], pBss->p2paddress[5]);
        if(pBss->p2prole == 2) //2: device
            DBG_DUMP("DEV");
        else if(pBss->p2prole == 1) // 1:GO
            DBG_DUMP("GO ");
        else
            DBG_DUMP("???");
        DBG_DUMP("  0x%04x", pBss->p2pwscconfig);
        DBG_DUMP("     %s\r\n", pBss->p2pdevname);
    }
}
void UINet_P2PScan(char *ifname,SS_STATUS_Tp pStatus)
{
#if defined(__ECOS) && (_WIFI_MODULE_ == _WIFI_MODULE_RTL8189_)
    int status;
    unsigned char res;
    int wait_time;

    // issue scan request
    wait_time = 0;
    while (1) {
        switch (getWlP2PScanRequest(ifname, &status)) {
        case -2:
            DBG_ERR("Auto scan running!!please wait...\r\n");
            goto ss_err;
            break;
        case -1:
            DBG_ERR("Site-survey request failed!\r\n");
            goto ss_err;
            break;
        default:
            break;
        }

        if (status != 0) { // not ready
            if (wait_time++ > 5) {
                DBG_ERR("scan request timeout!\r\n");
                goto ss_err;
            }
            //sleep(1);
            Delay_DelayMs(1000);
        }
        else
            break;
    }

    // wait until scan completely
    wait_time = 0;
    while (1) {
        res = 1;
        if (getWlP2PScanResult(ifname, (SS_STATUS_Tp)&res) < 0) {
            DBG_ERR("Read site-survey status failed!\r\n");
            goto ss_err;
        }

        if (res == 0xff) { // in progress
            if (wait_time++ > 20) {
                DBG_ERR("scan timeout!\r\n");
                goto ss_err;
            }
            //sleep(1);
            Delay_DelayMs(1000);
        }
        else
            break;
    }

    // get scan result
    pStatus->number = 0; // request BSS DB
    if ( getWlP2PScanResult(ifname, pStatus) < 0 ) {
        DBG_ERR("Read site-survey status failed!\r\n");
        //free(pStatus);
        pStatus = NULL;
        return;
    }

ss_err:
    pStatus = NULL;
#endif
}

UINT32 UINet_P2PConnect(SS_STATUS_Tp pStatus,UINT32 idx)
{
    UINT32 result=0;
#if defined(__ECOS) && (_WIFI_MODULE_ == _WIFI_MODULE_RTL8189_)
    if ((idx < 0) || (idx >= pStatus->number)) {
        DBG_ERR("wifi p2pconnect <0~%d>\r\n", (pStatus->number-1));
    }
    else {
        P2P_PROVISION_COMM_T p2pProvisionComm;

        //p2pProvisionComm.wsc_config_method = CONFIG_METHOD_DISPLAY;
        //p2pProvisionComm.wsc_config_method = CONFIG_METHOD_KEYPAD;
        p2pProvisionComm.wsc_config_method = CONFIG_METHOD_PBC;
        p2pProvisionComm.channel = pStatus->bssdb[idx].channel;
        memcpy(p2pProvisionComm.dev_address, pStatus->bssdb[idx].p2paddress, 6);
        result = sendP2PProvisionCommInfo("wlan0", &p2pProvisionComm);
        if ( result< 0) {
            DBG_ERR("SendP2PProvisionCommInfo fail\r\n");
        }
    }
#endif
    return result;
}
#elif defined(__ECOS) && (_NETWORK_ == _NETWORK_SPI_EVB_ETHERNET_)
static NET_IP_PAIR gExamNetIP[NET_IP_PAIR_MAX_CNT] =
{
    //eth0
    {EXAM_NET_IP_ETH0,EXAM_NET_NETMASK_ETH0,EXAM_NET_BRAODCAST_ETH0,EXAM_NET_GATEWAY_ETH0,EXAM_NET_SRVIP_ETH0, 0},
    //eth1
    {EXAM_NET_IP_ETH1,EXAM_NET_NETMASK_ETH1,EXAM_NET_BRAODCAST_ETH1,EXAM_NET_GATEWAY_ETH1,EXAM_NET_SRVIP_ETH1, 1},
    //wlan0
    {EXAM_NET_IP_WLAN0,EXAM_NET_NETMASK_WLAN0,EXAM_NET_BRAODCAST_WLAN0,EXAM_NET_GATEWAY_WLAN0,EXAM_NET_SRVIP_WLAN0, 2}
};

INT32 UINet_LanInit(UINT32 mode)
{
    NET_DHCP_RET ret =0;

    DBG_IND("[SetupObj] set Lan %d\r\n",mode);

    if(mode == NET_AP_MODE) //ap mode
    {
        ret=Dhcp_Init_Network_With_IP(gExamNetIP, NET_IP_PAIR_MAX_CNT);
        if (NET_DHCP_RET_OK == ret)
        {
            ret=Dhcp_Server_Start(gNvtDhcpHostNameSrv);
            if(NET_DHCP_RET_OK !=ret )
            {
                DBG_ERR("AP fail %d\r\n",ret);
            }
        }
        else
        {
            DBG_ERR("eth0 fail %d\r\n",ret);
        }
    }
    else if(mode == NET_STATION_MODE) //station mode
    {
        //start DHCP client
        unsigned int gotIP = 0;
        ret=Dhcp_Client_Start(gNvtDhcpHostNameCli, Net_DhcpCliCb, TRUE);
        if (NET_DHCP_RET_OK != ret)
        {
            DBG_ERR("dhcp cli fail\r\n");
            return E_SYS;
        }
        else
        {
            Dhcp_Client_Get_IP(&gotIP);
            sprintf(gCurrIP,"%d.%d.%d.%d",(gotIP&0xFF), (gotIP>>8)&0xFF,(gotIP>>16)&0xFF,(gotIP>>24)&0xFF );
            DBG_IND("^R:::: ip=0x%x %s\r\n",gotIP,gCurrIP);

            #if 0
            debug_err(("%x \r\n",gotIP));
            debug_err(("%x \r\n",(gotIP&0xFF) ));
            debug_err(("%x \r\n",(gotIP>>8)&0xFF ));
            debug_err(("%x \r\n",(gotIP>>16)&0xFF ));
            debug_err(("%x \r\n",(gotIP>>24)&0xFF ));
            #endif

        }
    }
    else
    {
        ret = E_SYS;
        DBG_ERR("not support mode %d\r\n",mode);
    }

    if(ret==NET_DHCP_RET_OK)
    {
        ret=UINet_HFSInit();
    }

    return ret;
}



INT32 UINet_LanUnInit(UINT32 mode)
{
    UINT32 ret = 0;
    if(mode == NET_AP_MODE) //ap mode
        ret =Dhcp_Server_Stop(FALSE);
    else
        ret =Dhcp_Client_Stop();
    return ret;
}
void UINet_RemoveUser(void)
{
}
void UINet_AddACLTable(void)
{
}
void UINet_ClearACLTable(void)
{
}
void UINet_SiteSurvey(char *ifname,SS_STATUS_Tp pStatus)
{
}
void UINet_DumpAllSSID(SS_STATUS_Tp pStatus)
{
}
UINT32 UINet_DHCP_Start(UINT32 isClient)
{
    return 0;
}
void UINet_P2PDump(SS_STATUS_Tp pStatus)
{
}
void UINet_P2PScan(char *ifname,SS_STATUS_Tp pStatus)
{
}
UINT32 UINet_P2PConnect(SS_STATUS_Tp pStatus,UINT32 idx)
{
    return 0;
}
void UINet_WifiBack2Dev(void)
{
}
void UINet_WifiP2PReInit(void)
{
}
#endif

INT32 UINet_HFSInit(void)
{
    int err;

    cyg_FileSys_RegisterCB();
    err = mount( "", MOUNT_FS_ROOT, "nvtfs" );
    if( err < 0 ) DBG_ERR("mount fs fail %d\r\n",err);
    //hfs start
    {
        cyg_hfs_open_obj  hfsObj={0};
        // register callback function of get XML data.
        hfsObj.getCustomData = WifiCmd_GetData;
        // noify some status of HTTP server
        hfsObj.notify = UINet_HFSNotifyStatus;
        // if need to check user & password
        hfsObj.check_pwd = NULL;//xExamHfs_CheckPasswd;
        // set port number
        hfsObj.portNum = 80;
        // set thread priority
        hfsObj.threadPriority = 20;
        // set socket buffer
        hfsObj.sockbufSize = 10240;//10K slow down get file speed
        // set query callback
        hfsObj.clientQueryCb = WifiCmd_getQueryData;
        hfsObj.uploadResultCb = UINet_HFSUploadResultCb;
        hfsObj.deleteResultCb = UINet_HFSDeleteResultCb;
        // set query string
        strcpy(hfsObj.clientQueryStr, "query_devinfo");
        // set HFS root dir path
        strcpy(hfsObj.rootdir, MOUNT_FS_ROOT);


        // start HFS
        cyg_hfs_open(&hfsObj);

    }

    WifiAppCmd_init();
    UserSocket_Open();
    #if (WIFI_FTP_FUNC==ENABLE)
    start_ftpserver();
    #endif

    return err;
}

INT32 UINet_HFSUnInit(void)
{
    cyg_hfs_close();
    #if (WIFI_FTP_FUNC==ENABLE)
    stop_ftpserver();
    #endif

    umount(MOUNT_FS_ROOT);

    return E_OK;
}

#if (WIFI_FTP_FUNC==ENABLE)
////////////////////////////////////FTP server ///////////////////////////////////

#include <network.h>
#include <net/ftpd.h>

#define STACK_SIZE (32*1024 + 0x1000)
static char stackFtpd[STACK_SIZE];
static cyg_thread thread_dataFtpd;
static cyg_handle_t thread_handleFtpd;

static ftp_server_t gFtpServer;

static int ftpCheckPasswd(const char * username, const char * passwd)
{
    if (strncmp(username, "ftpuser", strlen("ftpuser")) == 0)
    {
        if (strncmp(passwd, "12345", strlen("12345")) == 0)
        {
            // return 0 indicates user accepted
            return 0;
        }
    }
    else if (strncmp(username, "nvtuser", strlen("nvtuser")) == 0)
    {
        if (strncmp(passwd, "nvt123", strlen("nvt123")) == 0)
        {
            // return 0 indicates user accepted
            return 0;
        }
    }

    // return 1 indicates user rejected
    return 1;
}


static void ftpd_thread(cyg_addrword_t p)
{
    int sts;

    memset(&gFtpServer, 0, sizeof(gFtpServer));
    gFtpServer.allow_guest = 0;     // allow anonymous login
    gFtpServer.check_pwd = ftpCheckPasswd;
    gFtpServer.max_nr_of_clients = 4;
    strcpy(gFtpServer.chrootdir, MOUNT_FS_ROOT);
    sts = ftpd_server(&gFtpServer);
    if (sts != 0)
    {
        DBG_IND("%s: result is %d\r\n", __func__, sts);
    }
    DBG_IND("ftpd ended\r\n");

    cyg_thread_exit();
}

void start_ftpserver(void)
{
    // assume a file system is already mounted on /sdcard/ftp_home

/* Create a main thread, so we can run the scheduler and have time 'pass' */
    cyg_thread_create(10,                // Priority - just a number
                    ftpd_thread,          // entry
                    0,                 // entry parameter
                    "FTP test",        // Name
                    &stackFtpd[0],         // Stack
                    STACK_SIZE,        // Size
                    &thread_handleFtpd,    // Handle
                    &thread_dataFtpd       // Thread data structure
                    );
    cyg_thread_resume(thread_handleFtpd);  /* Start it */
}


static ftp_server_t gFtpServer;
static cyg_handle_t thread_handleFtpd;

void stop_ftpserver(void)
{
    // assume gFtpServer and thread_handleFtpd are those used to start ftp server

ftpd_server_stop(&gFtpServer);
cyg_thread_delete(thread_handleFtpd);
}
#endif


#endif
