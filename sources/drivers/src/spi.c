/**
  ******************************************************************************
  * @file    spi.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    20.04.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "spi.h"

static void nullcb(SPI_TypeDef *spi,enum spi_event_e event,void*arg){};
static struct spi_state_s {
  SPI_TypeDef         *spi;
  spi_callback_t      callback_txe;
  spi_callback_t      callback_rxne;
  spi_callback_t      callback_err;
  spi_callback_t      callback_err_ovr;
  spi_callback_t      callback_err_modf;
  spi_callback_t      callback_err_ti_fre;
  spi_callback_t      callback_err_crc;
  spi_callback_t      callback_err_udr;
  uint8_t               *buffer;
  uint32_t              len;
  void                  *arg;
} spi_state[] = {
  {
    .spi                = SPI1,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .spi                = SPI2,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .spi                = SPI3,
    .callback_txe       = nullcb,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .spi                = SPI4,
    .callback_txe       = nullcb,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .spi                = SPI5,
    .callback_txe       = nullcb,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .spi                = SPI6,
    .callback_txe       = nullcb,
    .callback_txe       = nullcb,
    .callback_rxne      = nullcb,
    .callback_err       = nullcb,
    .callback_err_ovr   = nullcb,
    .callback_err_modf  = nullcb,
    .callback_err_ti_fre= nullcb,
    .callback_err_crc   = nullcb,
    .callback_err_udr   = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
};


static int spi_get_id(SPI_TypeDef *pspi)
{
  switch((uint32_t)pspi)
  {
    case (uint32_t)SPI1: return 0;
    case (uint32_t)SPI2: return 1;
    case (uint32_t)SPI3: return 2;
    case (uint32_t)SPI4: return 3;
    case (uint32_t)SPI5: return 4;
    case (uint32_t)SPI6: return 5;
  }
  return SPI_ERR;
}


int spi_set_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_SPE;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_SPE;
  }
  return SPI_OK;
}
struct spi_af_gpio_s {
  SPI_TypeDef *pspi;
  GPIO_TypeDef *pgpio;
  uint32_t pin;
  uint8_t  af;
} static const spi_af_gpio_list[] ={
  
/****************************************************************************
   * GPIOA
   ****************************************************************************/
  /* SPI1 GPIOA AF5*/
  { .pspi  = SPI1,.pgpio = GPIOA,.pin   = 4,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOA,.pin   = 5,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOA,.pin   = 6,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOA,.pin   = 7,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOA,.pin   = 15,.af    = 5,},
  /* SPI3 GPIOA AF6*/
  { .pspi  = SPI3,.pgpio = GPIOA,.pin   = 4,.af    = 6,},
  { .pspi  = SPI3,.pgpio = GPIOA,.pin   = 15,.af    = 6,},
  
  /****************************************************************************
   * GPIOB
   ****************************************************************************/
  /* SPI1 GPIOB AF5*/
  { .pspi  = SPI1,.pgpio = GPIOB,.pin   = 3,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOB,.pin   = 4,.af    = 5,},
  { .pspi  = SPI1,.pgpio = GPIOB,.pin   = 5,.af    = 5,},
  /* SPI2 GPIOB AF5*/
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 9,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 10,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 12,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 13,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 14,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOB,.pin   = 15,.af    = 5,},
  /* SPI3 GPIOB AF6*/
  { .pspi  = SPI3,.pgpio = GPIOB,.pin   = 3,.af    = 6,},
  { .pspi  = SPI3,.pgpio = GPIOB,.pin   = 4,.af    = 6,},
  { .pspi  = SPI3,.pgpio = GPIOB,.pin   = 5,.af    = 6,},
  
  /****************************************************************************
   * GPIOC
   ****************************************************************************/
  /* SPI2 GPIOC AF5*/
  { .pspi  = SPI2,.pgpio = GPIOC,.pin   = 2,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOC,.pin   = 3,.af    = 5,},
  /* SPI3 GPIOC AF6*/
  { .pspi  = SPI3,.pgpio = GPIOC,.pin   = 10,.af    = 6,},
  { .pspi  = SPI3,.pgpio = GPIOC,.pin   = 11,.af    = 6,},
  { .pspi  = SPI3,.pgpio = GPIOC,.pin   = 12,.af    = 6,},
  
  /****************************************************************************
   * GPIOD
   ****************************************************************************/
  /* SPI2 GPIOD AF5*/
  { .pspi  = SPI2,.pgpio = GPIOD,.pin   = 3,.af    = 5,},
  /* SPI3 GPIOD AF5*/
  { .pspi  = SPI3,.pgpio = GPIOD,.pin   = 6,.af    = 5,},
  
  /****************************************************************************
   * GPIOE
   ****************************************************************************/
  /* SPI4 GPIOE AF5*/
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 2,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 4,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 5,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 6,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 11,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 12,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 13,.af    = 5,},
  { .pspi  = SPI4,.pgpio = GPIOE,.pin   = 14,.af    = 5,},
  
  /****************************************************************************
   * GPIOF
   ****************************************************************************/
  /* SPI5 GPIOF AF5*/
  { .pspi  = SPI5,.pgpio = GPIOF,.pin   = 6,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOF,.pin   = 7,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOF,.pin   = 8,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOF,.pin   = 9,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOF,.pin   = 11,.af    = 5,},
  
  /****************************************************************************
   * GPIOG
   ****************************************************************************/
  /* SPI6 GPIOG AF5*/
  { .pspi  = SPI6,.pgpio = GPIOG,.pin   = 8,.af    = 5,},
  { .pspi  = SPI6,.pgpio = GPIOG,.pin   = 12,.af    = 5,},
  { .pspi  = SPI6,.pgpio = GPIOG,.pin   = 13,.af    = 5,},
  { .pspi  = SPI6,.pgpio = GPIOG,.pin   = 14,.af    = 5,},
  
  /****************************************************************************
   * GPIOH
   ****************************************************************************/
  /* SPI5 GPIOH AF5*/
  { .pspi  = SPI5,.pgpio = GPIOH,.pin   = 5,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOH,.pin   = 6,.af    = 5,},
  { .pspi  = SPI5,.pgpio = GPIOH,.pin   = 7,.af    = 5,},
  
  /****************************************************************************
   * GPIOI
   ****************************************************************************/
  /* SPI5 GPIOI AF5*/
  { .pspi  = SPI2,.pgpio = GPIOI,.pin   = 0,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOI,.pin   = 1,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOI,.pin   = 2,.af    = 5,},
  { .pspi  = SPI2,.pgpio = GPIOI,.pin   = 3,.af    = 5,},
};
int spi_set_gpio(SPI_TypeDef *pspi, GPIO_TypeDef *pgpio, uint32_t pin)
{
  unsigned char af;
  
  if(pin>=16)
  {
    return SPI_ERR;
  }
  for(int index=0;index<sizeof(spi_af_gpio_list)/sizeof(struct spi_af_gpio_s);index++)
  {
    if((spi_af_gpio_list[index].pspi  == pspi) &&
       (spi_af_gpio_list[index].pgpio == pgpio) &&
       (spi_af_gpio_list[index].pin   == pin))
    {
      switch((uint32_t)pgpio)
      {
        case (uint32_t)GPIOA: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOAEN;break;
        case (uint32_t)GPIOB: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;break;
        case (uint32_t)GPIOC: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;break;
        case (uint32_t)GPIOD: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;break;
        case (uint32_t)GPIOG: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOGEN;break;
        default:
          return SPI_ERR;
      }
      
      af = spi_af_gpio_list[index].af;
      
      pgpio->MODER      &=~(0x3<<(pin<<1)); 
      pgpio->MODER      |= (0x2<<(pin<<1));
      if(pin>=8)
      {
        pgpio->AFR[1]   &=~(0xF<<((pin-8)<<2));
        pgpio->AFR[1]   |= (af<<((pin-8)<<2));
      }
      else
      {
        pgpio->AFR[0]   &=~(0xF<<((pin)<<2));
        pgpio->AFR[0]   |= (af<<((pin)<<2));
      }
      pgpio->OTYPER     &=~(0x1<<(pin)); 
      pgpio->OSPEEDR    |= (0x3<<(pin<<1));
      pgpio->PUPDR      &=~(0x3<<(pin<<1));
      
      return SPI_OK;
    }
  }
  
  return SPI_ERR;
}


static int spi_irq( SPI_TypeDef *pspi, int enable )
{
  if(enable)
  {
    switch((uint32_t)pspi)
    {
      case (uint32_t)SPI1:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI1_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI1_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI1_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)SPI2:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI2_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI2_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI2_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)SPI3:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI3_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI3_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI3_IRQn) & 0x1FUL) );
        }
        break;
        
#if defined(STM32F429_439xx) || defined(STM32F427_437xx)
      case (uint32_t)SPI4:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI4_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI4_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI4_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)SPI5:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI5_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI5_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI5_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)SPI6:
        {
          NVIC->IP[((uint32_t)(int32_t)SPI6_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)SPI6_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)SPI6_IRQn) & 0x1FUL) );
        }
        break;
        
#endif //defined(STM32F429_439xx) || defined(STM32F427_437xx)
      default:
        {
          return SPI_ERR;
        }
        break;
    }
  }
  else
  {
    return SPI_ERR;
  }
  return SPI_OK;
}

int spi_event_arg( SPI_TypeDef *pspi, 
                   void *arg)
{
  int index = 0;
  if(spi_irq(pspi,1) == SPI_ERR)
    return SPI_ERR;;
    
  index = spi_get_id(pspi);
  spi_state[index].arg = arg;
  return SPI_OK;
}

int spi_event( SPI_TypeDef *pspi, 
               enum spi_event_e event, 
               spi_callback_t cb)
{
  int index = 0;
  if(cb == 0)
    return SPI_ERR;
  if(spi_irq(pspi,1) == SPI_ERR)
    return SPI_ERR;;
  
  index = spi_get_id(pspi);
  switch(event)
  {
    case SPI_EVENT_TXE:
      {
        spi_state[index].callback_txe  = cb;
        spi_state[index].spi->CR2 |= SPI_CR2_TXEIE;
      }
      break;
    case SPI_EVENT_RXNE:
      {
        spi_state[index].callback_rxne = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_RXNEIE;
      }
      break;
    case SPI_EVENT_ERR:
      {
        spi_state[index].callback_err = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    case SPI_EVENT_ERR_OVR:
      {
        spi_state[index].callback_err_ovr  = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    case SPI_EVENT_ERR_MODF:
      {
        spi_state[index].callback_err_modf   = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    case SPI_EVENT_ERR_TI_FRE:
      {
        spi_state[index].callback_err_ti_fre   = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    case SPI_EVENT_ERR_UDR:
      {
        spi_state[index].callback_err_udr   = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    case SPI_EVENT_ERR_CRC:
      {
        spi_state[index].callback_err_crc   = cb;
        spi_state[index].spi->CR2 |=SPI_CR2_ERRIE;
      }
      break;
    default:
      {
        return SPI_ERR;
      }
      break;
  }
  return SPI_OK;
}

int spi_set_rcc_spi_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    switch((uint32_t)pspi)
    {
      case (uint32_t)SPI1:
        {
          RCC->APB2RSTR &=~RCC_APB2RSTR_SPI1RST;
          RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
          NVIC->IP[((uint32_t)(int32_t)SPI1_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI1_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI1_IRQn) & 0x1FUL));
  
        }
        break;
      case (uint32_t)SPI2:
        {
          RCC->APB1RSTR &=~RCC_APB1RSTR_SPI2RST;
          RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
          NVIC->IP[((uint32_t)(int32_t)SPI2_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI2_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI2_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)SPI3:
        {
          RCC->APB1RSTR &=~RCC_APB1RSTR_SPI3RST;
          RCC->APB1ENR |= RCC_APB1ENR_SPI3EN;
          NVIC->IP[((uint32_t)(int32_t)SPI3_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI3_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI3_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)SPI4:
        {
          RCC->APB2RSTR &=~RCC_APB2RSTR_SPI4RST;
          RCC->APB2ENR |= RCC_APB2ENR_SPI4EN;
          NVIC->IP[((uint32_t)(int32_t)SPI4_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI4_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI4_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)SPI5:
        {
          RCC->APB2RSTR &=~RCC_APB2RSTR_SPI5RST;
          RCC->APB2ENR |= RCC_APB2ENR_SPI5EN;
          NVIC->IP[((uint32_t)(int32_t)SPI5_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI5_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI5_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)SPI6:
        {
          RCC->APB2RSTR &=~RCC_APB2RSTR_SPI6RST;
          RCC->APB2ENR |= RCC_APB2ENR_SPI6EN;
          NVIC->IP[((uint32_t)(int32_t)SPI6_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)SPI6_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI6_IRQn) & 0x1FUL));
        }
        break;
      default:
        {
          return SPI_ERR;
        }
        break;
    }
  }
  else
  {
    switch((uint32_t)pspi)
    {
      case (uint32_t)SPI1:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI1_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI1_IRQn) & 0x1FUL));
          RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
          RCC->APB2ENR &=~RCC_APB2ENR_SPI1EN;
        }
        break;
      case (uint32_t)SPI2:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI2_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI2_IRQn) & 0x1FUL));
          RCC->APB1RSTR |= RCC_APB1RSTR_SPI2RST;
          RCC->APB1ENR &=~RCC_APB1ENR_SPI2EN;
        }
        break;
      case (uint32_t)SPI3:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI3_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI3_IRQn) & 0x1FUL));
          RCC->APB1RSTR |= RCC_APB1RSTR_SPI3RST;
          RCC->APB1ENR &=~RCC_APB1ENR_SPI3EN;
        }
        break;
      case (uint32_t)SPI4:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI4_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI4_IRQn) & 0x1FUL));
          RCC->APB2RSTR |= RCC_APB2RSTR_SPI4RST;
          RCC->APB2ENR &=~RCC_APB2ENR_SPI4EN;
        }
        break;
      case (uint32_t)SPI5:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI5_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI5_IRQn) & 0x1FUL));
          RCC->APB2RSTR |= RCC_APB2RSTR_SPI5RST;
          RCC->APB2ENR &=~RCC_APB2ENR_SPI5EN;
        }
        break;
      case (uint32_t)SPI6:
        {
          NVIC->ICER[(((uint32_t)(int32_t)SPI6_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)SPI6_IRQn) & 0x1FUL));
          RCC->APB2RSTR |= RCC_APB2RSTR_SPI6RST;
          RCC->APB1ENR &=~RCC_APB2ENR_SPI6EN;
        }
        break;
      default:
        {
          return SPI_ERR;
        }
        break;
    }
  }
  return SPI_OK;
}

int spi_set_rcc_gpio_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  switch((uint32_t)pspi)
  {
    case (uint32_t)SPI1:
    case (uint32_t)SPI2:
    case (uint32_t)SPI3:
    case (uint32_t)SPI4:
    case (uint32_t)SPI5:
    case (uint32_t)SPI6:
      break;
    default:
      {
        return SPI_ERR;
      }
      break;
  }
  
  if(enable)
  {
    for(int index=0;index<sizeof(spi_af_gpio_list)/sizeof(struct spi_af_gpio_s);index++)
    {
      if(spi_af_gpio_list[index].pspi  == pspi)
      {
        switch((uint32_t)spi_af_gpio_list[index].pgpio)
        {
          case (uint32_t)GPIOA: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOAEN;break;
          case (uint32_t)GPIOB: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;break;
          case (uint32_t)GPIOC: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;break;
          case (uint32_t)GPIOD: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;break;
          case (uint32_t)GPIOG: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOGEN;break;
          case (uint32_t)GPIOH: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOHEN;break;
          case (uint32_t)GPIOI: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOIEN;break;
          case (uint32_t)GPIOJ: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOJEN;break;
          case (uint32_t)GPIOK: RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOKEN;break;
          default:
            return SPI_ERR;
        }
      }
    }
    pspi->CR1 = 0;
    pspi->CR2 = 0;
    pspi->SR  = 0;
  }
  else
  {
    switch((uint32_t)pspi)
    {
      case (uint32_t)SPI1:
        {
        }
        break;
      case (uint32_t)SPI2:
        {
        }
        break;
      case (uint32_t)SPI3:
        {
        }
        break;
      case (uint32_t)SPI4:
        {
        }
        break;
      case (uint32_t)SPI5:
        {
        }
        break;
      case (uint32_t)SPI6:
        {
        }
        break;
      default:
        {
          return SPI_ERR;
        }
        break;
    }
  }
  return SPI_OK;
}


int spi_send( SPI_TypeDef *pspi, void *buffer, int len );
int spi_send_wait( SPI_TypeDef *pspi )
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  while(pspi->SR & SPI_SR_BSY != 0);
  return 0;
}
int spi_wait( SPI_TypeDef *pspi )
{
  uint8_t temp = 0;
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  while(1)
  {
    temp = pspi->SR;
    if((temp & SPI_SR_BSY) == 0)
    {
      break;
    }
  };
  return 0;
}

int spi_send_state( SPI_TypeDef *pspi );

int spi_send( SPI_TypeDef *pspi, void *buffer, int len )
{
  if(len <= 0)
    return SPI_ERR;
  if(buffer == 0)
    return SPI_ERR;
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  SPI_TypeDef *spi = pspi;
  spi_state[spi_get_id(spi)].buffer = buffer;
  spi_state[spi_get_id(spi)].len    = len;
  
  
  spi->SR ^= SPI_SR_TXE;
  spi->DR = *spi_state[spi_get_id(spi)].buffer++;
  //spi->CR2 |= SPI_CR2_TXEIE;
  spi_state[spi_get_id(spi)].len--;
  
  spi->SR ^= SPI_SR_RXNE;
  //while(spi_state[spi_get_id(spi)].len);
  
  return SPI_OK;
}

int spi_set_baudrate_control(SPI_TypeDef *pspi, uint32_t baudrate)
{
  uint32_t temp = 0;
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  if(baudrate > 7)
    return SPI_ERR;
  
  temp = pspi->CR1;
  temp &= ~SPI_CR1_BR;
  temp |= (SPI_CR1_BR & (baudrate<<3));
  pspi->CR1 = temp;
  
  return SPI_OK;
}

int spi_set_data_format(SPI_TypeDef *pspi, spi_data_format_t format)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  switch(format)
  {
    case SPI_DATA_FORMAT_8:
      {
        pspi->CR1 &=~SPI_CR1_DFF;
      }
      break;
    case SPI_DATA_FORMAT_16:
      {
        pspi->CR1 |= SPI_CR1_DFF;
      }
      break;
    default:
      {
        return SPI_ERR;
      }
      break;
  }
  return SPI_OK;
}

int spi_set_frame_format        (SPI_TypeDef *pspi, spi_frame_format_t format)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
#define SPI_CR2_FRF (1<<4)
  switch(format)
  {
    case SPI_FRAME_FORMAT_MOTOROLA:
      {
        pspi->CR2 &=~SPI_CR2_FRF;
      }
      break;
    case SPI_FRAME_FORMAT_TI:
      {
        pspi->CR2 |= SPI_CR2_FRF;
      }
      break;
    default:
      {
        return SPI_ERR;
      }
      break;
  }
  return SPI_OK;
}

int spi_set_master              (SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_MSTR;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_MSTR;
  }
  return SPI_OK;
}

int spi_set_cpol                (SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_CPOL;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_CPOL;
  }
  return SPI_OK;
}

int spi_set_cpha                (SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_CPHA;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_CPHA;
  }
  return SPI_OK;
}

int spi_set_bidi_mode_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_BIDIMODE;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_BIDIMODE;
  }
  return SPI_OK;
}

int spi_set_bidi_output_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_BIDIOE;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_BIDIOE;
  }
  return SPI_OK;
}

int spi_set_crc_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_CRCEN;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_CRCEN;
  }
  return SPI_OK;
}

int spi_set_crcnext(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_CRCNEXT;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_CRCNEXT;
  }
  return SPI_OK;
}

int spi_set_rxonly(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_RXONLY;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_RXONLY;
  }
  return SPI_OK;
}

int spi_set_ssi(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_SSI;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_SSI;
  }
  return SPI_OK;
}

int spi_set_ssm(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_SSM;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_SSM;
  }
  return SPI_OK;
}

int spi_set_lsb_first(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR1 |= SPI_CR1_LSBFIRST;
  }
  else
  {
    pspi->CR1 &=~SPI_CR1_LSBFIRST;
  }
  return SPI_OK;
}

int spi_set_dma_tx_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR2 |= SPI_CR2_TXDMAEN;
  }
  else
  {
    pspi->CR2 &=~SPI_CR2_TXDMAEN;
  }
  return SPI_OK;
}

int spi_set_dma_rx_enable(SPI_TypeDef *pspi, uint32_t enable)
{
  if(spi_get_id(pspi) == SPI_ERR)
    return SPI_ERR;
  
  if(enable)
  {
    pspi->CR2 |= SPI_CR2_RXDMAEN;
  }
  else
  {
    pspi->CR2 &=~SPI_CR2_RXDMAEN;
  }
  return SPI_OK;
}









#define SPI_SR_FRE      (1<<8)
void SPI1_IRQHandler()
{
#define SPIn   0
#define SPIx   SPI1
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
#undef SPIx
#undef SPIn
}

void SPI2_IRQHandler()
{
#define SPIn   1
#define SPIx   SPI2
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
  
#undef SPIx
#undef SPIn
}

void SPI3_IRQHandler()
{
#define SPIn   2
#define SPIx   SPI3
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
  
#undef SPIx
#undef SPIn
}

#if defined(STM32F429_439xx) || defined(STM32F427_437xx)
void SPI4_IRQHandler()
{
#define SPIn   3
#define SPIx   SPI4
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
#undef SPIx
#undef SPIn
}

void SPI5_IRQHandler()
{
#define SPIn   4
#define SPIx   SPI5
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
#undef SPIx
#undef SPIn
}

void SPI6_IRQHandler()
{
#define SPIn   5
#define SPIx   SPI6
  
  if(SPIx->SR & SPI_SR_TXE)
  {
    SPIx->SR ^= SPI_SR_TXE;
    spi_state[SPIn].callback_txe(SPIx,SPI_EVENT_TXE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_RXNE)
  {
    SPIx->SR ^= SPI_SR_RXNE;
    spi_state[SPIn].callback_rxne(SPIx,SPI_EVENT_RXNE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_FRE)
  {
    spi_state[SPIn].callback_err_ti_fre(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_TI_FRE,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_OVR)
  {
    SPIx->SR ^= SPI_SR_OVR;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_OVR,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_MODF)
  {
    SPIx->SR ^= SPI_SR_MODF;
    spi_state[SPIn].callback_err_ovr(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_MODF,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_CRCERR)
  {
    SPIx->SR ^= SPI_SR_CRCERR;
    spi_state[SPIn].callback_err_crc(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_CRC,spi_state[SPIn].arg);
  }
  if(SPIx->SR & SPI_SR_UDR)
  {
    SPIx->SR ^= SPI_SR_UDR;
    spi_state[SPIn].callback_err_udr(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
    spi_state[SPIn].callback_err(SPIx,SPI_EVENT_ERR_UDR,spi_state[SPIn].arg);
  }
  
#undef SPIx
#undef SPIn
}

#endif //defined(STM32F429_439xx) || defined(STM32F427_437xx)