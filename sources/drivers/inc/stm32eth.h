/**
  ******************************************************************************
  * @file    stm32eth.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    06.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _STM32ETH_H__
#define _STM32ETH_H__

#include <stdint.h>

/**
 * @brief stm32eth initialization
 *
 * @retval none.
 *
 */
void stm32eth_init( void );

/**
 * @brief stm32eth deinitialization
 *
 * @retval none.
 *
 */
void stm32eth_deinit( void );

/****** Ethernet MAC Event *****/
#define ETH_MAC_EVENT_RX_FRAME      (1UL << 0)  ///< Frame Received
#define ETH_MAC_EVENT_TX_FRAME      (1UL << 1)  ///< Frame Transmitted
#define ETH_MAC_EVENT_WAKEUP        (1UL << 2)  ///< Wake-up (on Magic Packet)
#define ETH_MAC_EVENT_TIMER_ALARM   (1UL << 3)  ///< Timer Alarm

typedef void (*stm32eth_callback_t)(uint32_t event);


void stm32eth_set_callback(stm32eth_callback_t);

void stm32eth_send(void *buffer, unsigned long len);
void stm32eth_recv(void *buffer, unsigned long len);

uint32_t stm32eth_currient_buffer(void **address);
void stm32eth_next_buffer();

void stm32eth_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);
void stm32eth_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data);



/**
 * @brief Ethernet descriptors and buffers size
 */
#define ETHERNET_DESCRIPTOR_RECEIVE_COUNT       5
#define ETHERNET_DESCRIPTOR_TRANSMIT_COUNT      5

#define ETHERNET_PACKET_SIZE                    1522


#endif /* _STM32ETH_H__ */


