/* Standard includes. */
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"

#include "EthernetTCPIP.h"

/* stm32 eth phy includes. */
#include "stm32h7xx_hal.h"

#define mainHOST_NAME					"RTOSDemo"
#define mainDEVICE_NICK_NAME			"stm32"

extern void vStartSimpleMQTTDemo( void );

/* Default MAC address configuration. */
uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };

static uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
static const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
static const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

void EthernetTCPInit(uint8_t addr)
{
	ucIPAddress[3] = ucIPAddress[3] + addr;
	ucMACAddress[5] = ucMACAddress[5] + addr;
	
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
}

UBaseType_t uxRand(void)
{
	return ((((UBaseType_t)rand()<<16)&0xFFFF0000UL)|(((UBaseType_t)rand())&0x0000FFFFUL));
}

const char *pcApplicationHostnameHook( void )
{
	/* Assign the name "rtosdemo" to this network node.  This function will be
	called during the DHCP: the machine will be registered with an IP address
	plus this name. */
	return mainHOST_NAME;
}

/*-----------------------------------------------------------*/

/*
 * Supply a random number to FreeRTOS+TCP stack.
 * THIS IS ONLY A DUMMY IMPLEMENTATION THAT RETURNS A PSEUDO RANDOM NUMBER
 * SO IS NOT INTENDED FOR USE IN PRODUCTION SYSTEMS.
 */
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *( pulNumber ) = uxRand();
    return pdTRUE;
}

/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
 * events are only received if implemented in the MAC driver. */
#if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
    void vApplicationIPNetworkEventHook_Multi( eIPCallbackEvent_t eNetworkEvent,
                                               struct xNetworkEndPoint * pxEndPoint )
#else
    void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
#endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
{
    uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
    char cBuffer[ 16 ];
    static BaseType_t xTasksAlreadyCreated = pdFALSE;

    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the IP stack if they have not already been
         * created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /* See the comments above the definitions of these pre-processor
             * macros at the top of this file for a description of the individual
             * demo tasks. */

            #if ( mainCREATE_TCP_ECHO_TASKS_SINGLE == 1 )
            {
                vStartTCPEchoClientTasks_SingleTasks( mainECHO_CLIENT_TASK_STACK_SIZE, mainECHO_CLIENT_TASK_PRIORITY );
            }
            #endif /* mainCREATE_TCP_ECHO_TASKS_SINGLE */
					
						/* Tasks that use the TCP/IP stack can be created here. */
						FreeRTOS_printf( ( "---------STARTING MQTT DEMO---------\r\n" ) );
						vStartSimpleMQTTDemo();

            xTasksAlreadyCreated = pdTRUE;
        }

        /* Print out the network configuration, which may have come from a DHCP
         * server. */
        #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
            FreeRTOS_GetEndPointConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress, pxNetworkEndPoints );
        #else
            FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
        #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
        FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
        FreeRTOS_printf( ( "\r\n\r\nIP Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
        FreeRTOS_printf( ( "Subnet Mask: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
        FreeRTOS_printf( ( "Gateway Address: %s\r\n", cBuffer ) );

        FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
        FreeRTOS_printf( ( "DNS Server Address: %s\r\n\r\n\r\n", cBuffer ) );
    }
    else
    {
        FreeRTOS_printf( ( "Application idle hook network down\n" ) );
    }
}
/*-----------------------------------------------------------*/




/*-----------------------------------------------------------*/
#if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 )

    #if defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 )
        BaseType_t xApplicationDNSQueryHook_Multi( struct xNetworkEndPoint * pxEndPoint,
                                                   const char * pcName )
    #else
        BaseType_t xApplicationDNSQueryHook( const char * pcName )
    #endif /* defined( ipconfigIPv4_BACKWARD_COMPATIBLE ) && ( ipconfigIPv4_BACKWARD_COMPATIBLE == 0 ) */
    {
        BaseType_t xReturn;

        /* Determine if a name lookup is for this node.  Two names are given
         * to this node: that returned by pcApplicationHostnameHook() and that set
         * by mainDEVICE_NICK_NAME. */
        if( strcasecmp( pcName, pcApplicationHostnameHook() ) == 0 )
        {
            xReturn = pdPASS;
        }
        else if( strcasecmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
        {
            xReturn = pdPASS;
        }
        else
        {
            xReturn = pdFAIL;
        }

        return xReturn;
    }

#endif /* if ( ipconfigUSE_LLMNR != 0 ) || ( ipconfigUSE_NBNS != 0 ) */
/*-----------------------------------------------------------*/
		
/* Called automatically when a reply to an outgoing ping is received. */
void vApplicationPingReplyHook( ePingReplyStatus_t eStatus,
                                uint16_t usIdentifier )
{
    static const uint8_t * pcSuccess = ( uint8_t * ) "Ping reply received - ";
    static const uint8_t * pcInvalidChecksum = ( uint8_t * ) "Ping reply received with invalid checksum - ";
    static const uint8_t * pcInvalidData = ( uint8_t * ) "Ping reply received with invalid data - ";
    static uint8_t cMessage[ 50 ];


    switch( eStatus )
    {
        case eSuccess:
            FreeRTOS_debug_printf( ( ( char * ) pcSuccess ) );
            break;

        case eInvalidChecksum:
            FreeRTOS_debug_printf( ( ( char * ) pcInvalidChecksum ) );
            break;

        case eInvalidData:
            FreeRTOS_debug_printf( ( ( char * ) pcInvalidData ) );
            break;

        default:

            /* It is not possible to get here as all enums have their own
             * case. */
            break;
    }

    sprintf( ( char * ) cMessage, "identifier %d\r\n", ( int ) usIdentifier );
    FreeRTOS_debug_printf( ( ( char * ) cMessage ) );
}
/*-----------------------------------------------------------*/

/*
 * Callback that provides the inputs necessary to generate a randomized TCP
 * Initial Sequence Number per RFC 6528.  THIS IS ONLY A DUMMY IMPLEMENTATION
 * THAT RETURNS A PSEUDO RANDOM NUMBER SO IS NOT INTENDED FOR USE IN PRODUCTION
 * SYSTEMS.
 */
extern uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    uint32_t ulRandomNumber;

    ( void ) ulSourceAddress;
    ( void ) usSourcePort;
    ( void ) ulDestinationAddress;
    ( void ) usDestinationPort;

    ( void ) xApplicationGetRandomNumber( &ulRandomNumber );

    return ulRandomNumber;
}
/*-----------------------------------------------------------*/



void vLoggingPrintf( const char * pcFormat,
                     ... )
{
}
