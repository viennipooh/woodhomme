/**
* @file
* Ethernet Interface
*
*/


#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
//#include "lwip/timers.h"
#include "netif/etharp.h"
#include "err.h"
#include "ethernetif.h"

#include "main.h"
//#include "stm32f4x7_eth.h"
#include <string.h>
#include "settings.h"
#include "eth_mac.h"

#include "info.h"
#include "semphr.h"
#include "timeouts.h"

#define netifMTU                          (MAX_MTU_SIZE)   //( 1500 )
#define netifINTERFACE_TASK_STACK_SIZE		( 512 )
#define netifINTERFACE_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
//configMAX_PRIORITIES
#define netifGUARD_BLOCK_TIME			( 250 )
/* The time to block waiting for input. */
#define emacBLOCK_TIME_WAITING_FOR_INPUT	( ( portTickType ) 100 )

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

extern eth_mac ETH_MAC_(0);
eth_mac * mac = &ETH_MAC_(0);

static eth_mac_addr addr/* = 
{
  {
    MAC_ADDR0,
    MAC_ADDR1,
    MAC_ADDR2,
    MAC_ADDR3,
    MAC_ADDR4,
    MAC_ADDR5
  }
}*/;

static struct netif *s_pxNetIf = NULL;
xSemaphoreHandle s_xSemaphore = NULL;

static void ethernetif_input( void * pvParameters );
static void arp_timer(void *arg);


volatile uint32_t eth_mac_recv_event = 0;
volatile uint32_t eth_mac_send_event = 0;

volatile uint32_t size=0;

static void eth_mac_signal_event_handler( uint32_t event )
{
  switch( event )
  {
    case ETH_MAC_EVENT_RX_FRAME:
      {
        portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
        eth_mac_recv_event++;
        xSemaphoreGiveFromISR( s_xSemaphore, &xHigherPriorityTaskWoken );   
        /* Switch tasks if necessary. */	
        if( xHigherPriorityTaskWoken != pdFALSE )
        {
          portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
        }
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

/**
* In this function, the hardware should be initialized.
* Called from ethernetif_init().
*
* @param netif the already initialized lwip network interface structure
*        for this ethernetif
*/
static void low_level_init(struct netif *netif)
{
//  struct info_s info;

  /* set netif MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  
  /* set netif MAC hardware address */
  netif->hwaddr[0] =  rsettings->mac[0];//MAC_ADDR0;
  netif->hwaddr[1] =  rsettings->mac[1];//MAC_ADDR1;
  netif->hwaddr[2] =  rsettings->mac[2];//MAC_ADDR2;
  netif->hwaddr[3] =  rsettings->mac[3];//MAC_ADDR3;
  netif->hwaddr[4] =  rsettings->mac[4];//MAC_ADDR4;
  netif->hwaddr[5] =  rsettings->mac[5];//MAC_ADDR5;
  
  /* set netif maximum transfer unit */
  // !!! Уменьшим максимальный размер кадра (чтобы он проходил через SW2)
  netif->mtu = MAX_MTU_SIZE; //1500;

  /* Accept broadcast address and ARP traffic */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  s_pxNetIf =netif;

  /* create binary semaphore used for informing ethernetif of frame reception */
  if (s_xSemaphore == NULL)
  {
    vSemaphoreCreateBinary(s_xSemaphore);
    xSemaphoreTake( s_xSemaphore, 0);
  }



  
  /* create the task that handles the ETH_MAC */
  xTaskCreate(ethernetif_input, (signed char*) "Eth_if", netifINTERFACE_TASK_STACK_SIZE, NULL,
              netifINTERFACE_TASK_PRIORITY,NULL);

  mac->initialize(eth_mac_signal_event_handler);
  memcpy(&addr,netif->hwaddr,6);
  mac->set_mac_address(&addr);
    /* Enable MAC and DMA transmission and reception */
  //ETH_Start();   
  
}

static void ethernet_watchdog(void)
{
	/* When Rx Buffer unavailable flag is set: clear it and resume reception */
	if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
	{
		/* Clear RBUS ETHERNET DMA flag */
		ETH->DMASR = ETH_DMASR_RBUS;

		/* Resume DMA reception. The register doesn't care what you write to it. */
		ETH->DMARPDR = 0;
	}
} 

/**
* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
* @param netif the lwip network interface structure for this ethernetif
* @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
*
* @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
*       strange results. You might consider waiting for space in the DMA queue
*       to become availale since the stack doesn't retry to send a packet
*       dropped because of memory failure (except for the TCP timers).
*/

#ifdef USE_STP
  // !!! Подстановка TailTag
  #include "stp.h"
  extern bool IsTailTag(int8_t iBridgeIdx);
  int sTxCount = 0;
  extern ksz8895fmq_read_dmac_t dmac_table[][1000];
  extern int dmac_table_size[];
//  // + Для отладки: для проверок кадров по Ethernet-адресам
//  extern const struct eth_addr ethrstp;
//  extern const struct eth_addr ethbroadcast;
//  // -
#endif
static uint8_t buffer[1522+ 64]={0,};
// Раздельные буферы Rx и Tx
#ifdef LWIP_SEPARATE_RX_TX_BUFFERS
  static uint8_t tbuffer[1522+ 64]={0,};
#else
  uint8_t * tbuffer = buffer;
#endif
err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  static xSemaphoreHandle xTxSemaphore = NULL;
  struct pbuf *q;
  uint16_t framelength = 0;
  uint32_t bufferoffset = 0;
  uint32_t byteslefttocopy = 0;
  uint32_t payloadoffset = 0;

  if (xTxSemaphore == NULL)
  {
    vSemaphoreCreateBinary (xTxSemaphore);
  }

  if (xSemaphoreTake(xTxSemaphore, netifGUARD_BLOCK_TIME))
  {
    
    bufferoffset = 0;
    
    for(q = p; q != NULL; q = q->next) 
    {
      /* Get bytes in current lwIP buffer  */
      byteslefttocopy = q->len;
      payloadoffset = 0;
      
      memcpy(&tbuffer[bufferoffset],(u8_t*)((u8_t*)q->payload + payloadoffset),byteslefttocopy);
      
      /* Copy the remaining bytes */
      bufferoffset = bufferoffset + byteslefttocopy;
      framelength = framelength + byteslefttocopy;
      
    }
    
#if (ENABLE_TAIL_TAG)
    // !!! Подстановка TailTag
    struct eth_hdr* ethhdr = (struct eth_hdr *)p->payload;
    bool cRstp = (memcmp(&tbuffer[0], &ethrstp.addr, ETH_HWADDR_LEN) == 0);
//    // + !== Для отладки
//    struct ethrstp_hdr * c_ethrstp_hdr = (struct ethrstp_hdr *)p->payload;
//    if (cRstp) {
//      c_ethrstp_hdr = (struct ethrstp_hdr *)p->payload;
//    }
//    // - !== Для отладки
    if (IsTailTag(0) && !cRstp) { //Если TailTag и dst != ethrstp, то:
      // + !== Для отладки. Подсчет передаваемых ARP кадров
      if (ethhdr->type == PP_HTONS(ETHTYPE_ARP)) { //0x0608
        ++sTxCount;
      }
      // - !== Для отладки.
      if (framelength < MIN_FRAME_SIZE) { //Добавление pad до MIN_FRAME_SIZE байт
        memset(&tbuffer[framelength], 0, MIN_FRAME_SIZE - framelength);
        framelength = MIN_FRAME_SIZE;
      }
      //Определение порта назначения для кадра
      uint8_t cPort = 0x0f; //Все порты
      for (int i = 0; i < dmac_table_size[0]; ++i) {
        if (memcmp(&tbuffer[0], &dmac_table[0][i].mac[0], ETH_HWADDR_LEN) == 0) {
          //Если есть запись в dmac_table, послать в этот порт
          cPort = (1 << dmac_table[0][i].port);
          break;
        }
      }
      tbuffer[framelength] = cPort; //0x0f; //
      ++framelength;
    }
    // + !== Для проверки, есть ли кадры < MIN_FRAME_SIZE
    if (framelength < MIN_FRAME_SIZE) {
      ethhdr = (struct eth_hdr *)p->payload;
    }
    // - !==
#endif
    // !!!
    if(mac->send_frame(tbuffer,framelength,0) != DRIVER_OK)
    {
#ifdef REPEAT_TX_ON_TX_ERROR
  #ifdef REPEAT_TX_ON_TX_ERROR_BY_RTOS
      vTaskDelay(1);
  #else
      volatile uint32_t cCounter = 200;
      while (cCounter)
        --cCounter;
  #endif
      mac->send_frame(tbuffer,framelength,0);
#endif
      goto error;
    }

    /* Give semaphore and exit */
  error:

    xSemaphoreGive(xTxSemaphore);
  }

  return ERR_OK;
}

/**
* Should allocate a pbuf and transfer the bytes of the incoming
* packet from the interface into the pbuf.
*
* @param netif the lwip network interface structure for this ethernetif
* @return a pbuf filled with the received packet (including MAC header)
*         NULL on memory error
*/
static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p= NULL, *q;
  u32_t len;
  //FrameTypeDef frame;
  uint32_t bufferoffset = 0;
  uint32_t payloadoffset = 0;
  uint32_t byteslefttocopy = 0;

  /* get received frame */
  //frame = ETH_Get_Received_Frame_interrupt();
//        while(size = mac->get_rx_frame_size())
//        {
//          mac->read_frame(buffer,size);
//        }
  /* Obtain the size of the packet and put it into the "len" variable. */
  len = mac->get_rx_frame_size();
  
  if (len > 0)
  {
    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
    p = pbuf_alloc(PBUF_RAW, len+16, PBUF_POOL);
  }
  
  if (p != NULL)
  {
    bufferoffset = 0;
    mac->read_frame(buffer,len);
    for(q = p; q != NULL; q = q->next)
    {
      byteslefttocopy = q->len;
      if(len<byteslefttocopy)
      {
        byteslefttocopy = len;
      }
      len-=byteslefttocopy;
      payloadoffset = 0;
      //mac->read_frame((u8_t*)((u8_t*)q->payload + payloadoffset),byteslefttocopy);
      memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), (u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
      /* Copy remaining data in pbuf */
      //memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), (u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
      bufferoffset = bufferoffset + byteslefttocopy;
    }
  }
  return p;
}
//uint8_t buffer_output[1500];
//static err_t low_level_output(struct netif *netif, struct pbuf *p)
//{
//    static xSemaphoreHandle xTxSemaphore = NULL;
//    struct pbuf *q;
//    __IO ETH_DMADESCTypeDef *DmaTxDesc;
//    u8 * buffer;
//    uint32_t l = 0, mac_send_ok_cntr = 0;
//
//    if (xTxSemaphore == NULL)
//    {
//        vSemaphoreCreateBinary (xTxSemaphore);
//    }
//    
//    if (xSemaphoreTake(xTxSemaphore, netifGUARD_BLOCK_TIME))
//    {
//        /* Get bytes in current lwIP buffer  */
//        for(q = p; q != NULL; q = q->next) 
//        {
//            memcpy((u8_t*)&buffer_output[l], q->payload, q->len);
//            l = l + q->len;
//        }
//        
//        if(mac->send_frame(buffer_output,l,0) != DRIVER_OK)
//            mac_send_ok_cntr++;
//                
//        /* Give semaphore and exit */
//        xSemaphoreGive(xTxSemaphore);
//    }
//    
//    pbuf_free(q);
//    return ERR_OK;
//}
///**
//* Should allocate a pbuf and transfer the bytes of the incoming
//* packet from the interface into the pbuf.
//*
//* @param netif the lwip network interface structure for this ethernetif
//* @return a pbuf filled with the received packet (including MAC header)
//*         NULL on memory error
//*/
//static struct pbuf * low_level_input(struct netif *netif)
//{
//  struct pbuf *p= NULL, *q;
//  u32_t len;
//  FrameTypeDef frame;
//  u8 *buffer;
//  __IO ETH_DMADESCTypeDef *DMARxDesc;
//  uint32_t bufferoffset = 0;
//  uint32_t payloadoffset = 0;
//  uint32_t byteslefttocopy = 0;
//  uint32_t i=0;  
//
//  /* get received frame */
//  //frame = ETH_Get_Received_Frame_interrupt();
////        while(size = mac->get_rx_frame_size())
////        {
////          mac->read_frame(buffer,size);
////        }
//  /* Obtain the size of the packet and put it into the "len" variable. */
//  len = mac->get_rx_frame_size();
//  buffer = (u8 *)frame.buffer;
//  
//  if (len > 0)
//  {
//    /* We allocate a pbuf chain of pbufs from the Lwip buffer pool */
//    p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
//  }
//  
//  if (p != NULL)
//  {
//    DMARxDesc = frame.descriptor;
//    bufferoffset = 0;
//    for(q = p; q != NULL; q = q->next)
//    {
//      byteslefttocopy = q->len;
//      payloadoffset = 0;
//      
//      mac->read_frame((u8_t*)((u8_t*)q->payload + payloadoffset),byteslefttocopy);
//      
//      /* Copy remaining data in pbuf */
//      //memcpy( (u8_t*)((u8_t*)q->payload + payloadoffset), (u8_t*)((u8_t*)buffer + bufferoffset), byteslefttocopy);
//      //bufferoffset = bufferoffset + byteslefttocopy;
//    }
//  }
//  return p;
//}


/**
* This function is the ethernetif_input task, it is processed when a packet 
* is ready to be read from the interface. It uses the function low_level_input() 
* that should handle the actual reception of bytes from the network
* interface. Then the type of the received packet is determined and
* the appropriate input function is called.
*
* @param netif the lwip network interface structure for this ethernetif
*/
void ethernetif_input( void * pvParameters )
{
  struct pbuf *p;
  
  for( ;; )
  {
    if (xSemaphoreTake( s_xSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE)
    {
TRY_GET_NEXT_FRAME:
      p = low_level_input( s_pxNetIf );
      if   (p != NULL)
      {
        if (ERR_OK != s_pxNetIf->input( p, s_pxNetIf))
        {
          pbuf_free(p);
          p = NULL;
        }
        else
        {
          goto TRY_GET_NEXT_FRAME;
        }
      }
    }
    else
    {
      while(DRIVER_OK == mac->read_frame(buffer,mac->get_rx_frame_size()));
//      do
//			{   
//				p = low_level_input( s_pxNetIf );
//				if (p != NULL)
//				{
//					if (s_pxNetIf->input( p, s_pxNetIf) != ERR_OK )
//					{
//						pbuf_free(p);
//						p = NULL;
//					}
//				}
//			} while(p!=NULL);
//      if(mac->get_rx_frame_size())
//      {
//      }
//      else
//      {
//        p = low_level_input( s_pxNetIf );
//        if   (p != NULL)
//        {
//          if (ERR_OK != s_pxNetIf->input( p, s_pxNetIf))
//          {
//            pbuf_free(p);
//          }
//          else
//          {
//            goto TRY_GET_NEXT_FRAME;
//          }
//        }
//      }
    }
    ethernet_watchdog();
  }
}
//void ethernetif_input( void * pvParameters )
//{
//  struct pbuf *p;
//  
//  for( ;; )
//  {
//    if (xSemaphoreTake( s_xSemaphore, emacBLOCK_TIME_WAITING_FOR_INPUT)==pdTRUE)
//    {
//GET_NEXT_FRAGMENT:      
//      p = low_level_input( s_pxNetIf );
//      if (p != NULL) 
//      {      
//          if (ERR_OK != s_pxNetIf->input( p, s_pxNetIf))
//          {
//            pbuf_free(p);
//            p=NULL;
//          }
//          else 
//          {
//            xSemaphoreTake(s_xSemaphore, 0);
//            goto GET_NEXT_FRAGMENT;
//          }
//      }
//    }
//  }
//}  
//      
/**
* Should be called at the beginning of the program to set up the
* network interface. It calls the function low_level_init() to do the
* actual setup of the hardware.
*
* This function should be passed as a parameter to netif_add().
*
* @param netif the lwip network interface structure for this ethernetif
* @return ERR_OK if the loopif is initialized
*         ERR_MEM if private data couldn't be allocated
*         any other err_t on error
*/
err_t ethernetif_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;

  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* initialize the hardware */
  low_level_init(netif);

  etharp_init();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL); 

  return ERR_OK;
}


static void arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

