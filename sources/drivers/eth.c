#include "main.h"
#include "eth_mac.h"


extern eth_mac ETH_MAC_(0);
eth_mac * mac = &ETH_MAC_(0);

static eth_mac_addr addr;/* = 
{
  {
    MAC_ADDR0,
    MAC_ADDR1,
    MAC_ADDR2,
    MAC_ADDR3,
    MAC_ADDR4,
    MAC_ADDR5
  }
};*/


volatile uint32_t eth_mac_recv_event = 0;
volatile uint32_t eth_mac_send_event = 0;

volatile uint32_t size=0;
static uint8_t buffer_eth[1530] = {0,};

static void eth_mac_signal_event_handler( uint32_t event )
{
  switch( event )
  {
    case ETH_MAC_EVENT_RX_FRAME:
      {
        while(size = mac->get_rx_frame_size())
        {
          mac->read_frame(buffer_eth,size);
        }
        //eth_recv(buffer, int *len);
        //size = mac->get_rx_frame_size();
        //mac->read_frame(buffer,size);
        eth_mac_recv_event++;
      }
      break;
    case ETH_MAC_EVENT_TX_FRAME:
      {
        eth_mac_send_event++;
      }
      break;
    case ETH_MAC_EVENT_WAKEUP:
      {
      }
      break;
    case ETH_MAC_EVENT_TIMER_ALARM:
      {
      }
      break;
  };
}

int eth_send(void *buff, int len)
{
  if(!buff)
    return -1;
  if(!len)
    return -1;
  mac->send_frame(buff,len,0);
  return 0;
}

int eth_recv(void *buff, int *len)
{
  if(*len = mac->get_rx_frame_size())
  {
    mac->read_frame(buff,*len);
    return 0;
  }
  return -1;
}


int eth_init()
{
  mac->initialize(eth_mac_signal_event_handler);
  addr.b[0] = MAC_ADDR0;
  addr.b[1] = MAC_ADDR1;
  addr.b[2] = MAC_ADDR2;
  addr.b[3] = MAC_ADDR3;
  addr.b[4] = MAC_ADDR4;
  addr.b[5] = MAC_ADDR5;
  
  mac->set_mac_address(&addr);
  
  return 0;
}

int eth_deinit()
{
  mac->uninitialize();
  return 0;
}