/**
 * WinnerMicro W800 Wifi library for Lua
 *
 * WangWei
 * 2022.04.01
 */

#include <string.h>
#include <stdio.h>
#include "lua/lua.h"

#include "lua/lauxlib.h"
#include "lua/lualib.h"

#include "wm_include.h"
#include "lwip/netif.h"
#include "wm_netif.h"
#include "wm_sockets.h"

#define APSTA_TASK_SIZE             256
#define APSTA_TASK_PRIO				38
#define APSTA_QUEUE_SIZE				4

#define APSTA_DEMO_CMD_SOFTAP_CREATE        0x0
#define APSTA_DEMO_CMD_STA_JOIN_NET         0x1
#define APSTA_DEMO_CMD_SOFTAP_CLOSE         0x2

static tls_os_task_t wifi_tsk_handle = NULL;
static OS_STK apstaTaskStk[APSTA_TASK_SIZE]; 
static tls_os_queue_t *apstaTaskQueue = NULL;
static char apSsid[32] = {0};
static char apPass[64] = {0};
static char staSsid[32] = {0};
static char staPass[64] = {0};

static int soft_ap(char *apssid, char *appwd);

static void apsta_demo_client_event(u8 *mac, enum tls_wifi_client_event_type event)
{
    printf("client %M is %s\r\n", mac, event ? "offline" : "online");
}

static void apsta_demo_net_status(u8 status)
{
    struct netif *netif = tls_get_netif();

    switch(status)
    {
    case NETIF_WIFI_JOIN_FAILED:
        printf("sta join net failed\n");
        break;
    case NETIF_WIFI_DISCONNECTED:
        printf("sta net disconnected\n");
        //tls_os_queue_send(apstaTaskQueue, (void *)APSTA_DEMO_CMD_SOFTAP_CLOSE, 0);
        break;
    case NETIF_IP_NET_UP:
        printf("\nsta ip: %v\n", netif->ip_addr.addr);
        break;
    case NETIF_WIFI_SOFTAP_FAILED:
        printf("softap create failed\n");
        break;
    case NETIF_WIFI_SOFTAP_CLOSED:
        printf("softap closed\n");
        break;
    case NETIF_IP_NET2_UP:
        printf("\nsoftap ip: %v\n", netif->next->ip_addr.addr);
        break;
    default:
        break;
    }
}

static void connect_wifi(char *ssid, char *pwd)
{
    int ret = tls_wifi_connect((u8 *)ssid, strlen(ssid), (u8 *)pwd, strlen(pwd));
    if (WM_SUCCESS == ret) {
        printf("\nplease wait connect net......\n");
    } else {
        printf("\napsta connect net failed, please check configure......\n");
	}
}

static void apsta_task(void *p)
{
    int ret;
    void *msg;

    for( ; ; )
    {
        ret = tls_os_queue_receive(apstaTaskQueue, (void **)&msg, 0, 0);
        if (!ret)
        {
            switch((u32)msg)
            {
            case APSTA_DEMO_CMD_STA_JOIN_NET:
                connect_wifi(staSsid, staPass);
                break;
            case APSTA_DEMO_CMD_SOFTAP_CREATE:
                soft_ap(apSsid, apPass);
                break;
            case APSTA_DEMO_CMD_SOFTAP_CLOSE:
                tls_wifi_softap_destroy();
                break;
            default:
                break;
            }
        }
    }
}

static int wm_wifi_init(lua_State *L) 
{
	u8 ssid_set = 0;
    u8 wireless_protocol = 0;
    struct tls_param_ip *ip_param = NULL;
	
	tls_wifi_disconnect();
    tls_wifi_softap_destroy();
    tls_wifi_set_oneshot_flag(0);
	tls_param_get(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, TRUE);
    if (TLS_PARAM_IEEE80211_INFRA != wireless_protocol)
    {
        wireless_protocol = TLS_PARAM_IEEE80211_INFRA;
        tls_param_set(TLS_PARAM_ID_WPROTOCOL, (void *) &wireless_protocol, FALSE);
    }

    tls_param_get(TLS_PARAM_ID_BRDSSID, (void *)&ssid_set, (bool)0);
    if (0 == ssid_set)
    {
        ssid_set = 1;
        tls_param_set(TLS_PARAM_ID_BRDSSID, (void *)&ssid_set, (bool)1); /*set flag to broadcast BSSID*/
    }

    ip_param = tls_mem_alloc(sizeof(struct tls_param_ip));
    if (ip_param)
    {
        tls_param_get(TLS_PARAM_ID_IP, ip_param, FALSE);
        ip_param->dhcp_enable = TRUE;
        tls_param_set(TLS_PARAM_ID_IP, ip_param, FALSE);

        tls_param_get(TLS_PARAM_ID_SOFTAP_IP, ip_param, FALSE);
        ip_param->dhcp_enable = TRUE;
        tls_param_set(TLS_PARAM_ID_SOFTAP_IP, ip_param, FALSE);

        tls_mem_free(ip_param);
    }

    tls_netif_add_status_event(apsta_demo_net_status);
    tls_wifi_softap_client_event_register(apsta_demo_client_event);
	
	if (wifi_tsk_handle != NULL) {
		printf("wifi was initialized\n");
		return 0;
	}
	printf("wifi task initializing ...\n");
	tls_os_task_create(&wifi_tsk_handle, NULL, apsta_task,
                           (void *)0, (void *)apstaTaskStk,/* task's stack start address */
                           APSTA_TASK_SIZE * sizeof(u32), /* task's stack size, unit:byte */
                           APSTA_TASK_PRIO, 0);

    tls_os_queue_create(&apstaTaskQueue, APSTA_QUEUE_SIZE);
	
	return 0;
}

//STA mode, connect to wifi. Params: ssid, password. return 0: OK, other: error
static int wm_sta(lua_State *L) 
{
	const char *ssid = luaL_optstring(L, 1, NULL);
	const char *pwd = luaL_optstring(L, 2, NULL);
	if (ssid == NULL || pwd == NULL) {
		printf("Not input SSID/password\n");
		lua_pushinteger(L, 1);
		return 1;
	}
	printf("\nConnect to: %s, pwd: %s\n", ssid, pwd);
	
	memset(staSsid, 0, 32);
	memset(staPass, 0, 64);
	memcpy(staSsid, ssid, strlen(ssid));
	memcpy(staPass, pwd, strlen(pwd));
	tls_os_queue_send(apstaTaskQueue, (void *)APSTA_DEMO_CMD_STA_JOIN_NET, 0);
	lua_pushinteger(L, 0);
	return 1;
}

static int soft_ap(char *apssid, char *appwd)
{
	//printf("\nAP: %s, pwd: %s\n", apssid, appwd);
	struct tls_softap_info_t apinfo;
    struct tls_ip_info_t ipinfo;
    u8 ret = 0;

    memset(&apinfo, 0, sizeof(apinfo));
    memset(&ipinfo, 0, sizeof(ipinfo));

    u8 *ssid = (u8 *)"w600_apsta_demo";
    u8 ssid_len = strlen("w600_apsta_demo");

    if (apssid) {
        ssid_len = strlen(apssid);
        memcpy(apinfo.ssid, apssid, ssid_len);
        apinfo.ssid[ssid_len] = '\0';
    } else {
        memcpy(apinfo.ssid, ssid, ssid_len);
        apinfo.ssid[ssid_len] = '\0';
    }

    apinfo.encrypt = strlen(appwd) ? IEEE80211_ENCRYT_CCMP_WPA2 : IEEE80211_ENCRYT_NONE;
    apinfo.channel = 11; /*channel*/
    apinfo.keyinfo.format = 1; /*format:0,hex, 1,ascii*/
    apinfo.keyinfo.index = 1;  /*wep index*/
    apinfo.keyinfo.key_len = strlen(appwd); /*key length*/
    memcpy(apinfo.keyinfo.key, appwd, strlen(appwd));

    /*ip information:ip address,mask, DNS name*/
    ipinfo.ip_addr[0] = 192;
    ipinfo.ip_addr[1] = 168;
    ipinfo.ip_addr[2] = 8;
    ipinfo.ip_addr[3] = 1;
    ipinfo.netmask[0] = 255;
    ipinfo.netmask[1] = 255;
    ipinfo.netmask[2] = 255;
    ipinfo.netmask[3] = 0;
    memcpy(ipinfo.dnsname, "local.wm", sizeof("local.wm"));

    ret = tls_wifi_softap_create((struct tls_softap_info_t * )&apinfo, (struct tls_ip_info_t * )&ipinfo);
    //printf("\nap create %s ! \n", (ret == WM_SUCCESS) ? "Successfully" : "Error");

	return 0;
}

//AP mode
static int wm_ap(lua_State *L) 
{
	const char *apssid = luaL_optstring(L, 1, NULL);
	const char *appwd = luaL_optstring(L, 2, NULL);
	if (apssid == NULL || appwd == NULL) {
		printf("Not input SSID/password\n");
		lua_pushinteger(L, 1);
		return 1;
	}
	
	memset(apSsid, 0, 32);
	memset(apPass, 0, 64);
	memcpy(apSsid, apssid, strlen(apssid));
	memcpy(apPass, appwd, strlen(appwd));
	tls_os_queue_send(apstaTaskQueue, (void *)APSTA_DEMO_CMD_SOFTAP_CREATE, 0);
	lua_pushinteger(L, 0);
	return 1;
}

static const luaL_Reg wifilib[] = {
	{"init", wm_wifi_init},
	{"sta", wm_sta},
	{"ap", wm_ap},
	{NULL, NULL},
};

// set global var
static void set_lua_global(lua_State *L)
{
}

LUAMOD_API int luaopen_wm_wifi (lua_State *L) 
{
	luaL_newlib(L, wifilib);
	set_lua_global(L);
	return 1;
}