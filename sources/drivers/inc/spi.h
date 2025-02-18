/**
  ******************************************************************************
  * @file    spi.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    20.04.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _SPI_H__
#define _SPI_H__

#include "stm32f4xx.h"

#define SPI_OK        0
#define SPI_ERR       -1

typedef enum spi_frame_format_e {
  SPI_FRAME_FORMAT_MOTOROLA,
  SPI_FRAME_FORMAT_TI,
}spi_frame_format_t;

typedef enum spi_data_format_e {
  SPI_DATA_FORMAT_8,
  SPI_DATA_FORMAT_16,
}spi_data_format_t;

typedef enum spi_event_e {
  SPI_EVENT_TXE,
  SPI_EVENT_RXNE,
  SPI_EVENT_ERR,
  SPI_EVENT_ERR_OVR,
  SPI_EVENT_ERR_MODF,
  SPI_EVENT_ERR_TI_FRE,
  SPI_EVENT_ERR_UDR,
  SPI_EVENT_ERR_CRC,
}spi_event_t;

typedef void (*spi_callback_t)(SPI_TypeDef *,enum spi_event_e, void *);


int spi_set_gpio(SPI_TypeDef *pspi, GPIO_TypeDef *pgpio, uint32_t pin);

int spi_set_bidi_mode_enable    (SPI_TypeDef *pspi, uint32_t enable);
int spi_set_bidi_output_enable  (SPI_TypeDef *pspi, uint32_t enable);
int spi_set_crc_enable          (SPI_TypeDef *pspi, uint32_t enable);
int spi_set_crcnext             (SPI_TypeDef *pspi, uint32_t set);
int spi_set_rxonly              (SPI_TypeDef *pspi, uint32_t set);
int spi_set_ssi                 (SPI_TypeDef *pspi, uint32_t set);
int spi_set_ssm                 (SPI_TypeDef *pspi, uint32_t set);
int spi_set_lsb_first           (SPI_TypeDef *pspi, uint32_t set);

int spi_set_dma_rx_enable       (SPI_TypeDef *pspi, uint32_t enable);
int spi_set_dma_tx_enable       (SPI_TypeDef *pspi, uint32_t enable);

int spi_set_rcc_gpio_enable     (SPI_TypeDef *pspi, uint32_t enable);
int spi_set_rcc_spi_enable      (SPI_TypeDef *pspi, uint32_t enable);

int spi_set_baudrate_control    (SPI_TypeDef *pspi, uint32_t baudrate);

int spi_set_master              (SPI_TypeDef *pspi, uint32_t set);
int spi_set_cpol                (SPI_TypeDef *pspi, uint32_t set);
int spi_set_cpha                (SPI_TypeDef *pspi, uint32_t set);
int spi_set_enable              (SPI_TypeDef *pspi, uint32_t enable);

int spi_set_data_format         (SPI_TypeDef *pspi, spi_data_format_t format);
int spi_set_frame_format        (SPI_TypeDef *pspi, spi_frame_format_t format);

int spi_send( SPI_TypeDef *pspi, void *buffer, int len );
int spi_send_wait( SPI_TypeDef *pspi );
int spi_send_state( SPI_TypeDef *pspi );

int spi_wait( SPI_TypeDef *pspi );

int spi_event_arg( SPI_TypeDef *pspi, void *arg);
int spi_event( SPI_TypeDef *pspi, enum spi_event_e, spi_callback_t);

#endif /* _SPI_H__ */


