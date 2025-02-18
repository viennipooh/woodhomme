#include "main.h"
#include "ksz8895fmq.h"
#include "spi.h"
#include "dma.h"

void delay(uint32_t pause);

static inline void gpio_init_pp(GPIO_TypeDef *gpio, int pin)
{
  gpio->LCKR &= ~(1<<pin);

  gpio->MODER &= ~(3<<(pin<<1));
  gpio->MODER |=  (MODE_OUT<<(pin<<1));

  gpio->OTYPER &= ~(1<<pin);
  gpio->OTYPER |=  (OTYPE_PP<<pin);

  gpio->OSPEEDR &= ~(3<<(pin<<1));
  gpio->OSPEEDR |=  (OSPEED_VHS<<(pin<<1));

  gpio->PUPDR &= ~(3<<(pin<<1));
  gpio->PUPDR |=  (PUPD_N<<(pin<<1));
}

static inline void gpio_deinit_pp(GPIO_TypeDef *gpio, int pin)
{
  gpio->LCKR &= ~(1<<pin);
  gpio->MODER &= ~(3<<(pin<<1));
  gpio->OTYPER &= ~(1<<pin);
  gpio->OSPEEDR &= ~(3<<(pin<<1));
  gpio->PUPDR &= ~(3<<(pin<<1));
}

static inline void gpio_high(GPIO_TypeDef *gpio, int pin)
{
  gpio->BSRRL |= (1<<pin);
}
static inline void gpio_low(GPIO_TypeDef *gpio, int pin)
{
  gpio->BSRRH |= (1<<pin);
}

#define SEL()        {gpio_low(psw->cs.gpio, psw->cs.pin);;delay(10);}
#define DESEL()      {delay(10);gpio_high(psw->cs.gpio, psw->cs.pin);}

/* Сброс свитча */
int ksz8895fmq_reset_on( ksz8895fmq_t *psw )
{
  gpio_low( psw->rst.gpio, psw->rst.pin );
  return 0;
}

int ksz8895fmq_reset_off( ksz8895fmq_t *psw )
{
  gpio_high( psw->rst.gpio, psw->rst.pin );
  return 0;
}

/* Питание свитча */
int ksz8895fmq_pwr_on( ksz8895fmq_t *psw )
{
  if ( psw->pwr.gpio == 0 )
      return -1;
  gpio_low( psw->pwr.gpio, psw->pwr.pin );
  return 0;
}

int ksz8895fmq_pwr_off( ksz8895fmq_t *psw )
{
  if ( psw->pwr.gpio == 0 )
      return -1;
  gpio_high( psw->pwr.gpio, psw->pwr.pin );
  return 0;
}

int ksz8895fmq_select_flow_control(ksz8895fmq_t *psw, ksz8895fmq_port_t port,ksz8895fmq_flow_control_t mode)
{
  uint8_t regn = 0x02;
  uint8_t regv;
  switch(port)
  {
    case PORT_1:
      {
        regn |= 0x10;
      }
      break;
    case PORT_2:
      {
        regn |= 0x20;
      }
      break;
    case PORT_3:
      {
        regn |= 0x30;
      }
      break;
    case PORT_4:
      {
        regn |= 0x40;
      }
      break;
    case PORT_5:
      {
        regn |= 0x50;
      }
      break;
    default:
      return -1;
  }

  switch(mode)
  {
    case ENABLE_FLOW_CONTROL:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv |= 1<<4;
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
    case DISABLE_FLOW_CONTROL:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv &= ~(1<<4);
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
  }
  return 0;
}

int ksz8895fmq_select_duplex(ksz8895fmq_t *psw, ksz8895fmq_port_t port,ksz8895fmq_duplex_t mode)
{
  uint8_t regn = 0x0C;
  uint8_t regv;
  switch(port)
  {
    case PORT_1:
      {
        regn |= 0x10;
      }
      break;
    case PORT_2:
      {
        regn |= 0x20;
      }
      break;
    case PORT_3:
      {
        regn |= 0x30;
      }
      break;
    case PORT_4:
      {
        regn |= 0x40;
      }
      break;
    case PORT_5:
      {
        regn |= 0x50;
      }
      break;
    default:
      return -1;
  }
  switch(mode)
  {
    case HALF_DUPLEX:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv &= ~(1<<5);
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
    case FULL_DUPLEX:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv |= 1<<5;
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
  }
  return 0;
}

int ksz8895fmq_select_speed(ksz8895fmq_t *psw, ksz8895fmq_port_t port,ksz8895fmq_speed_t mode)
{
  uint8_t regn = 0x0C;
  uint8_t regv;
  switch(port)
  {
    case PORT_1:
      {
        regn |= 0x10;
      }
      break;
    case PORT_2:
      {
        regn |= 0x20;
      }
      break;
    case PORT_3:
      {
        regn |= 0x30;
      }
      break;
    case PORT_4:
      {
        regn |= 0x40;
      }
      break;
    case PORT_5:
      {
        regn |= 0x50;
      }
      break;
    default:
      return -1;
  }
  switch(mode)
  {
    case SPEED_10:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv &= ~(1<<6);
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
    case SPEED_100:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv |= 1<<6;
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
  }
  return 0;
}

int ksz8895fmq_select_autonegotiation(ksz8895fmq_t *psw,  ksz8895fmq_port_t port,ksz8895fmq_autonegotiation_t mode)
{
  uint8_t regn = 0x0C;
  uint8_t regv;
  switch(port)
  {
    case PORT_1:
      {
        regn |= 0x10;
      }
      break;
    case PORT_2:
      {
        regn |= 0x20;
      }
      break;
    case PORT_3:
      {
        regn |= 0x30;
      }
      break;
    case PORT_4:
      {
        regn |= 0x40;
      }
      break;
    case PORT_5:
      {
        regn |= 0x50;
      }
      break;
    default:
      return -1;
  }
  switch(mode)
  {
    case AUTO_ENABLE:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv &= ~(1<<7);
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
    case AUTO_DISABLE:
      {
        ksz8895fmq_read_register( psw, regn, &regv);
        regv |= 1<<7;
        ksz8895fmq_write_register( psw, regn, regv);
      }
      break;
  }
  return 0;
}

static void ksz8895fmq_init_gpio(void)
{

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                  RCC_AHB1ENR_GPIOBEN |
                  RCC_AHB1ENR_GPIOCEN |
                  RCC_AHB1ENR_GPIODEN |
                  RCC_AHB1ENR_GPIOEEN |
                  RCC_AHB1ENR_GPIOFEN |
                  RCC_AHB1ENR_GPIOGEN;
}

static uint8_t temp[0x210];
int ksz8895fmq_write_register( ksz8895fmq_t *psw, uint8_t reg, uint8_t value)
{
  temp[2] = value;
  return ksz8895fmq_write_registers(psw, reg, &temp[2], 1);
}
int ksz8895fmq_read_register( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value)
{
  temp[2] = *value;
  ksz8895fmq_read_registers( psw, reg, &temp[2], 1);
  *value = temp[2];
  return 0;
}

#include "semphr.h"
xSemaphoreHandle ksz8895fmq_mutex;

int ksz8895fmq_write_registers( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value, uint8_t len)
{
  if (xSemaphoreTake(ksz8895fmq_mutex, 200)) {
    temp[0] = 0x02;
    temp[1] = reg;
    SEL();
    dma_transfer_stop(psw->dma_rx);
    dma_transfer_stop(psw->dma_tx);
    dma_transfer(psw->dma_rx, (void*)temp, 2);
    dma_transfer(psw->dma_tx, (void*)temp, 2);
    dma_transfer_wait(psw->dma_tx);
    spi_wait(psw->pspi);
    dma_transfer(psw->dma_rx, (void*)temp, len);
    dma_transfer(psw->dma_tx, (void*)value, len);
    dma_transfer_wait(psw->dma_tx);
    spi_wait(psw->pspi);
    DESEL();
    xSemaphoreGive(ksz8895fmq_mutex);
  } else {
    return -1;
  }
  return 0;
}

int ksz8895fmq_read_registers( ksz8895fmq_t *psw, uint8_t reg, uint8_t *value, uint8_t len)
{
  if (xSemaphoreTake(ksz8895fmq_mutex, 200)) {
    temp[0] = 0x03;
    temp[1] = reg;
    SEL();
    dma_transfer_stop(psw->dma_rx);
    dma_transfer_stop(psw->dma_tx);
    dma_transfer(psw->dma_rx, (void*)temp, 2);
    dma_transfer(psw->dma_tx, (void*)temp, 2);
    dma_transfer_wait(psw->dma_tx);
    spi_wait(psw->pspi);
    dma_transfer(psw->dma_rx, (void*)value, len);
    dma_transfer(psw->dma_tx, (void*)value, len);
    dma_transfer_wait(psw->dma_tx);
    spi_wait(psw->pspi);
    DESEL();
    xSemaphoreGive(ksz8895fmq_mutex);
  } else {
    return -1;
  }
  return 0;
}

void ksz8895fmq_init_pwr( ksz8895fmq_t *psw )
{
  GPIO_SetBits(psw->pwr.gpio, (1 << psw->pwr.pin));
  gpio_init_pp( psw->pwr.gpio, psw->pwr.pin );
  GPIO_ResetBits(psw->rst.gpio, (1 << psw->rst.pin));
  gpio_init_pp( psw->rst.gpio, psw->rst.pin );
}

int ksz8895fmq_init( ksz8895fmq_t *psw )
{
  //Мютекс для разделения доступа к регистрам свитча
  ksz8895fmq_mutex = xSemaphoreCreateMutex();
  ksz8895fmq_init_gpio();
  ksz8895fmq_init_pwr( psw );
  gpio_init_pp( psw->cs.gpio,  psw->cs.pin );

  GPIO_SetBits(psw->fb[0].gpio, (1 << psw->fb[0].pin));
  GPIO_SetBits(psw->fb[1].gpio, (1 << psw->fb[1].pin));
  gpio_init_pp( psw->fb[0].gpio, psw->fb[0].pin );
  gpio_init_pp( psw->fb[1].gpio, psw->fb[1].pin );
  DESEL();
  return 0;
}

int ksz8895fmq_deinit(  ksz8895fmq_t *psw  )
{
  ksz8895fmq_init_gpio();
  gpio_deinit_pp( psw->rst.gpio, psw->rst.pin );
  gpio_deinit_pp( psw->pwr.gpio, psw->pwr.pin );
  gpio_deinit_pp( psw->cs.gpio,  psw->cs.pin );
  gpio_deinit_pp( psw->fb[0].gpio, psw->fb[0].pin );
  gpio_deinit_pp( psw->fb[1].gpio, psw->fb[1].pin );
  DESEL();
  return 0;
}

//только включение питания оптического порта
void ksz8895fmq_port_power_set( ksz8895fmq_t *psw, ksz8895fmq_port_t port, uint8_t state )
{
  switch(port)
  {
    case P3:
      if (state)
        gpio_low( psw->fb[0].gpio, psw->fb[0].pin );
      else
        gpio_high( psw->fb[0].gpio, psw->fb[0].pin );
      break;
    case P4:
      if (state)
        gpio_low( psw->fb[1].gpio, psw->fb[1].pin );
      else
        gpio_high( psw->fb[1].gpio, psw->fb[1].pin );
      break;
    default:
      break;
  }
}

#define REG_PORT(n,m)    ((((n)&0x7)<<4)|(0xF&(m)))
int ksz8895fmq_port_state_set( ksz8895fmq_t *psw, ksz8895fmq_port_t port, uint8_t state )
{
  int p = 0;
  switch(port) {
    case P1:
      p=1;
      break;
    case P2:
      p=2;
      break;
    case P3:
      p=3;
      break;
    case P4:
      p=4;
      break;
    case P5:
      p=5;
      break;
    default:
      return -1;
  }
  ksz8895fmq_read_register(psw, REG_PORT(p,2), &temp[2]);
  if (state)
    temp[2] |= (0x03<<1);
  else
    temp[2] &= ~(0x03<<1);
  ksz8895fmq_write_register(psw, REG_PORT(p,2), temp[2]);
  return 0;
}

int ksz8895fmq_port_on( ksz8895fmq_t *psw, ksz8895fmq_port_t port )
{
  int p=0,c=2;
  switch(port)
  {
    case P1:
      {
        p=1;
      }
      break;
    case P2:
      {
        p=2;
      }
      break;
    case P3:
      {
        p=3;
        gpio_low( psw->fb[0].gpio, psw->fb[0].pin );
      }
      break;
    case P4:
      {
        p=4;
        gpio_low( psw->fb[1].gpio, psw->fb[1].pin );
      }
      break;
    case P5:
      {
        p=5;
      }
      break;
    default:
      {
        return -1;
      }
      break;
  }
  ksz8895fmq_read_register(psw,REG_PORT(p,c),&temp[2]);
  temp[2] |= (0x03<<1);
  ksz8895fmq_write_register(psw,REG_PORT(p,c),temp[2]);
  return 0;
}

int ksz8895fmq_port_off( ksz8895fmq_t *psw, ksz8895fmq_port_t port)
{
  int p=0,c=2;
  switch(port)
  {
    case P1:
      {
        p=1;
      }
      break;
    case P2:
      {
        p=2;
      }
      break;
    case P3:
      {
        p=3;
        gpio_high( psw->fb[0].gpio, psw->fb[0].pin );
      }
      break;
    case P4:
      {
        p=4;
        gpio_high( psw->fb[1].gpio, psw->fb[1].pin );
      }
      break;
    case P5:
      {
        p=5;
      }
      break;
    default:
      {
        return -1;
      }
      break;
  }
  ksz8895fmq_read_register(psw,REG_PORT(p,c),&temp[2]);
  temp[2] &=~(0x03<<1);
  ksz8895fmq_write_register(psw,REG_PORT(p,c),temp[2]);
  return 0;
}

//void port_power_set(ksz8895fmq_t * pSW, int8_t iPort, bool iOn) {
//  enum extio_led_mode_e cMode = (iOn) ? EXTIO_LED_ON : EXTIO_LED_OFF;
//  if ((pSW != NULL) && ((iPort >= 0) || (iPort <= 3))) {
//    int8_t cPort = -1;
//    switch (iPort) {
//    case 1:   cPort = 4;      break;
//    case 2:   cPort = 1;      break;
//    case 3:   cPort = 0;      break;
//    default:
//      break;
//    }
//    extio_led((enum extio_led_e)cPort,cMode);
//    if (iOn) {
//      gpio_low( psw->fb[iPort].gpio, psw->fb[iPort].pin );
//    } else {
//      gpio_high( psw->fb[iPort].gpio, psw->fb[iPort].pin );
//    }
//  }
//}