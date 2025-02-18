/**
  ******************************************************************************
  * @file    usart.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _USART_H__
#define _USART_H__

#include "stm32f4xx.h"

#define USART_OK        0
#define USART_ERR       -1

typedef enum usart_parity_e {
  USART_PARITY_NOT,
  USART_PARITY_EVEN,
  USART_PARITY_ODD,
}usart_parity_t;

typedef enum usart_event_e {
  USART_EVENT_CTS,
  USART_EVENT_LBD,
  USART_EVENT_TXE,
  USART_EVENT_TC,
  USART_EVENT_RXNE,
  USART_EVENT_IDLE,
  USART_EVENT_ORE,
  USART_EVENT_NF,
  USART_EVENT_FE,
  USART_EVENT_PE,
  USART_EVENT_NE = USART_EVENT_NF,
}usart_event_t;

typedef void (*usart_callback_t)(USART_TypeDef *,enum usart_event_e, void *);

int usart_set_gpio(USART_TypeDef *pusart, GPIO_TypeDef *pgpio, uint32_t pin);

int usart_set_rx_enable(USART_TypeDef *pusart, uint32_t enable);
int usart_set_tx_enable(USART_TypeDef *pusart, uint32_t enable);

int usart_set_dma_rx_enable(USART_TypeDef *pusart, uint32_t enable);
int usart_set_dma_tx_enable(USART_TypeDef *pusart, uint32_t enable);

int usart_set_parity(USART_TypeDef *pusart, enum usart_parity_e parity);
int usart_set_stopbits(USART_TypeDef *pusart, uint32_t stopbits);
int usart_set_databits(USART_TypeDef *pusart, uint32_t databits);

int usart_set_rcc_gpio_enable(USART_TypeDef *pusart, uint32_t enable);
int usart_set_rcc_usart_enable(USART_TypeDef *pusart, uint32_t enable);

int usart_set_baudrate(USART_TypeDef *pusart, uint32_t baudrate);

int usart_set_enable(USART_TypeDef *pusart, uint32_t enable);

int usart_send( USART_TypeDef *pusart, void *buffer, int len );
int usart_send_wait( USART_TypeDef *pusart );
int usart_send_state( USART_TypeDef *pusart );

int usart_event_arg( USART_TypeDef *pusart, void *arg);
int usart_event( USART_TypeDef *pusart, enum usart_event_e, usart_callback_t);

#endif /* _USART_H__ */


