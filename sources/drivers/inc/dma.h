/**
  ******************************************************************************
  * @file    dma.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _DMA_H__
#define _DMA_H__

#include "stm32f4xx.h"

#define DMA_ERR -1
#define DMA_OK  0

typedef enum dma_dir_e {
  DMA_DIR_TO_PERIPHERY,
  DMA_DIR_FROM_PERIPHERY,
  DMA_DIR_FROM_MEMORY,
} dma_dir_t;

typedef enum dma_event_e {
  DMA_TRANSFER_COMPLETE,
  DMA_TRANSFER_HALF,
  DMA_TRANSFER_ERROR,
} dma_event_t;

typedef enum dma_fifo_threshold_e {
  DMA_FIFO_THRESHOLD_1_4,
  DMA_FIFO_THRESHOLD_1_2,
  DMA_FIFO_THRESHOLD_3_4,
  DMA_FIFO_THRESHOLD_FULL,
}dma_fifo_threshold_t;

typedef void (*dma_callback_t)( DMA_TypeDef *dma, DMA_Stream_TypeDef *stream, enum dma_event_e, void *);

int dma_init();
int dma_circular(int dma, int enable);
int dma_alloc(void *addr, enum dma_dir_e dir); // Выделяем поток для периферии

int dma_fifo(int dma, int enable);
int dma_fifo_threshold(int dma, enum dma_fifo_threshold_e threshold);

int dma_transfer(int dma, void *addr, uint32_t len);

int dma_event(int dma, dma_event_t event, dma_callback_t cb);
int dma_event_arg(int dma, void *arg);
int dma_event_clear(int dma, dma_event_t event);
int dma_event_clear_all_flags(int dma);

int dma_counter(int dma);

int dma_transfer_stop(int dma);
int dma_transfer_wait(int dma);
int dma_transfer_state(int dma);

#endif /* _DMA_H__ */


