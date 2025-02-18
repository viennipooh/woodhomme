/**
  ******************************************************************************
  * @file    usart.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    09.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "usart.h"

enum usart_state_number_e {
  USART_STATE_1,
  USART_STATE_2,
  USART_STATE_3,
  USART_STATE_4,
  USART_STATE_5,
  USART_STATE_6,
};

static void nullcb(USART_TypeDef *usart,enum usart_event_e event,void*arg){};
static struct usart_state_s {
  USART_TypeDef         *usart;
  usart_callback_t      callback_txe;
  usart_callback_t      callback_cts;
  usart_callback_t      callback_tc;
  usart_callback_t      callback_rxne;
  usart_callback_t      callback_ore;
  usart_callback_t      callback_idle;
  usart_callback_t      callback_pe;
  usart_callback_t      callback_lbd;
  usart_callback_t      callback_ne;
  usart_callback_t      callback_fe;
  uint8_t               *buffer;
  uint32_t              len;
  void                  *arg;
} usart_state[] = {
  {
    .usart              = USART1,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .usart              = USART2,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .usart              = USART3,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .usart              = UART4,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .usart              = UART5,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
  {
    .usart              = USART6,
    .callback_txe       = nullcb,
    .callback_cts       = nullcb,
    .callback_tc        = nullcb,
    .callback_rxne      = nullcb,
    .callback_ore       = nullcb,
    .callback_idle      = nullcb,
    .callback_pe        = nullcb,
    .callback_lbd       = nullcb,
    .callback_ne        = nullcb,
    .callback_fe        = nullcb,
    .buffer             = 0,
    .len                = 0,
    .arg                = 0,
  },
};


static int usart_get_id(USART_TypeDef *pusart)
{
  switch((uint32_t)pusart)
  {
    case (uint32_t)USART1: return 0;
    case (uint32_t)USART2: return 1;
    case (uint32_t)USART3: return 2;
    case (uint32_t)UART4:  return 3;
    case (uint32_t)UART5:  return 4;
    case (uint32_t)USART6: return 5;
  }
  return USART_ERR;
}


int usart_set_baudrate(USART_TypeDef *pusart, uint32_t baudrate)
{
  uint32_t baud = baudrate;
  uint32_t apbclock;
  uint32_t integerdivider,fractionaldivider;
  uint32_t tmpreg;
  
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(baudrate > 10000000)
    return USART_ERR;
  
  if(baudrate > 5000000)
  {
    pusart->CR1 |= USART_CR1_OVER8;
  }
  
  uint8_t cIsPclk2_Div4 = ((RCC->CFGR & RCC_CFGR_PPRE2) == RCC_CFGR_PPRE2_DIV4);
  
  extern uint32_t SystemCoreClock;
  if ((pusart == USART1) || (pusart == USART6))
  {
    if (cIsPclk2_Div4)
      apbclock = SystemCoreClock/4;
    else
      apbclock = SystemCoreClock/2;
  }
  else
  {
    apbclock = SystemCoreClock/4;
  }
  
  if ((pusart->CR1 & USART_CR1_OVER8) != 0)
  {
    integerdivider = ((25 * apbclock) / (2 * (baud)));
  }
  else
  {
    integerdivider = ((25 * apbclock) / (4 * (baud)));
  }
  
  tmpreg = (integerdivider / 100) << 4;

  /* Determine the fractional part */
  fractionaldivider = integerdivider - (100 * (tmpreg >> 4));

  /* Implement the fractional part in the register */
  if ((pusart->CR1 & USART_CR1_OVER8) != 0)
  {
    tmpreg |= ((((fractionaldivider * 8) + 50) / 100)) & ((uint8_t)0x07);
  }
  else /* if ((USARTx->CR1 & USART_CR1_OVER8) == 0) */
  {
    tmpreg |= ((((fractionaldivider * 16) + 50) / 100)) & ((uint8_t)0x0F);
  }
  
  /* Write to USART BRR register */
  if (tmpreg > 65520)
    tmpreg = 65520;
  pusart->BRR = (uint16_t)tmpreg;
  return USART_OK;
}

int usart_set_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    pusart->CR1 |= USART_CR1_UE;
  }
  else
  {
    pusart->CR1 &=~USART_CR1_UE;
  }
  return USART_OK;
}

int usart_set_rcc_usart_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    switch((uint32_t)pusart)
    {
      case (uint32_t)USART1:
        {
          RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
          NVIC->IP[((uint32_t)(int32_t)USART1_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)USART1_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)USART1_IRQn) & 0x1FUL));
  
        }
        break;
      case (uint32_t)USART2:
        {
          RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
          NVIC->IP[((uint32_t)(int32_t)USART2_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)USART2_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)USART2_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)USART3:
        {
          RCC->APB1ENR |= RCC_APB1ENR_USART3EN;
          NVIC->IP[((uint32_t)(int32_t)USART3_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)USART3_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)USART3_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)UART4:
        {
          RCC->APB1ENR |= RCC_APB1ENR_UART4EN;
          NVIC->IP[((uint32_t)(int32_t)UART4_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)UART4_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)UART4_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)UART5:
        {
          RCC->APB1ENR |= RCC_APB1ENR_UART5EN;
          NVIC->IP[((uint32_t)(int32_t)UART5_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)UART5_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)UART5_IRQn) & 0x1FUL));
        }
        break;
      case (uint32_t)USART6:
        {
          RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
          NVIC->IP[((uint32_t)(int32_t)USART6_IRQn)]
            = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
          NVIC->ISER[(((uint32_t)(int32_t)USART6_IRQn) >> 5UL)]
            = (uint32_t)(1UL << (((uint32_t)(int32_t)USART6_IRQn) & 0x1FUL));
        }
        break;
      default:
        {
          return USART_ERR;
        }
        break;
    }
  }
  else
  {
    switch((uint32_t)pusart)
    {
      case (uint32_t)USART1:
        {
          RCC->APB1ENR &=~RCC_APB2ENR_USART1EN;
        }
        break;
      case (uint32_t)USART2:
        {
          RCC->APB1ENR &=~RCC_APB1ENR_USART2EN;
        }
        break;
      case (uint32_t)USART3:
        {
          RCC->APB1ENR &=~RCC_APB1ENR_USART3EN;
        }
        break;
      case (uint32_t)UART4:
        {
          RCC->APB1ENR &=~RCC_APB1ENR_UART4EN;
        }
        break;
      case (uint32_t)UART5:
        {
          RCC->APB1ENR &=~RCC_APB1ENR_UART5EN;
        }
        break;
      case (uint32_t)USART6:
        {
          RCC->APB1ENR &=~RCC_APB2ENR_USART6EN;
        }
        break;
      default:
        {
          return USART_ERR;
        }
        break;
    }
  }
  return USART_OK;
}

int usart_set_rcc_gpio_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    switch((uint32_t)pusart)
    {
      case (uint32_t)USART1:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
        }
        break;
      case (uint32_t)USART2:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
        }
        break;
      case (uint32_t)USART3:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
        }
        break;
      case (uint32_t)UART4:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
        }
        break;
      case (uint32_t)UART5:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
        }
        break;
      case (uint32_t)USART6:
        {
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
          RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
        }
        break;
      default:
        {
          return USART_ERR;
        }
        break;
    }
    pusart->CR1 = 0;
    pusart->CR2 = 0;
    pusart->CR3 = 0;
    pusart->SR  = 0;
    pusart->BRR = 0;
    pusart->GTPR= 0;
  }
  else
  {
    switch((uint32_t)pusart)
    {
      case (uint32_t)USART1:
        {
        }
        break;
      case (uint32_t)USART2:
        {
        }
        break;
      case (uint32_t)USART3:
        {
        }
        break;
      case (uint32_t)UART4:
        {
        }
        break;
      case (uint32_t)UART5:
        {
        }
        break;
      case (uint32_t)USART6:
        {
        }
        break;
      default:
        {
          return USART_ERR;
        }
        break;
    }
  }
  return USART_OK;
}

int usart_set_parity(USART_TypeDef *pusart, enum usart_parity_e parity)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  switch(parity)
  {
    case USART_PARITY_EVEN:
      {
        pusart->CR1 |= USART_CR1_PCE;
        pusart->CR1 &=~USART_CR1_PS;
      }
      break;
    case USART_PARITY_ODD:
      {
        pusart->CR1 |= USART_CR1_PCE;
        pusart->CR1 |= USART_CR1_PS;
      }
      break;
    case USART_PARITY_NOT:
    default:
      {
        pusart->CR1 &=~USART_CR1_PCE;
        return USART_OK;
      }
      break;
  }
  return USART_OK;
}

int usart_set_databits(USART_TypeDef *pusart, uint32_t databits)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  switch(databits)
  {
    case 7:
      {
      }
      break;
    case 8:
      {
        pusart->CR1 &=~USART_CR1_M;
      }
      break;
    case 9:
      {
        pusart->CR1 |= USART_CR1_M;
      }
      break;
    default:
      {
        return USART_ERR;
      }
      break;
  }
  return USART_OK;
}

int usart_set_stopbits(USART_TypeDef *pusart, uint32_t stopbits)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  switch(stopbits)
  {
    case 0x0001:
      {
        pusart->CR2 &=~USART_CR2_STOP;
        pusart->CR2 |= USART_CR2_STOP_0;
      }
      break;
    case 0x0105:
      {
        pusart->CR2 &=~USART_CR2_STOP;
        pusart->CR2 |= USART_CR2_STOP_1 | USART_CR2_STOP_0;
      }
      break;
    case 0x0002:
      {
        pusart->CR2 &=~USART_CR2_STOP;
        pusart->CR2 |= USART_CR2_STOP_1;
      }
      break;
    case 0x0100:
      {
        pusart->CR2 &=~USART_CR2_STOP;
      }
      break;
    default:
      {
        pusart->CR2 &=~USART_CR2_STOP;
        return USART_ERR;
      }
      break;
  }
  return USART_OK;
}

int usart_set_dma_tx_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    pusart->CR3 |= USART_CR3_DMAT;
  }
  else
  {
    pusart->CR3 &=~USART_CR3_DMAT;
  }
  return USART_OK;
}

int usart_set_dma_rx_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    pusart->CR3 |= USART_CR3_DMAR;
  }
  else
  {
    pusart->CR3 &=~USART_CR3_DMAR;
  }
  return USART_OK;
}

int usart_set_tx_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    pusart->CR1 |= USART_CR1_TE;
  }
  else
  {
    pusart->CR1 &=~USART_CR1_TE;
  }
  return USART_OK;
}

int usart_set_rx_enable(USART_TypeDef *pusart, uint32_t enable)
{
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  if(enable)
  {
    pusart->CR1 |= USART_CR1_RE;
  }
  else
  {
    pusart->CR1 &=~USART_CR1_RE;
  }
  return USART_OK;
}

int usart_set_gpio(USART_TypeDef *pusart, GPIO_TypeDef *pgpio, uint32_t pin)
{
  unsigned char af;
  
  if(pin>=16)
  {
    return USART_ERR;
  }
  
  switch((uint32_t)pusart)
  {
    case (uint32_t)USART1:
    case (uint32_t)USART2:
    case (uint32_t)USART3:
      {
        af=7;
      }
      break;
    case (uint32_t)UART4:
    case (uint32_t)UART5:
    case (uint32_t)USART6:
      {
        af=8;
      }
      break;
    default:
      return USART_ERR;
  }
  
  switch((uint32_t)pgpio)
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
      return USART_ERR;
  }
  
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
  
  return USART_OK;
}

int usart_irq( USART_TypeDef *pusart, int enable )
{
  if(enable)
  {
    switch((uint32_t)pusart)
    {
      case (uint32_t)USART1:
        {
          NVIC->IP[((uint32_t)(int32_t)USART1_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)USART1_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART1_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)USART2:
        {
          NVIC->IP[((uint32_t)(int32_t)USART2_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)USART2_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART2_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)USART3:
        {
          NVIC->IP[((uint32_t)(int32_t)USART3_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)USART3_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART3_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)UART4:
        {
          NVIC->IP[((uint32_t)(int32_t)UART4_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)UART4_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)UART4_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)UART5:
        {
          NVIC->IP[((uint32_t)(int32_t)UART5_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)UART5_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)UART5_IRQn) & 0x1FUL) );
        }
        break;
      case (uint32_t)USART6:
        {
          NVIC->IP[((uint32_t)(int32_t)USART6_IRQn)]
            = (uint8_t)( ((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) &
                         (uint32_t)0xFFUL );
          NVIC->ISER[(((uint32_t)(int32_t)USART6_IRQn) >> 5UL)]
            = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART6_IRQn) & 0x1FUL) );
        }
        break;
      default:
        {
          return USART_ERR;
        }
        break;
    }
  }
  else
  {
    return USART_ERR;
  }
  return USART_OK;
}

void usart_irq_disable_all() {
  NVIC->ICER[(((uint32_t)(int32_t)USART1_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART1_IRQn) & 0x1FUL) );
  NVIC->ICER[(((uint32_t)(int32_t)USART2_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART2_IRQn) & 0x1FUL) );
  NVIC->ICER[(((uint32_t)(int32_t)USART3_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART3_IRQn) & 0x1FUL) );
  NVIC->ICER[(((uint32_t)(int32_t)UART4_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)UART4_IRQn) & 0x1FUL) );
  NVIC->ICER[(((uint32_t)(int32_t)UART5_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)UART5_IRQn) & 0x1FUL) );
  NVIC->ICER[(((uint32_t)(int32_t)USART6_IRQn) >> 5UL)]
    = (uint32_t)( 1UL << (((uint32_t)(int32_t)USART6_IRQn) & 0x1FUL) );
}


int usart_event_arg( USART_TypeDef *pusart, 
                     void *arg)
{
  int index = 0;
  if(usart_irq(pusart,1) == USART_ERR)
    return USART_ERR;;
    
  index = usart_get_id(pusart);
  usart_state[index].arg = arg;
  return USART_OK;
}

int usart_event( USART_TypeDef *pusart, 
                 enum usart_event_e event, 
                 usart_callback_t cb)
{
  int index = 0;
  if(cb == 0)
    return USART_ERR;
  if(usart_irq(pusart,1) == USART_ERR)
    return USART_ERR;;
  
  index = usart_get_id(pusart);
  switch(event)
  {
    case USART_EVENT_CTS:
      {
        usart_state[index].callback_cts  = cb;
      }
      break;
    case USART_EVENT_LBD:
      {
        usart_state[index].callback_lbd  = cb;
      }
      break;
    case USART_EVENT_TXE:
      {
        usart_state[index].callback_txe  = cb;
        usart_state[index].usart->CR1 |= USART_CR1_TXEIE;
      }
      break;
    case USART_EVENT_TC:
      {
        usart_state[index].callback_tc   = cb;
        usart_state[index].usart->CR1 |=USART_CR1_TCIE;
      }
      break;
    case USART_EVENT_RXNE:
      {
        usart_state[index].callback_rxne = cb;
        usart_state[index].usart->CR1 |=USART_CR1_RXNEIE;
      }
      break;
    case USART_EVENT_IDLE:
      {
        usart_state[index].callback_idle = cb;
        usart_state[index].usart->CR1 |=USART_CR1_IDLEIE;
      }
      break;
    case USART_EVENT_ORE:
      {
        usart_state[index].callback_ore  = cb;
      }
      break;
    case USART_EVENT_NE:
      {
        usart_state[index].callback_ne   = cb;
      }
      break;
    case USART_EVENT_FE:
      {
        usart_state[index].callback_fe   = cb;
      }
      break;
    case USART_EVENT_PE:
      {
        usart_state[index].callback_pe   = cb;
      }
      break;
    default:
      {
        return USART_ERR;
      }
      break;
  }
  return USART_OK;
}

int usart_send( USART_TypeDef *pusart, void *buffer, int len )
{
  if(len <= 0)
    return USART_ERR;
  if(buffer == 0)
    return USART_ERR;
  if(usart_get_id(pusart) == USART_ERR)
    return USART_ERR;
  
  USART_TypeDef *usart = USART2;
  usart_state[usart_get_id(usart)].buffer = buffer;
  usart_state[usart_get_id(usart)].len    = len;
  
  
  usart->SR ^= USART_SR_TXE;
  usart->SR ^= USART_SR_TC;
  usart->DR = *usart_state[usart_get_id(usart)].buffer++;
  usart->CR1 |= USART_CR1_TXEIE;
  usart_state[usart_get_id(usart)].len--;
  
  usart->SR ^= USART_SR_RXNE | USART_SR_IDLE;
  while(usart_state[usart_get_id(usart)].len);
  
  return USART_OK;
}

void USART1_IRQHandler()
{
#define USARTn   0
#define USARTx   USART1
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
  
#undef USARTx
#undef USARTn
}

void USART2_IRQHandler()
{
#define USARTn   1
#define USARTx   USART2
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
  
#undef USARTx
#undef USARTn
}

void USART3_IRQHandler()
{
#define USARTn   2
#define USARTx   USART3
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
  
#undef USARTx
#undef USARTn
}

void UART4_IRQHandler()
{
#define USARTn   3
#define USARTx   UART4
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
#undef USARTx
#undef USARTn
}

void UART5_IRQHandler()
{
#define USARTn   4
#define USARTx   UART5
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
#undef USARTx
#undef USARTn
}

void USART6_IRQHandler()
{
#define USARTn   5
#define USARTx   USART6
  
  /**
   * Прием
   */
  if(USARTx->SR & USART_SR_FE)
  {
    //USARTx->SR ^= USART_SR_FE;
    usart_state[USARTn].callback_fe(USARTx,USART_EVENT_FE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_NE)
  {
    //USARTx->SR ^= USART_SR_NE;
    usart_state[USARTn].callback_ne(USARTx,USART_EVENT_NE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_PE)
  {
    //USARTx->SR ^= USART_SR_PE;
    usart_state[USARTn].callback_pe(USARTx,USART_EVENT_PE,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_ORE)
  {
    //USARTx->SR ^= USART_SR_ORE;
    usart_state[USARTn].callback_ore(USARTx,USART_EVENT_ORE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_IDLE)
  {
    //USARTx->SR ^= USART_SR_IDLE;
    usart_state[USARTn].callback_idle(USARTx,USART_EVENT_IDLE,usart_state[USARTn].arg);
    (void)USARTx->SR;
    (void)USARTx->DR;
  }
  if(USARTx->SR & USART_SR_RXNE)
  {
    USARTx->SR ^= USART_SR_RXNE;
    usart_state[USARTn].callback_rxne(USARTx,USART_EVENT_RXNE,usart_state[USARTn].arg);
  }
  
  /**
   * Передача
   */
  if(USARTx->SR & USART_SR_TC)
  {
    USARTx->SR ^= USART_SR_TC;
    usart_state[USARTn].callback_tc(USARTx,USART_EVENT_TC,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_TXE)
  {
    //USARTx->SR ^= USART_SR_TXE;
    usart_state[USARTn].callback_txe(USARTx,USART_EVENT_TXE,usart_state[USARTn].arg);
  }
  
  /**
   * Контроль
   */
  if(USARTx->SR & USART_SR_CTS)
  {
    USARTx->SR ^= USART_SR_CTS;
    usart_state[USARTn].callback_cts(USARTx,USART_EVENT_CTS,usart_state[USARTn].arg);
  }
  if(USARTx->SR & USART_SR_LBD)
  {
    USARTx->SR ^= USART_SR_LBD;
    usart_state[USARTn].callback_lbd(USARTx,USART_EVENT_LBD,usart_state[USARTn].arg);
  }
  
#undef USARTx
#undef USARTn
}

