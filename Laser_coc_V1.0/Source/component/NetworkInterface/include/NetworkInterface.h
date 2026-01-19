/*
 * FreeRTOS+TCP <DEVELOPMENT BRANCH>
 * Copyright (C) 2022 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

#include "FreeRTOS_IP.h"

/* INTERNAL API FUNCTIONS. */

/* Since there are multiple interfaces, there are multiple versions
 * of the following functions.
 * These are now declared static in NetworkInterface.c and their addresses
 * are stored in a struct NetworkInterfaceDescriptor_t.
 *
 *  BaseType_t xNetworkInterfaceInitialise( struct xNetworkInterface *pxInterface );
 *  BaseType_t xGetPhyLinkStatus( struct xNetworkInterface *pxInterface );
 */

/* The following function is defined only when BufferAllocation_1.c is linked in the project. */
void vNetworkInterfaceAllocateRAMToBuffers( NetworkBufferDescriptor_t pxNetworkBuffers[ ipconfigNUM_NETWORK_BUFFER_DESCRIPTORS ] );

BaseType_t xGetPhyLinkStatus( struct xNetworkInterface * pxInterface );

#define MAC_IS_MULTICAST( pucMACAddressBytes )    ( ( pucMACAddressBytes[ 0 ] & 1U ) != 0U )
#define MAC_IS_UNICAST( pucMACAddressBytes )      ( ( pucMACAddressBytes[ 0 ] & 1U ) == 0U )

//ÒÔÌ«Íø¶¨Òå
/*
ETH_MDIO -------------------------> PA2
ETH_MDC --------------------------> PC1
ETH_RMII_REF_CLK------------------> PA1
ETH_RMII_CRS_DV ------------------> PA7
ETH_RMII_RXD0 --------------------> PC4
ETH_RMII_RXD1 --------------------> PC5
ETH_RMII_TX_EN -------------------> PB11
ETH_RMII_TXD0 --------------------> PB12
ETH_RMII_TXD1 --------------------> PB13
*/

#define ETH_MDC_Port	  	GPIOC 
#define ETH_MDC_Pin       	GPIO_PIN_1

#define ETH_MDIO_Port	  	GPIOA 
#define ETH_MDIO_Pin      	GPIO_PIN_2

#define ETH_RXD0_Port	  	GPIOC
#define ETH_RXD0_Pin     	GPIO_PIN_4

#define ETH_RXD1_Port     	GPIOC
#define ETH_RXD1_Pin     	GPIO_PIN_5

#define ETH_TX_EN_Port		GPIOB
#define ETH_TX_EN_Pin       GPIO_PIN_11

#define ETH_TXD0_Port		GPIOB
#define ETH_TXD0_Pin        GPIO_PIN_12

#define ETH_TXD1_Port		GPIOB
#define ETH_TXD1_Pin        GPIO_PIN_13

#define ETH_REF_CLK_Port	GPIOA
#define ETH_REF_CLK_Pin     GPIO_PIN_1

#define ETH_CRS_DV_Port		GPIOA
#define ETH_CRS_DV_Pin      GPIO_PIN_7

//MII
#define ETH_RX_CLK_Port		GPIOA
#define ETH_RX_CLK_Pin      GPIO_PIN_1

#define ETH_RX_ER_Port		GPIOB
#define ETH_RX_ER_Pin     	GPIO_PIN_10

#define ETH_RX_DV_Port		GPIOA
#define ETH_RX_DV_Pin     	GPIO_PIN_7

#define ETH_RXD2_Port		GPIOB
#define ETH_RXD2_Pin     	GPIO_PIN_0

#define ETH_RXD3_Port		GPIOB
#define ETH_RXD3_Pin    	GPIO_PIN_1

#define ETH_TX_CLK_Port		GPIOC
#define ETH_TX_CLK_Pin      GPIO_PIN_3

#define ETH_TXD2_Port		GPIOC
#define ETH_TXD2_Pin     	GPIO_PIN_2

#define ETH_TXD3_Port		GPIOB
#define ETH_TXD3_Pin     	GPIO_PIN_8

#define ETH_COL_Port		GPIOA
#define ETH_COL_Pin     	GPIO_PIN_3

#define ETH_CRS_Port		GPIOA
#define ETH_CRS_Pin     	GPIO_PIN_0
/* *INDENT-OFF* */
#ifdef __cplusplus
    } /* extern "C" */
#endif
/* *INDENT-ON* */

#endif /* NETWORK_INTERFACE_H */
