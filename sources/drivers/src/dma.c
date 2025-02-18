/**
  ******************************************************************************
  * @file    dma.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "dma.h"

enum dma_state_index_e {
  DMA1_STREAM0_STATE,
  DMA1_STREAM1_STATE,
  DMA1_STREAM2_STATE,
  DMA1_STREAM3_STATE,
  DMA1_STREAM4_STATE,
  DMA1_STREAM5_STATE,
  DMA1_STREAM6_STATE,
  DMA1_STREAM7_STATE,
  DMA2_STREAM0_STATE,
  DMA2_STREAM1_STATE,
  DMA2_STREAM2_STATE,
  DMA2_STREAM3_STATE,
  DMA2_STREAM4_STATE,
  DMA2_STREAM5_STATE,
  DMA2_STREAM6_STATE,
  DMA2_STREAM7_STATE,
};

static void nullcb(DMA_TypeDef *dma,
                    DMA_Stream_TypeDef *stream,
                    enum dma_event_e event,
                    void *arg){};

struct dma_state_s {
  DMA_TypeDef           *dma;
  DMA_Stream_TypeDef    *stream;
  dma_callback_t        callback_transfer_complete;
  dma_callback_t        callback_transfer_half;
  dma_callback_t        callback_transfer_error;
  uint32_t              allocated;
  void                  *arg;
} static dma_state[] = {
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream0,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream1,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream2,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream3,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream4,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream5,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream6,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA1,
    .stream     = DMA1_Stream7,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  
  
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream0,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream1,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream2,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream3,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream4,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream5,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream6,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
  {
    .dma        = DMA2,
    .stream     = DMA2_Stream7,
    .callback_transfer_complete = nullcb,
    .callback_transfer_half     = nullcb,
    .callback_transfer_error    = nullcb,
    .allocated  = 0,
    .arg        = 0,
  },
};

struct dma_alloc_table_s {
  void                          *periphery;
  void                          *reg;
  enum dma_dir_e                dir;
  enum dma_state_index_e        index;
  uint32_t                      channel;
} static const dma_alloc_table[] = {
 /******************************************************************************
  * USART modules
  *****************************************************************************/
  /* USART1 */
  {
    .periphery  = USART1,
    .reg        = (void*)&USART1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM7_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = USART1,
    .reg        = (void*)&USART1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = USART1,
    .reg        = (void*)&USART1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  /* USART2 */
  {
    .periphery  = USART2,
    .reg        = (void*)&USART2->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM6_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = USART2,
    .reg        = (void*)&USART2->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  /* USART3 */
  {
    .periphery  = USART3,
    .reg        = (void*)&USART3->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM3_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = USART3,
    .reg        = (void*)&USART3->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM4_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = USART3,
    .reg        = (void*)&USART3->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM1_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  /* UART4 */
  {
    .periphery  = UART4,
    .reg        = (void*)&UART4->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM4_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = UART4,
    .reg        = (void*)&UART4->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  /* UART5 */
  {
    .periphery  = UART5,
    .reg        = (void*)&UART5->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM7_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = UART5,
    .reg        = (void*)&UART5->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM0_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  /* USART6 */
  {
    .periphery  = USART6,
    .reg        = (void*)&USART6->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM6_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = USART6,
    .reg        = (void*)&USART6->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM7_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = USART6,
    .reg        = (void*)&USART6->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM1_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = USART6,
    .reg        = (void*)&USART6->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
 /******************************************************************************
  * SPI modules
  *****************************************************************************/
  /* SPI1 */
  {
    .periphery  = SPI1,
    .reg        = (void*)&SPI1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM0_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI1,
    .reg        = (void*)&SPI1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI1,
    .reg        = (void*)&SPI1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM3_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI1,
    .reg        = (void*)&SPI1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  /* SPI2 */
  {
    .periphery  = SPI2,
    .reg        = (void*)&SPI2->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM3_STATE,
    .channel    = 0,
  },
  {
    .periphery  = SPI2,
    .reg        = (void*)&SPI2->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM4_STATE,
    .channel    = 0,
  },
  /* SPI3 */
  {
    .periphery  = SPI3,
    .reg        = (void*)&SPI3->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM2_STATE,
    .channel    = 0,
  },
  {
    .periphery  = SPI3,
    .reg        = (void*)&SPI3->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM5_STATE,
    .channel    = 0,
  },
  {
    .periphery  = SPI3,
    .reg        = (void*)&SPI3->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM0_STATE,
    .channel    = 0,
  },
  {
    .periphery  = SPI3,
    .reg        = (void*)&SPI3->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM7_STATE,
    .channel    = 0,
  },
  /* SPI4 */
  {
    .periphery  = SPI4,
    .reg        = (void*)&SPI4->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM0_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = SPI4,
    .reg        = (void*)&SPI4->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM1_STATE,
    .channel    = DMA_SxCR_CHSEL_2,
  },
  {
    .periphery  = SPI4,
    .reg        = (void*)&SPI4->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM3_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI4,
    .reg        = (void*)&SPI4->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM4_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_0,
  },
  /* SPI5 */
  {
    .periphery  = SPI5,
    .reg        = (void*)&SPI5->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM3_STATE,
    .channel    = DMA_SxCR_CHSEL_1,
  },
  {
    .periphery  = SPI5,
    .reg        = (void*)&SPI5->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM4_STATE,
    .channel    = DMA_SxCR_CHSEL_1,
  },
  {
    .periphery  = SPI5,
    .reg        = (void*)&SPI5->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI5,
    .reg        = (void*)&SPI5->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM6_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  /* SPI6 */
  {
    .periphery  = SPI6,
    .reg        = (void*)&SPI6->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA2_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = SPI6,
    .reg        = (void*)&SPI6->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA2_STREAM6_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
 /******************************************************************************
  * I2C modules
  *****************************************************************************/
  /* I2C1 */
  {
    .periphery  = I2C1,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM6_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = I2C1,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM0_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = I2C1,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM7_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = I2C1,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM5_STATE,
    .channel    = DMA_SxCR_CHSEL_0,
  },
  /* I2C2 */
  {
    .periphery  = I2C2,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM7_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = I2C2,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_2 | DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  /* I2C3 */
  {
    .periphery  = I2C3,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_TO_PERIPHERY,
    .index      = DMA1_STREAM4_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
  {
    .periphery  = I2C3,
    .reg        = (void*)&I2C1->DR,
    .dir        = DMA_DIR_FROM_PERIPHERY,
    .index      = DMA1_STREAM2_STATE,
    .channel    = DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_0,
  },
};

int dma_circular(int dma, int enable)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  if(enable)
  {
    dma_state[dma].stream->CR |= DMA_SxCR_CIRC;
  }
  else
  {
    dma_state[dma].stream->CR &=~DMA_SxCR_CIRC;
  }
  return DMA_OK;
}

int dma_event_clear_all_flags(int dma)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  switch(dma)
  {
    case DMA1_STREAM0_STATE:
      {
        DMA1->LIFCR |= (0x3F<<(0*6));
      }
      break;
    case DMA1_STREAM1_STATE:
      {
        DMA1->LIFCR |= (0x3F<<(1*6));
      }
      break;
    case DMA1_STREAM2_STATE:
      {
        DMA1->LIFCR |= (0x3F<<(0*6+16));
      }
      break;
    case DMA1_STREAM3_STATE:
      {
        DMA1->LIFCR |= (0x3F<<(1*6+16));
      }
      break;
    case DMA1_STREAM4_STATE:
      {
        DMA1->HIFCR |= (0x3F<<(0*6));
      }
      break;
    case DMA1_STREAM5_STATE:
      {
        DMA1->HIFCR |= (0x3F<<(1*6));
      }
      break;
    case DMA1_STREAM6_STATE:
      {
        DMA1->HIFCR |= (0x3F<<(0*6+16));
      }
      break;
    case DMA1_STREAM7_STATE:
      {
        DMA1->HIFCR |= (0x3F<<(1*6+16));
      }
      break;
    case DMA2_STREAM0_STATE:
      {
        DMA2->LIFCR |= (0x3F<<(0*6));
      }
      break;
    case DMA2_STREAM1_STATE:
      {
        DMA2->LIFCR |= (0x3F<<(1*6));
      }
      break;
    case DMA2_STREAM2_STATE:
      {
        DMA2->LIFCR |= (0x3F<<(0*6+16));
      }
      break;
    case DMA2_STREAM3_STATE:
      {
        DMA2->LIFCR |= (0x3F<<(1*6+16));
      }
      break;
    case DMA2_STREAM4_STATE:
      {
        DMA2->HIFCR |= (0x3F<<(0*6));
      }
      break;
    case DMA2_STREAM5_STATE:
      {
        DMA2->HIFCR |= (0x3F<<(1*6));
      }
      break;
    case DMA2_STREAM6_STATE:
      {
        DMA2->HIFCR |= (0x3F<<(0*6+16));
      }
      break;
    case DMA2_STREAM7_STATE:
      {
        DMA2->HIFCR |= (0x3F<<(1*6+16));
      }
      break;
  }
  return DMA_OK;
}

int dma_event_clear(int dma, dma_event_t event)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  switch(event)
  {
    case DMA_TRANSFER_ERROR:
      {
        dma_state[dma].callback_transfer_half = nullcb;
        dma_state[dma].stream->CR &=~(DMA_SxCR_TEIE|DMA_SxCR_DMEIE);
        dma_state[dma].stream->FCR &=~(DMA_SxFCR_FEIE);
      }
      break;
    case DMA_TRANSFER_HALF:
      {
        dma_state[dma].callback_transfer_half = nullcb;
        dma_state[dma].stream->CR &=~DMA_SxCR_HTIE;
      }
      break;
    case DMA_TRANSFER_COMPLETE:
      {
        dma_state[dma].callback_transfer_complete = nullcb;
        dma_state[dma].stream->CR &=~DMA_SxCR_TCIE;
      }
      break;
    default:
      {
        return DMA_ERR;
      }
      break;
  }
  return DMA_OK;
}

int dma_event_arg(int dma, void *arg)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  
  dma_state[dma].arg = arg;
  return DMA_OK;
}

int dma_event(int dma, dma_event_t event, dma_callback_t cb)
{
  if(cb == 0)
    return DMA_ERR;
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  
  switch(event)
  {
    case DMA_TRANSFER_ERROR:
      {
        dma_state[dma].callback_transfer_error = cb;
        dma_state[dma].stream->CR  |= (DMA_SxCR_TEIE|DMA_SxCR_DMEIE);
        dma_state[dma].stream->FCR |= (DMA_SxFCR_FEIE);
      }
      break;
    case DMA_TRANSFER_HALF:
      {
        dma_state[dma].callback_transfer_half = cb;
        dma_state[dma].stream->CR |= DMA_SxCR_HTIE;
      }
      break;
    case DMA_TRANSFER_COMPLETE:
      {
        dma_state[dma].callback_transfer_complete = cb;
        dma_state[dma].stream->CR |= DMA_SxCR_TCIE;
      }
      break;
    default:
      {
        return DMA_ERR;
      }
      break;
  }
  return DMA_OK;
}

static int dma_alloc_nvic(DMA_Stream_TypeDef    *stream)
{
  switch((uint32_t)stream)
  {
    case (uint32_t)DMA1_Stream0:
      {
#define DMA_Stream_IRQn DMA1_Stream0_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream1:
      {
#define DMA_Stream_IRQn DMA1_Stream1_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream2:
      {
#define DMA_Stream_IRQn DMA1_Stream2_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream3:
      {
#define DMA_Stream_IRQn DMA1_Stream3_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream4:
      {
#define DMA_Stream_IRQn DMA1_Stream4_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream5:
      {
#define DMA_Stream_IRQn DMA1_Stream5_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream6:
      {
#define DMA_Stream_IRQn DMA1_Stream6_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream7:
      {
#define DMA_Stream_IRQn DMA1_Stream7_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
      
    case (uint32_t)DMA2_Stream0:
      {
#define DMA_Stream_IRQn DMA2_Stream0_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream1:
      {
#define DMA_Stream_IRQn DMA2_Stream1_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream2:
      {
#define DMA_Stream_IRQn DMA2_Stream2_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream3:
      {
#define DMA_Stream_IRQn DMA2_Stream3_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream4:
      {
#define DMA_Stream_IRQn DMA2_Stream4_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream5:
      {
#define DMA_Stream_IRQn DMA2_Stream5_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream6:
      {
#define DMA_Stream_IRQn DMA2_Stream6_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream7:
      {
#define DMA_Stream_IRQn DMA2_Stream7_IRQn
        NVIC->IP[((uint32_t)(int32_t)DMA_Stream_IRQn)]
          = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    default:
      {
        return DMA_ERR;
      }
      break;
  }
  return DMA_OK;
}

static int dma_dealloc_nvic(DMA_Stream_TypeDef    *stream)
{
  switch((uint32_t)stream)
  {
    case (uint32_t)DMA1_Stream0:
      {
#define DMA_Stream_IRQn DMA1_Stream0_IRQn
        NVIC->ICER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream1:
      {
#define DMA_Stream_IRQn DMA1_Stream1_IRQn
        NVIC->ICER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream2:
      {
#define DMA_Stream_IRQn DMA1_Stream2_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream3:
      {
#define DMA_Stream_IRQn DMA1_Stream3_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream4:
      {
#define DMA_Stream_IRQn DMA1_Stream4_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream5:
      {
#define DMA_Stream_IRQn DMA1_Stream5_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream6:
      {
#define DMA_Stream_IRQn DMA1_Stream6_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA1_Stream7:
      {
#define DMA_Stream_IRQn DMA1_Stream7_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
      
    case (uint32_t)DMA2_Stream0:
      {
#define DMA_Stream_IRQn DMA2_Stream0_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream1:
      {
#define DMA_Stream_IRQn DMA2_Stream1_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream2:
      {
#define DMA_Stream_IRQn DMA2_Stream2_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream3:
      {
#define DMA_Stream_IRQn DMA2_Stream3_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream4:
      {
#define DMA_Stream_IRQn DMA2_Stream4_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream5:
      {
#define DMA_Stream_IRQn DMA2_Stream5_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream6:
      {
#define DMA_Stream_IRQn DMA2_Stream6_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    case (uint32_t)DMA2_Stream7:
      {
#define DMA_Stream_IRQn DMA2_Stream7_IRQn
        NVIC->ISER[(((uint32_t)(int32_t)DMA_Stream_IRQn) >> 5UL)]
          = (uint32_t)(1UL << (((uint32_t)(int32_t)DMA_Stream_IRQn) & 0x1FUL));
#undef DMA_Stream_IRQn
      }
      break;
    default:
      {
        return DMA_ERR;
      }
      break;
  }
  return DMA_OK;
}

void dma_dealloc_nvic_all() {
  dma_dealloc_nvic(DMA1_Stream0);
  dma_dealloc_nvic(DMA1_Stream1);
  dma_dealloc_nvic(DMA1_Stream2);
  dma_dealloc_nvic(DMA1_Stream3);
  dma_dealloc_nvic(DMA1_Stream4);
  dma_dealloc_nvic(DMA1_Stream5);
  dma_dealloc_nvic(DMA1_Stream6);
  dma_dealloc_nvic(DMA1_Stream7);
  
  dma_dealloc_nvic(DMA2_Stream0);
  dma_dealloc_nvic(DMA2_Stream1);
  dma_dealloc_nvic(DMA2_Stream2);
  dma_dealloc_nvic(DMA2_Stream3);
  dma_dealloc_nvic(DMA2_Stream4);
  dma_dealloc_nvic(DMA2_Stream5);
  dma_dealloc_nvic(DMA2_Stream6);
  dma_dealloc_nvic(DMA2_Stream7);
}

int dma_alloc(void *address, enum dma_dir_e dir)
{
  int index;
  int iter;
  /* Если работаем с периферией */
  if(dir == DMA_DIR_TO_PERIPHERY ||
     dir == DMA_DIR_FROM_PERIPHERY)
  {
    for(iter=0;iter<sizeof(dma_alloc_table)/sizeof(struct dma_alloc_table_s);iter++)
    {
      /* Периферия найдена */
      if(dma_alloc_table[iter].periphery == address &&
         dma_alloc_table[iter].dir == dir)
      {
        /* Периферия свободна */
        if(dma_state[dma_alloc_table[iter].index].allocated == 0)
        {
          index = dma_alloc_table[iter].index;
          dma_state[index].allocated = 1;
          
          if(dma_state[index].dma == DMA1)
          {
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
            if(dma_alloc_nvic(dma_state[index].stream) == DMA_ERR)
              return DMA_ERR;
          }
          else if(dma_state[index].dma == DMA2)
          {
            RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
            if(dma_alloc_nvic(dma_state[index].stream) == DMA_ERR)
              return DMA_ERR;
          }
          else return DMA_ERR;
          
          dma_state[index].stream->CR = 0;
          if(dir == DMA_DIR_FROM_PERIPHERY)
          {
            dma_state[index].stream->CR &=~DMA_SxCR_DIR;
          }
          else
          {
            dma_state[index].stream->CR &=~DMA_SxCR_DIR;
            dma_state[index].stream->CR |= DMA_SxCR_DIR_0;
          }
          dma_state[index].stream->CR |=  dma_alloc_table[iter].channel;
          dma_state[index].stream->CR |= (DMA_SxCR_MINC);
          dma_state[index].stream->CR |= (DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
          dma_state[index].stream->FCR &= ~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);
          dma_state[index].stream->FCR |=  (DMA_SxFCR_FEIE);
          dma_state[index].stream->PAR = (uint32_t)dma_alloc_table[iter].reg;
          
          dma_event_clear_all_flags(index);
          return index;
        }
      }
    }
  }
  else if(dir == DMA_DIR_FROM_MEMORY) 
  {
    /* Проверяем на пригодность адресного пространства.
     * Проверяем свободные каналы DMA2 - DMA с доступом к памяти
     * 0x20000000 <= addr < 0x20020000
     * 0x08000000 <= addr < 0x08100000
     * нужно добавить внешнюю память
     */
    if(!(((SRAM1_BASE <= (uint32_t)address) && ((uint32_t)address < (SRAM1_BASE + 0x20000)))) || 
         ((FLASH_BASE <= (uint32_t)address) && ((uint32_t)address < (FLASH_BASE + 0x100000))))
    {
      return DMA_ERR;
    }
  
    for(iter=0;iter<sizeof(dma_alloc_table)/sizeof(struct dma_alloc_table_s);iter++)
    {
      /* Поиск свободных потоков DMA2 */
      if( (dma_state[iter].allocated == 0) &&
          (dma_state[iter].dma == DMA2) )
      {
        dma_state[index].allocated = 1;
        
        /* Clock enabled */
        RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;
        
        /* Memory-to-memory */
        dma_state[index].stream->CR = 0;
        dma_state[index].stream->CR &=~DMA_SxCR_DIR;
        dma_state[index].stream->CR |= DMA_SxCR_DIR_1;
        
        dma_state[index].stream->CR |= (DMA_SxCR_MINC | DMA_SxCR_PINC);
        dma_state[index].stream->CR |= (DMA_SxCR_TEIE | DMA_SxCR_DMEIE);
        dma_state[index].stream->FCR &= ~(DMA_SxFCR_DMDIS | DMA_SxFCR_FTH);
        dma_state[index].stream->FCR |=  (DMA_SxFCR_DMDIS | DMA_SxFCR_FEIE);
        
        /* Source Address */
        dma_state[index].stream->PAR = (uint32_t)dma_alloc_table[iter].reg;
        
        dma_event_clear_all_flags(index);
      }
    }
    return DMA_ERR;
  }
  return DMA_ERR;
}

int dma_fifo(int dma, int enable)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  
  if(enable)
  {
    dma_state[dma].stream->FCR |= DMA_SxFCR_DMDIS;
  }
  else
  {
    dma_state[dma].stream->FCR &=~DMA_SxFCR_DMDIS;
  }
  
  return DMA_OK;
}


int dma_fifo_threshold(int dma, enum dma_fifo_threshold_e threshold)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  dma_state[dma].stream->FCR &=~DMA_SxFCR_FTH;
  switch(threshold)
  {
    
    case DMA_FIFO_THRESHOLD_1_4:
      {
      }
      break;
    case DMA_FIFO_THRESHOLD_1_2:
      {
        dma_state[dma].stream->FCR |= DMA_SxFCR_FTH_0;
      }
      break;
    case DMA_FIFO_THRESHOLD_3_4:
      {
        dma_state[dma].stream->FCR |= DMA_SxFCR_FTH_1;
      }
      break;
    case DMA_FIFO_THRESHOLD_FULL:
      {
        dma_state[dma].stream->FCR |= DMA_SxFCR_FTH_1 | DMA_SxFCR_FTH_0;
      }
      break;
    default:
      {
        return DMA_ERR;
      }
      break;
  }
  return DMA_OK;
}

int dma_transfer_wait(int dma)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  while(dma_state[dma].stream->CR & DMA_SxCR_EN);
  return DMA_OK;
}

int dma_transfer_state(int dma)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  if(dma_state[dma].stream->CR & DMA_SxCR_EN)
  {
    return 1;
  }
  return 0;
}

int dma_counter(int dma)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  
  return dma_state[dma].stream->NDTR;
}

int dma_transfer_stop(int dma)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  
  dma_state[dma].stream->CR &=~DMA_SxCR_EN;
  return DMA_OK;
}

int dma_transfer(int dma, void *addr, uint32_t len)
{
  if(0 > dma || dma > DMA2_STREAM7_STATE)
    return DMA_ERR;
  if(dma_state[dma].allocated == 0)
    return DMA_ERR;
  if(!(((SRAM1_BASE <= (uint32_t)addr) && ((uint32_t)addr+len < (SRAM1_BASE + 0x30000)))) || 
       ((FLASH_BASE <= (uint32_t)addr) && ((uint32_t)addr+len < (FLASH_BASE + 0x100000))))
    return DMA_ERR;
  
  /* От периферии */
  if((dma_state[dma].stream->CR & DMA_SxCR_DIR) == 0)
  {
  }
  /* К периферии */
  else if((dma_state[dma].stream->CR & DMA_SxCR_DIR) == DMA_SxCR_DIR_0)
  {
  }
  /* Память-память */
  else if((dma_state[dma].stream->CR & DMA_SxCR_DIR) == DMA_SxCR_DIR_1)
  {
    if(!((SRAM1_BASE <= (uint32_t)addr) && ((uint32_t)addr+len < (SRAM1_BASE + 0x20000))))
    {
      return DMA_ERR;
    }
    dma_fifo(dma,1);
  }
  else return DMA_ERR;
  
  dma_event_clear_all_flags(dma);
  dma_state[dma].stream->CR &=~DMA_SxCR_EN;
  dma_event_clear_all_flags(dma);
  
  dma_state[dma].stream->NDTR = len;
  dma_state[dma].stream->M0AR = (uint32_t)addr;
  
  
  dma_state[dma].stream->CR |= DMA_SxCR_EN;
  return DMA_OK;
}



/** 
 * DMA1 Stream 0
 * ch0 SPI3
 * ch1 I2C1
 * ch2 TIM4_CH1
 * ch3 I2S3_EXT_RX
 * ch4 UART5_RX
 * ch5 UART8_TX
 * ch6 TIM5_CH3, TIM5_UP
 */
void DMA1_Stream0_IRQHandler()
{
  if(DMA1->LISR & DMA_LISR_TCIF0)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTCIF0;
    dma_state[DMA1_STREAM0_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream0,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM0_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_HTIF0)
  {
    DMA1->LIFCR |= DMA_LIFCR_CHTIF0;
    dma_state[DMA1_STREAM0_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream0,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM0_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_TEIF0)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTEIF0;
    dma_state[DMA1_STREAM0_STATE].callback_transfer_error(DMA1,DMA1_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM0_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_DMEIF0)
  {
    DMA1->LIFCR |= DMA_LIFCR_CDMEIF0;
    dma_state[DMA1_STREAM0_STATE].callback_transfer_error(DMA1,DMA1_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM0_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_FEIF0)
  {
    DMA1->LIFCR |= DMA_LIFCR_CFEIF0;
    dma_state[DMA1_STREAM0_STATE].callback_transfer_error(DMA1,DMA1_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM0_STATE].arg);
  }
}

/**
 * DMA1 Stream 1
 * ch3 TIM2_CH3, TIM2_UP
 * ch4 USART3_RX
 * ch5 UART7_TX
 * ch6 TIM5_CH4, TIM5_TRIG
 * ch7 TIM6_UP
 */
void DMA1_Stream1_IRQHandler()
{
  if(DMA1->LISR & DMA_LISR_TCIF1)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTCIF1;
    dma_state[DMA1_STREAM1_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream1,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM1_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_HTIF1)
  {
    DMA1->LIFCR |= DMA_LIFCR_CHTIF1;
    dma_state[DMA1_STREAM1_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream1,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM1_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_TEIF1)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTEIF1;
    dma_state[DMA1_STREAM1_STATE].callback_transfer_error(DMA1,DMA1_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM1_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_DMEIF1)
  {
    DMA1->LIFCR |= DMA_LIFCR_CDMEIF1;
    dma_state[DMA1_STREAM1_STATE].callback_transfer_error(DMA1,DMA1_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM1_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_FEIF1)
  {
    DMA1->LIFCR |= DMA_LIFCR_CFEIF1;
    dma_state[DMA1_STREAM1_STATE].callback_transfer_error(DMA1,DMA1_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM1_STATE].arg);
  }
}

/**
 * DMA1 Stream 2
 */
void DMA1_Stream2_IRQHandler()
{
  if(DMA1->LISR & DMA_LISR_TCIF2)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTCIF2;
    dma_state[DMA1_STREAM2_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream2,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM2_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_HTIF2)
  {
    DMA1->LIFCR |= DMA_LIFCR_CHTIF2;
    dma_state[DMA1_STREAM2_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream2,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM2_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_TEIF2)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTEIF2;
    dma_state[DMA1_STREAM2_STATE].callback_transfer_error(DMA1,DMA1_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM2_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_DMEIF2)
  {
    DMA1->LIFCR |= DMA_LIFCR_CDMEIF2;
    dma_state[DMA1_STREAM2_STATE].callback_transfer_error(DMA1,DMA1_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM2_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_FEIF2)
  {
    DMA1->LIFCR |= DMA_LIFCR_CFEIF2;
    dma_state[DMA1_STREAM2_STATE].callback_transfer_error(DMA1,DMA1_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM2_STATE].arg);
  }
}

/**
 * DMA1 Stream 3
 */
void DMA1_Stream3_IRQHandler()
{
  if(DMA1->LISR & DMA_LISR_TCIF3)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTCIF3;
    dma_state[DMA1_STREAM3_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream3,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM3_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_HTIF3)
  {
    DMA1->LIFCR |= DMA_LIFCR_CHTIF3;
    dma_state[DMA1_STREAM3_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream3,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM3_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_TEIF3)
  {
    DMA1->LIFCR |= DMA_LIFCR_CTEIF3;
    dma_state[DMA1_STREAM3_STATE].callback_transfer_error(DMA1,DMA1_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM3_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_DMEIF3)
  {
    DMA1->LIFCR |= DMA_LIFCR_CDMEIF3;
    dma_state[DMA1_STREAM3_STATE].callback_transfer_error(DMA1,DMA1_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM3_STATE].arg);
  }

  if(DMA1->LISR & DMA_LISR_FEIF3)
  {
    DMA1->LIFCR |= DMA_LIFCR_CFEIF3;
    dma_state[DMA1_STREAM3_STATE].callback_transfer_error(DMA1,DMA1_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM3_STATE].arg);
  }
}

/**
 * DMA1 Stream 4
 */
void DMA1_Stream4_IRQHandler()
{
  if(DMA1->HISR & DMA_HISR_TCIF4)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTCIF4;
    dma_state[DMA1_STREAM4_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream4,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM4_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_HTIF4)
  {
    DMA1->HIFCR |= DMA_HIFCR_CHTIF4;
    dma_state[DMA1_STREAM4_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream4,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM4_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_TEIF4)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTEIF4;
    dma_state[DMA1_STREAM4_STATE].callback_transfer_error(DMA1,DMA1_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM4_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_DMEIF4)
  {
    DMA1->HIFCR |= DMA_HIFCR_CDMEIF4;
    dma_state[DMA1_STREAM4_STATE].callback_transfer_error(DMA1,DMA1_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM4_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_FEIF4)
  {
    DMA1->HIFCR |= DMA_HIFCR_CFEIF4;
    dma_state[DMA1_STREAM4_STATE].callback_transfer_error(DMA1,DMA1_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM4_STATE].arg);
  }
}

/**
 * DMA1 Stream 5
 */
void DMA1_Stream5_IRQHandler()
{
  if(DMA1->HISR & DMA_HISR_TCIF5)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
    dma_state[DMA1_STREAM5_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream5,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM5_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_HTIF5)
  {
    DMA1->HIFCR |= DMA_HIFCR_CHTIF5;
    dma_state[DMA1_STREAM5_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream5,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM5_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_TEIF5)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTEIF5;
    dma_state[DMA1_STREAM5_STATE].callback_transfer_error(DMA1,DMA1_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM5_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_DMEIF5)
  {
    DMA1->HIFCR |= DMA_HIFCR_CDMEIF5;
    dma_state[DMA1_STREAM5_STATE].callback_transfer_error(DMA1,DMA1_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM5_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_FEIF5)
  {
    DMA1->HIFCR |= DMA_HIFCR_CFEIF5;
    dma_state[DMA1_STREAM5_STATE].callback_transfer_error(DMA1,DMA1_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM5_STATE].arg);
  }
}

/**
 * DMA1 Stream 6
 */
void DMA1_Stream6_IRQHandler()
{
  if(DMA1->HISR & DMA_HISR_TCIF6)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTCIF6;
    dma_state[DMA1_STREAM6_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream6,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM6_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_HTIF6)
  {
    DMA1->HIFCR |= DMA_HIFCR_CHTIF6;
    dma_state[DMA1_STREAM6_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream6,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM6_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_TEIF6)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTEIF6;
    dma_state[DMA1_STREAM6_STATE].callback_transfer_error(DMA1,DMA1_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM6_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_DMEIF6)
  {
    DMA1->HIFCR |= DMA_HIFCR_CDMEIF6;
    dma_state[DMA1_STREAM6_STATE].callback_transfer_error(DMA1,DMA1_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM6_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_FEIF6)
  {
    DMA1->HIFCR |= DMA_HIFCR_CFEIF6;
    dma_state[DMA1_STREAM6_STATE].callback_transfer_error(DMA1,DMA1_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM6_STATE].arg);
  }
}

/**
 * DMA1 Stream 7
 */
void DMA1_Stream7_IRQHandler()
{
  if(DMA1->HISR & DMA_HISR_TCIF7)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTCIF7;
    dma_state[DMA1_STREAM7_STATE]
      .callback_transfer_complete(DMA1,DMA1_Stream7,DMA_TRANSFER_COMPLETE,dma_state[DMA1_STREAM7_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_HTIF7)
  {
    DMA1->HIFCR |= DMA_HIFCR_CHTIF7;
    dma_state[DMA1_STREAM7_STATE]
      .callback_transfer_half(DMA1,DMA1_Stream7,DMA_TRANSFER_HALF,dma_state[DMA1_STREAM7_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_TEIF7)
  {
    DMA1->HIFCR |= DMA_HIFCR_CTEIF7;
    dma_state[DMA1_STREAM7_STATE].callback_transfer_error(DMA1,DMA1_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM7_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_DMEIF7)
  {
    DMA1->HIFCR |= DMA_HIFCR_CDMEIF7;
    dma_state[DMA1_STREAM7_STATE].callback_transfer_error(DMA1,DMA1_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM7_STATE].arg);
  }

  if(DMA1->HISR & DMA_HISR_FEIF7)
  {
    DMA1->HIFCR |= DMA_HIFCR_CFEIF7;
    dma_state[DMA1_STREAM7_STATE].callback_transfer_error(DMA1,DMA1_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA1_STREAM7_STATE].arg);
  }
}

/* DMA2 Handlers */
void DMA2_Stream0_IRQHandler()
{
  if(DMA2->LISR & DMA_LISR_TCIF0)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTCIF0;
    dma_state[DMA2_STREAM0_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream0,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM0_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_HTIF0)
  {
    DMA2->LIFCR |= DMA_LIFCR_CHTIF0;
    dma_state[DMA2_STREAM0_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream0,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM0_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_TEIF0)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTEIF0;
    dma_state[DMA2_STREAM0_STATE].callback_transfer_error(DMA2,DMA2_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM0_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_DMEIF0)
  {
    DMA2->LIFCR |= DMA_LIFCR_CDMEIF0;
    dma_state[DMA2_STREAM0_STATE].callback_transfer_error(DMA2,DMA2_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM0_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_FEIF0)
  {
    DMA2->LIFCR |= DMA_LIFCR_CFEIF0;
    dma_state[DMA2_STREAM0_STATE].callback_transfer_error(DMA2,DMA2_Stream0,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM0_STATE].arg);
  }
}

void DMA2_Stream1_IRQHandler()
{
  if(DMA2->LISR & DMA_LISR_TCIF1)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTCIF1;
    dma_state[DMA2_STREAM1_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream1,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM1_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_HTIF1)
  {
    DMA2->LIFCR |= DMA_LIFCR_CHTIF1;
    dma_state[DMA2_STREAM1_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream1,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM1_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_TEIF1)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTEIF1;
    dma_state[DMA2_STREAM1_STATE].callback_transfer_error(DMA2,DMA2_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM1_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_DMEIF1)
  {
    DMA2->LIFCR |= DMA_LIFCR_CDMEIF1;
    dma_state[DMA2_STREAM1_STATE].callback_transfer_error(DMA2,DMA2_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM1_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_FEIF1)
  {
    DMA2->LIFCR |= DMA_LIFCR_CFEIF1;
    dma_state[DMA2_STREAM1_STATE].callback_transfer_error(DMA2,DMA2_Stream1,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM1_STATE].arg);
  }
}

void DMA2_Stream2_IRQHandler()
{
  if(DMA2->LISR & DMA_LISR_TCIF2)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTCIF2;
    dma_state[DMA2_STREAM2_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream2,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM2_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_HTIF2)
  {
    DMA2->LIFCR |= DMA_LIFCR_CHTIF2;
    dma_state[DMA2_STREAM2_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream2,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM2_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_TEIF2)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTEIF2;
    dma_state[DMA2_STREAM2_STATE].callback_transfer_error(DMA2,DMA2_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM2_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_DMEIF2)
  {
    DMA2->LIFCR |= DMA_LIFCR_CDMEIF2;
    dma_state[DMA2_STREAM2_STATE].callback_transfer_error(DMA2,DMA2_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM2_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_FEIF2)
  {
    DMA2->LIFCR |= DMA_LIFCR_CFEIF2;
    dma_state[DMA2_STREAM2_STATE].callback_transfer_error(DMA2,DMA2_Stream2,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM2_STATE].arg);
  }
}

void DMA2_Stream3_IRQHandler()
{
  if(DMA2->LISR & DMA_LISR_TCIF3)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTCIF3;
    dma_state[DMA2_STREAM3_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream3,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM3_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_HTIF3)
  {
    DMA2->LIFCR |= DMA_LIFCR_CHTIF3;
    dma_state[DMA2_STREAM3_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream3,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM3_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_TEIF3)
  {
    DMA2->LIFCR |= DMA_LIFCR_CTEIF3;
    dma_state[DMA2_STREAM3_STATE].callback_transfer_error(DMA2,DMA2_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM3_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_DMEIF3)
  {
    DMA2->LIFCR |= DMA_LIFCR_CDMEIF3;
    dma_state[DMA2_STREAM3_STATE].callback_transfer_error(DMA2,DMA2_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM3_STATE].arg);
  }

  if(DMA2->LISR & DMA_LISR_FEIF3)
  {
    DMA2->LIFCR |= DMA_LIFCR_CFEIF3;
    dma_state[DMA2_STREAM3_STATE].callback_transfer_error(DMA2,DMA2_Stream3,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM3_STATE].arg);
  }
}

void DMA2_Stream4_IRQHandler()
{
  if(DMA2->HISR & DMA_HISR_TCIF4)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTCIF4;
    dma_state[DMA2_STREAM4_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream4,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM4_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_HTIF4)
  {
    DMA2->HIFCR |= DMA_HIFCR_CHTIF4;
    dma_state[DMA2_STREAM4_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream4,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM4_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_TEIF4)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTEIF4;
    dma_state[DMA2_STREAM4_STATE].callback_transfer_error(DMA2,DMA2_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM4_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_DMEIF4)
  {
    DMA2->HIFCR |= DMA_HIFCR_CDMEIF4;
    dma_state[DMA2_STREAM4_STATE].callback_transfer_error(DMA2,DMA2_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM4_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_FEIF4)
  {
    DMA2->HIFCR |= DMA_HIFCR_CFEIF4;
    dma_state[DMA2_STREAM4_STATE].callback_transfer_error(DMA2,DMA2_Stream4,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM4_STATE].arg);
  }
}

void DMA2_Stream5_IRQHandler()
{
  if(DMA2->HISR & DMA_HISR_TCIF5)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTCIF5;
    dma_state[DMA2_STREAM5_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream5,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM5_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_HTIF5)
  {
    DMA2->HIFCR |= DMA_HIFCR_CHTIF5;
    dma_state[DMA2_STREAM5_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream5,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM5_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_TEIF5)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTEIF5;
    dma_state[DMA2_STREAM5_STATE].callback_transfer_error(DMA2,DMA2_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM5_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_DMEIF5)
  {
    DMA2->HIFCR |= DMA_HIFCR_CDMEIF5;
    dma_state[DMA2_STREAM5_STATE].callback_transfer_error(DMA2,DMA2_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM5_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_FEIF5)
  {
    DMA2->HIFCR |= DMA_HIFCR_CFEIF5;
    dma_state[DMA2_STREAM5_STATE].callback_transfer_error(DMA2,DMA2_Stream5,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM5_STATE].arg);
  }
}

void DMA2_Stream6_IRQHandler()
{
  if(DMA2->HISR & DMA_HISR_TCIF6)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTCIF6;
    dma_state[DMA2_STREAM6_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream6,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM6_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_HTIF6)
  {
    DMA2->HIFCR |= DMA_HIFCR_CHTIF6;
    dma_state[DMA2_STREAM6_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream6,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM6_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_TEIF6)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTEIF6;
    dma_state[DMA2_STREAM6_STATE].callback_transfer_error(DMA2,DMA2_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM6_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_DMEIF6)
  {
    DMA2->HIFCR |= DMA_HIFCR_CDMEIF6;
    dma_state[DMA2_STREAM6_STATE].callback_transfer_error(DMA2,DMA2_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM6_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_FEIF6)
  {
    DMA2->HIFCR |= DMA_HIFCR_CFEIF6;
    dma_state[DMA2_STREAM6_STATE].callback_transfer_error(DMA2,DMA2_Stream6,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM6_STATE].arg);
  }
}

void DMA2_Stream7_IRQHandler()
{
  if(DMA2->HISR & DMA_HISR_TCIF7)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTCIF7;
    dma_state[DMA2_STREAM7_STATE]
      .callback_transfer_complete(DMA2,DMA2_Stream7,DMA_TRANSFER_COMPLETE,dma_state[DMA2_STREAM7_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_HTIF7)
  {
    DMA2->HIFCR |= DMA_HIFCR_CHTIF7;
    dma_state[DMA2_STREAM7_STATE]
      .callback_transfer_half(DMA2,DMA2_Stream7,DMA_TRANSFER_HALF,dma_state[DMA2_STREAM7_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_TEIF7)
  {
    DMA2->HIFCR |= DMA_HIFCR_CTEIF7;
    dma_state[DMA2_STREAM7_STATE].callback_transfer_error(DMA2,DMA2_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM7_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_DMEIF7)
  {
    DMA2->HIFCR |= DMA_HIFCR_CDMEIF7;
    dma_state[DMA2_STREAM7_STATE].callback_transfer_error(DMA2,DMA2_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM7_STATE].arg);
  }

  if(DMA2->HISR & DMA_HISR_FEIF7)
  {
    DMA2->HIFCR |= DMA_HIFCR_CFEIF7;
    dma_state[DMA2_STREAM7_STATE].callback_transfer_error(DMA2,DMA2_Stream7,DMA_TRANSFER_ERROR,dma_state[DMA2_STREAM7_STATE].arg);
  }
}

DMA_Stream_TypeDef * getStreamByIdx(int iIdx) {
  return dma_state[iIdx].stream;
}

DMA_TypeDef * getDmaByIdx(int iIdx) {
  return dma_state[iIdx].dma;
}
