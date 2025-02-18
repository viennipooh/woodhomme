#include "settings.h"
#include "usart.h"
#include "mode.h"
#include "main.h"

#if (MKPSH10 != 0)
static struct mode_struct_s mode_rs485_1 = {
  .usart    = UART5,
  .gpio_re  = GPIOD,
  .pin_re   = 7,
  .gpio_led = GPIOG,
  .pin_led  = 3,

  .gpio_tx  = GPIOC,
  .pin_tx   = 12,
  .gpio_rx  = GPIOD,
  .pin_rx   = 2,

  .mode=SETTINGS_IP_PORT_TCP_CLIENT,
  .timeout=0,
  .idle  = 1,
  .tblen = 0,
  .rblen = 0,
  .cport = 8081,
  .sport = 8081,
  .ip    = {
    .addr = IPADDR_ANY,
  },
  .rs485 = 1,
  .count = 0,
};

static struct mode_struct_s mode_rs485_2 = {
  .usart    = USART1,
  .gpio_re  = GPIOA,
  .pin_re   = 8,
  .gpio_led = GPIOD,
  .pin_led  = 5,

  .gpio_tx  = GPIOA,
  .pin_tx   = 9,
  .gpio_rx  = GPIOA,
  .pin_rx   = 10,

  .mode=SETTINGS_IP_PORT_TCP_CLIENT,
  .timeout=0,
  .idle  = 1,
  .tblen = 0,
  .rblen = 0,
  .cport = 8081,
  .sport = 8081,
  .ip    = {
    .addr = IPADDR_ANY,
  },
  .rs485 = 1,
  .count = 0,
};

struct mode_struct_s * mode_rs485[RS_PORTS_NUMBER] = { &mode_rs485_1, &mode_rs485_2 };
#endif

#if (IMC_FTX_MC != 0)
static struct mode_struct_s mode_rs485_1 = {
  .usart    = USART1,
  .gpio_re  = GPIOA,
#if PIXEL
  .pin_re   = 11,
#else
  .pin_re   = 12,
#endif
  .gpio_led = GPIOA,
  .pin_led  = 8,

  .gpio_tx  = GPIOA,
  .pin_tx   = 9,
  .gpio_rx  = GPIOA,
  .pin_rx   = 10,

  .mode=SETTINGS_IP_PORT_TCP_CLIENT,
  .timeout=0,
  .idle  = 1,
  .tblen = 0,
  .rblen = 0,
  .cport = 8081,
  .sport = 8081,
  .ip    = {
    .addr = IPADDR_ANY,
  },
  .rs485 = 1,
  .count = 0,
};

struct mode_struct_s * mode_rs485[RS_PORTS_NUMBER] = { &mode_rs485_1 };
#endif

#if ((UTD_M != 0) || (IIP != 0))
static struct mode_struct_s mode_rs485_1 = {
  .usart    = USART1,
  .gpio_re  = GPIOA,
  .pin_re   = 11,
  .gpio_led = NULL, //GPIOD,
  .pin_led  = 13,

  .gpio_tx  = GPIOA,
  .pin_tx   = 9,
  .gpio_rx  = GPIOA,
  .pin_rx   = 10,

  .mode=SETTINGS_IP_PORT_TCP_CLIENT,
  .timeout=0,
  .idle  = 1,
  .tblen = 0,
  .rblen = 0,
  .cport = 8081,
  .sport = 8081,
  .ip    = {
    .addr = IPADDR_ANY,
  },
  .rs485 = 1,
  .count = 0,
};

static struct mode_struct_s mode_rs485_2 = {
  .usart    = USART6,
  .gpio_re  = GPIOC,
  .pin_re   = 8,
  .gpio_led = NULL, //GPIOD,
  .pin_led  = 12,

  .gpio_tx  = GPIOC,
  .pin_tx   = 6,
  .gpio_rx  = GPIOC,
  .pin_rx   = 7,

  .mode=SETTINGS_IP_PORT_TCP_CLIENT,
  .timeout=0,
  .idle  = 1,
  .tblen = 0,
  .rblen = 0,
  .cport = 8081,
  .sport = 8081,
  .ip    = {
    .addr = IPADDR_ANY,
  },
  .rs485 = 1,
  .count = 0,
};

struct mode_struct_s * mode_rs485[RS_PORTS_NUMBER] = { &mode_rs485_1, &mode_rs485_2 };
#endif

struct mode_struct_s * GetMode(int iModeIdx) {
  if ((iModeIdx < 0) || (iModeIdx >= RS_PORTS_NUMBER))
    return NULL;
  return mode_rs485[iModeIdx];
}

bool GetTxState(int iModeIdx) {
  if ((iModeIdx < 0) || (iModeIdx >= RS_PORTS_NUMBER))
    return false;
  switch(mode_rs485[iModeIdx]->mode) {
  case SETTINGS_IP_PORT_MODBUS_GW:
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
    return (mode_rs485[iModeIdx]->state == MODE_STATE_CONNECTED);
  case SETTINGS_IP_PORT_MODBUS_GW_UDP:
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
    return (mode_rs485[iModeIdx]->count != 0);
  default:
    break;
  }
  return false;
}

//Загрузка уставок порта
void LoadSettingsSerial_ByIdx(int iSettingIdx) {
  if ((iSettingIdx < 0) || (iSettingIdx >= RS_PORTS_NUMBER))
    return;
  struct uart_port_s *port;
  USART_TypeDef *usart = mode_rs485[iSettingIdx]->usart;
  usart_set_enable(usart, 0);

  usart_set_tx_enable(usart, 0);
  usart_set_rx_enable(usart, 0);

  usart_set_dma_tx_enable(usart, 0);
  usart_set_dma_rx_enable(usart, 0);

  port = &(rsettings->uart[iSettingIdx]);

  switch(port->databits) {
  case 7:
  case 8:
  case 9:
    usart_set_databits(usart, port->databits);
    break;
  default:
    usart_set_databits(usart, 8);
    break;
  }

  switch(port->stopbits) {
  case SETINGS_UART_STOPBITS_1:
    usart_set_stopbits(usart, 0x0100);
    break;
  case SETINGS_UART_STOPBITS_1_5:
    usart_set_stopbits(usart, 0x0105);
    break;
  case SETINGS_UART_STOPBITS_2:
    usart_set_stopbits(usart, 0x0200);
    break;
  default:
    usart_set_stopbits(usart, 0x0100);
      break;
  }

  switch(port->parity) {
  case SETTINGS_UART_PARITY_ODD:
    if(port->databits == 7) usart_set_databits(usart, 8);
    else if (port->databits == 8) usart_set_databits(usart, 9);
    usart_set_parity(usart,USART_PARITY_ODD);
    break;
  case SETTINGS_UART_PARITY_EVEN:
    if(port->databits == 7) usart_set_databits(usart, 8);
    else if (port->databits == 8) usart_set_databits(usart, 9);
    usart_set_parity(usart,USART_PARITY_EVEN);
    break;
  case SETTINGS_UART_PARITY_NOT:
  default:
    usart_set_parity(usart,USART_PARITY_NOT);
    break;
  }

  usart_set_dma_tx_enable(usart, 1);
  usart_set_dma_rx_enable(usart, 1);

  usart_set_tx_enable(usart, 1);
  usart_set_rx_enable(usart, 1);

  usart_set_baudrate(usart, port->baudrate);

  usart_set_enable(usart, 1);
}

//Загрузка уставок порта по IP
void LoadSettingsSerialIP_ByIdx(int iSettingIdx) {
  if ((iSettingIdx < 0) || (iSettingIdx >= RS_PORTS_NUMBER))
    return;
  struct mode_struct_s *mode = mode_rs485[iSettingIdx];
  struct ip_port_s   *ip_port = &(rsettings->ip_port[iSettingIdx]);

  mode->cport    = ip_port->port_local;
  mode->sport    = ip_port->port_conn;
  mode->ip       = ip_port->ip;
  mode->mode     = ip_port->type;
  mode->timeout  = ip_port->timeout;
}

void settings_load_serial_mode(int iSettingIdx)
{
  if ((iSettingIdx < 0) || (iSettingIdx >= RS_PORTS_NUMBER))
    return;
  struct mode_struct_s *mode = mode_rs485[iSettingIdx];
  USART_TypeDef *usart;
  struct uart_port_s *port;
  struct ip_port_s   *ip_port;

  usart = mode->usart;

  /** init rcc usart & gpio */
  usart_set_rcc_usart_enable(usart, 1);
  usart_set_rcc_gpio_enable(usart, 1);

  port        = &(rsettings->uart[iSettingIdx]);
  ip_port     = &(rsettings->ip_port[iSettingIdx]);

  gpio_init_pp(mode->gpio_re, mode->pin_re);
  gpio_init_pp(mode->gpio_led, mode->pin_led);
  gpio_set_pp(mode->gpio_led, mode->pin_led, 1);

  usart_set_gpio(usart, mode->gpio_tx, mode->pin_tx);
  usart_set_gpio(usart, mode->gpio_rx, mode->pin_rx);

  mode->cport    = ip_port->port_local;
  mode->sport    = ip_port->port_conn;
  mode->ip       = ip_port->ip;
  mode->mode     = ip_port->type;
  mode->timeout  = ip_port->timeout;

  mode->dma_recv = dma_alloc(usart, DMA_DIR_FROM_PERIPHERY);
  mode->dma_send = dma_alloc(usart, DMA_DIR_TO_PERIPHERY);

  switch(port->databits) {
  case 7:
  case 8:
  case 9:
    usart_set_databits(usart, port->databits);
    break;
  default:
    usart_set_databits(usart, 8);
    break;
  }

  switch(port->stopbits) {
  case SETINGS_UART_STOPBITS_1:
    usart_set_stopbits(usart, 0x0100);
    break;
  case SETINGS_UART_STOPBITS_1_5:
    usart_set_stopbits(usart, 0x0105);
    break;
  case SETINGS_UART_STOPBITS_2:
    usart_set_stopbits(usart, 0x0200);
    break;
  default:
    usart_set_stopbits(usart, 0x0100);
    break;
  }

  switch(port->parity) {
  case SETTINGS_UART_PARITY_ODD:
    if(port->databits == 7) usart_set_databits(usart, 8);
    else if (port->databits == 8) usart_set_databits(usart, 9);
    usart_set_parity(usart, USART_PARITY_ODD);
    break;
  case SETTINGS_UART_PARITY_EVEN:
    if(port->databits == 7) usart_set_databits(usart, 8);
    else if (port->databits == 8) usart_set_databits(usart, 9);
    usart_set_parity(usart, USART_PARITY_EVEN);
    break;
  case SETTINGS_UART_PARITY_NOT:
  default:
    usart_set_parity(usart,USART_PARITY_NOT);
    break;
  }

  usart_set_dma_tx_enable(usart, 0);
  usart_set_dma_rx_enable(usart, 0);

  usart_set_tx_enable(usart, 1);
  usart_set_rx_enable(usart, 1);

  usart_set_baudrate(usart, port->baudrate);

  usart_set_enable(usart, 1);

  mode_init(mode);
}

