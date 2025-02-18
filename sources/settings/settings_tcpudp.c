#include "settings.h"
#include "modbus_gw.h"
#include "Driver_USART.h"

extern ARM_DRIVER_USART Driver_USART4;
extern ARM_DRIVER_USART Driver_USART6;

static struct modbus_gw_s *mbgw=0;
static void settings_modbus_gw(struct ip_port_s *port, 
                               struct uart_port_s *uart, 
                               ARM_DRIVER_USART *driver)
{
  /**
   * Тута происходит настройка 
   * необходимой конфигурации портов
   * ModBus GateWay/UDP/TCP Client-Server
   */
  mbgw = modbus_gw_create();
  if(mbgw)
  {
    modbus_gw_set_usart(mbgw,&Driver_USART6);
    if(1)
    {
      modbus_gw_set_ip(mbgw,IP_ADDR_ANY);
      modbus_gw_set_port(mbgw,502);
      modbus_gw_set_mode(mbgw,MODBUS_GW_MODE_SERVER);
    }
    else
    {
      // Клиент реализован только в RAW api
      //modbus_gw_set_ip(mbgw,inet_addr("192.168.1.1"));
      modbus_gw_set_port(mbgw,502);
      modbus_gw_set_mode(mbgw,MODBUS_GW_MODE_CLIENT);
    }
    
    Driver_USART6.Control(  ARM_USART_MODE_ASYNCHRONOUS |
                            ARM_USART_DATA_BITS_8 |
                            ARM_USART_PARITY_NONE |
                            ARM_USART_STOP_BITS_1 |
                            ARM_USART_FLOW_CONTROL_NONE, 115200);
    modbus_gw_listen(mbgw);
  }
  
}

