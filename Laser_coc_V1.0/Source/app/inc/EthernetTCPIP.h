#ifndef ETHERNET_TCPIP_H
#define ETHERNET_TCPIP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"

/* Default MAC address configuration. */
#define configMAC_ADDR0		0x00
#define configMAC_ADDR1		0x11
#define configMAC_ADDR2		0x22
#define configMAC_ADDR3		0x33
#define configMAC_ADDR4		0x44
#define configMAC_ADDR5		0x46

/* Default IP address configuration.  Used in case ipconfigUSE_DHCP is set to 0,
or ipconfigUSE_DHCP is set to 1 but a DHCP server cannot be contacted. */
#define configIP_ADDR0		192
#define configIP_ADDR1		168
#define configIP_ADDR2		2
#define configIP_ADDR3		100

/* Default gateway IP address configuration.  Used in case ipconfigUSE_DHCP is
set to 0, or ipconfigUSE_DHCP is set to 1 but a DHCP server cannot be contacted. */
#define configGATEWAY_ADDR0	192
#define configGATEWAY_ADDR1	168
#define configGATEWAY_ADDR2	2
#define configGATEWAY_ADDR3	1

/* Default DNS server configuration.  OpenDNS addresses are 208.67.222.222 and
208.67.220.220.  Used in ipconfigUSE_DHCP is set to 0, or ipconfigUSE_DHCP is set
to 1 but a DHCP server cannot be contacted.*/
#define configDNS_SERVER_ADDR0 	208
#define configDNS_SERVER_ADDR1 	67
#define configDNS_SERVER_ADDR2 	222
#define configDNS_SERVER_ADDR3 	222

/* Default netmask configuration.  Used in case ipconfigUSE_DHCP is set to 0,
or ipconfigUSE_DHCP is set to 1 but a DHCP server cannot be contacted. */
#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		255
#define configNET_MASK3		0


UBaseType_t uxRand(void);
void EthernetTCPInit(uint8_t addr);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* NET_WORK_CONFIG_H */

