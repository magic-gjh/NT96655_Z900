#ifndef _WIFI_UTILITY_H
#define _WIFI_UTILITY_H

#define MAX_BSS_DESC    64

enum NETWORK_TYPE {
    WIRELESS_11B = 1,
    WIRELESS_11G = 2,
    WIRELESS_11A = 4,
    WIRELESS_11N = 8,
    WIRELESS_11AC = 64
};

typedef enum _Capability {
    cESS        = 0x01,
    cIBSS       = 0x02,
    cPollable       = 0x04,
    cPollReq        = 0x01,
    cPrivacy        = 0x10,
    cShortPreamble  = 0x20,
} Capability;

typedef struct _sitesurvey_status {
    unsigned char number;
    unsigned char pad[3];
    //BssDscr bssdb[MAX_BSS_DESC];
    struct bss_desc bssdb[MAX_BSS_DESC];
} SS_STATUS_T, *SS_STATUS_Tp;

enum wifi_state {
    WIFI_STATION_STATE  =   0x00000008,
    WIFI_AP_STATE       =   0x00000010,
    WIFI_P2P_SUPPORT    =   0x08000000
};
enum p2p_role_more{
    P2P_DEVICE=1,
    P2P_PRE_CLIENT=2,
    P2P_CLIENT=3,
    P2P_PRE_GO=4,    // after GO nego , we are GO and proceed WSC exchange
    P2P_TMP_GO=5     // after GO nego , we are GO and proceed WSC exchange is done
};
/* Any changed here MUST sync with 8192cd_p2p.h */
typedef struct _p2p_provision_comm
{
    unsigned char dev_address[6];
    unsigned short wsc_config_method;
    unsigned char channel;
} P2P_PROVISION_COMM_T, *P2P_PROVISION_COMM_Tp;


extern int getWlSiteSurveyRequest(char *interface, int *pStatus);
extern int getWlSiteSurveyResult(char *interface, SS_STATUS_Tp pStatus );
extern int delete_interface_addr(char *intf);
extern void generate_pin_code(char *pinbuf);
extern void print_wscd_status(int status);
extern int getWlP2PScanRequest(char *interface, int *pStatus);
extern int getWlP2PScanResult(char *interface, SS_STATUS_Tp pStatus);
extern int sendP2PProvisionCommInfo(char *interface, P2P_PROVISION_COMM_Tp pP2PProvisionComm);

#endif
