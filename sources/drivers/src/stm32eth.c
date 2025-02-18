/**
  ******************************************************************************
  * @file    stm32eth.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    06.02.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "stm32eth.h"
#include "stm32f4xx.h"
#include "stm32eth_regs.h"

#include "sch.h"

static unsigned int stm32eth_init_en = 0;
static stm32eth_callback_t _mac_event = 0;

/**
 * @brief Ethernet descriptors
 */
static volatile ethernet_descriptor_t descriptor_receive[ETHERNET_DESCRIPTOR_RECEIVE_COUNT];
static volatile ethernet_descriptor_t descriptor_transmit[ETHERNET_DESCRIPTOR_TRANSMIT_COUNT];
static volatile ethernet_descriptor_t *descriptor_receive_p = descriptor_receive;
static volatile ethernet_descriptor_t *descriptor_transmit_p = descriptor_transmit;

/**
 * @brief Ethernet buffers receive_buffer and transmit_bufer
 */
static uint8_t receive_buffer[ETHERNET_DESCRIPTOR_RECEIVE_COUNT][ETHERNET_PACKET_SIZE];
static uint8_t transmit_bufer[ETHERNET_DESCRIPTOR_TRANSMIT_COUNT][ETHERNET_PACKET_SIZE];


//
//  Functions
//
static int eth_mac_init_rcc()
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | 
                  RCC_AHB1ENR_GPIOBEN | 
                  RCC_AHB1ENR_GPIOCEN;
  
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST;
  
  RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOEEN;
  RCC->AHB1ENR  |= RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN;
  return 0;
}

static int eth_mac_deinit_rcc()
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | 
                  RCC_AHB1ENR_GPIOBEN | 
                  RCC_AHB1ENR_GPIOCEN;
  
  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST;
  
  RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN | RCC_AHB1ENR_GPIOEEN;
  RCC->AHB1ENR  |= RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN | RCC_AHB1ENR_ETHMACRXEN;
  return 0;
}


static void eth_mac_init_gpio( void )
{
  gpio_init(ETH_MII_CRS,          GPIO_ETH_MII);
  gpio_init(ETH_MII_COL,          GPIO_ETH_MII);
  gpio_init(ETH_MII_RX_DV,        GPIO_ETH_MII);
  //gpio_init(ETH_MII_RX_ER,        GPIO_ETH_MII);
  gpio_init(ETH_MII_TX_EN,        GPIO_ETH_MII);

  gpio_init(ETH_MII_RX_CLK,       GPIO_ETH_MII);
  gpio_init(ETH_MII_RXD0,         GPIO_ETH_MII);
  gpio_init(ETH_MII_RXD1,         GPIO_ETH_MII);
  gpio_init(ETH_MII_RXD2,         GPIO_ETH_MII);
  gpio_init(ETH_MII_RXD3,         GPIO_ETH_MII);

  gpio_init(ETH_MII_TX_CLK,       GPIO_ETH_MII);
  gpio_init(ETH_MII_TXD0,         GPIO_ETH_MII);
  gpio_init(ETH_MII_TXD1,         GPIO_ETH_MII);
  gpio_init(ETH_MII_TXD2,         GPIO_ETH_MII);
  gpio_init(ETH_MII_TXD3,         GPIO_ETH_MII);

  gpio_init(ETH_MDIO,             GPIO_ETH_MII);
  gpio_init(ETH_MDC,              GPIO_ETH_MII);
}

static void eth_mac_deinit_gpio( void )
{
  gpio_init(ETH_MII_CRS,          GPIO_NONE);
  gpio_init(ETH_MII_COL,          GPIO_NONE);
  gpio_init(ETH_MII_RX_DV,        GPIO_NONE);
  gpio_init(ETH_MII_RX_ER,        GPIO_NONE);
  gpio_init(ETH_MII_TX_EN,        GPIO_NONE);

  gpio_init(ETH_MII_RX_CLK,       GPIO_NONE);
  gpio_init(ETH_MII_RXD0,         GPIO_NONE);
  gpio_init(ETH_MII_RXD1,         GPIO_NONE);
  gpio_init(ETH_MII_RXD2,         GPIO_NONE);
  gpio_init(ETH_MII_RXD3,         GPIO_NONE);

  gpio_init(ETH_MII_TX_CLK,       GPIO_NONE);
  gpio_init(ETH_MII_TXD0,         GPIO_NONE);
  gpio_init(ETH_MII_TXD1,         GPIO_NONE);
  gpio_init(ETH_MII_TXD2,         GPIO_NONE);
  gpio_init(ETH_MII_TXD3,         GPIO_NONE);

  gpio_init(ETH_MDIO,             GPIO_NONE);
  gpio_init(ETH_MDC,              GPIO_NONE);
}

static int eth_mac_init_desc()
{
  uint32_t index = 0;
  
  for( index = 0 ; index < ETHERNET_DESCRIPTOR_TRANSMIT_COUNT ; index++ )
  {
    descriptor_transmit[index].status                    = DESCRIPTOR_TRANSMIT_STATUS_CTRL_TCH | DESCRIPTOR_TRANSMIT_STATUS_CTRL_IC;
  
    descriptor_transmit[index].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_HARDWARE;
    
    descriptor_transmit[index].buffer_count              = ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
    descriptor_transmit[index].buffer_address            = (uint32_t)transmit_bufer[index];
    
    if(index == ETHERNET_DESCRIPTOR_RECEIVE_COUNT - 1)
    {
      descriptor_transmit[index].buffer_next_address       = (uint32_t)&descriptor_transmit[0];
    }
    else
    {
      descriptor_transmit[index].buffer_next_address       = (uint32_t)&descriptor_transmit[index+1];
    }
#ifdef defined(ETHERNET_DESCRIPTOR_EXTENDED) == 1
    descriptor_transmit[index].status_extended           = 0;
    descriptor_transmit[index].reserved1                 = 0;
    descriptor_transmit[index].time_stamp_low            = 0;
    descriptor_transmit[index].time_stamp_high           = 0;
#endif
  }
  
  for( index = 0 ; index < ETHERNET_DESCRIPTOR_RECEIVE_COUNT ; index++ )
  {
    descriptor_receive[index].status                    = DESCRIPTOR_RECEIVE_STATUS_OWN;
    descriptor_receive[index].buffer_count              = DESCRIPTOR_BUFFER_COUNT_RCH | ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
      
    if(index == ETHERNET_DESCRIPTOR_RECEIVE_COUNT - 1)
    {
      descriptor_receive[index].buffer_next_address       = (uint32_t)&descriptor_receive[0];
    }
    else
    {
      descriptor_receive[index].buffer_next_address       = (uint32_t)&descriptor_receive[index+1];
    }
    descriptor_receive[index].buffer_address            = (uint32_t)receive_buffer[index];
#if defined(ETHERNET_DESCRIPTOR_EXTENDED) && ETHERNET_DESCRIPTOR_EXTENDED == 1
    descriptor_receive[index].status_extended           = 0;
    descriptor_receive[index].reserved1                 = 0;
    descriptor_receive[index].time_stamp_low            = 0;
    descriptor_receive[index].time_stamp_high           = 0;
#endif
  }
  
  return 0;
}

static int eth_mac_deinit_desc()
{
  return 0;
}

static int eth_mac_init_regs()
{
   
  /**
   * ETHERNET init
   */
  //
  ETH->MACMIIAR = ETH_MACMIIAR_CR_Div102;
  //ETH->MACFCR   = ETH_MACFFR_RA | ETH_MACFFR_PCF_ForwardAll | ETH_MACFFR_PM;
  //ETH->MACMIIAR = (ETH_MACMIIAR_PA | 0x01) | ETH_MACMIIAR_MR | ETH_MACMIIAR_CR_Div102;
  //
  
  ETH->MACCR    = /*ETH_MACCR_TE | ETH_MACCR_RE | */ETH_MACCR_IPCO | ETH_MACCR_FES | ETH_MACCR_DM/*| ETH_MACCR_LM */ | ETH_MACCR_IFG_64Bit/* | ETH_MACCR_BL_1*/;
  ETH->MACFFR   = ETH_MACFFR_RA | ETH_MACFFR_PCF_ForwardAll | ETH_MACFFR_PAM | ETH_MACFFR_PM;
  
  //ETH->MACHTHR  = 0x00;
  //ETH->MACHTLR  = 0x00;
  
  //ETH->MACFCR   = 0x00;
  
  ETH->DMARDLAR = (uint32_t)descriptor_receive;
  ETH->DMATDLAR = (uint32_t)descriptor_transmit;
  ETH->MACA0LR  = 0x12345678;
  ETH->MACA0HR  = 0x00000330;
  
  
    /* Enable transmit state machine of the MAC for transmission on the MII */  
  ETH->MACCR   |= ETH_MACCR_TE;//ETH_MACTransmissionCmd(ENABLE);
  /* Flush Transmit FIFO */
  ETH->DMAOMR  |= ETH_DMAOMR_FTF | ETH_DMAOMR_TSF; // ETH_FlushTransmitFIFO();
  /* Enable receive state machine of the MAC for reception from the MII */  
  ETH->MACCR |= ETH_MACCR_RE;  //ETH_MACReceptionCmd(ENABLE);
 
  /* Start DMA transmission */
  ETH->DMAOMR |= ETH_DMAOMR_ST; //ETH_DMATransmissionCmd(ENABLE); 
  /* Start DMA reception */
  ETH->DMAOMR |= ETH_DMAOMR_SR; //ETH_DMAReceptionCmd(ENABLE);  
  
  
   
  //ETH->DMAOMR   = ETH_DMAOMR_SR | ETH_DMAOMR_ST;
  
//  (void)ETH->DMABMR;
//  ETH->DMABMR   = ETH_DMABMR_SR;
//  ETH->DMABMR   = ETH_DMABMR_USP | ETH_DMABMR_RDP_4xPBL_32Beat | ETH_DMABMR_PBL_4xPBL_32Beat | ETH_DMABMR_RTPR_1_1;
//  ETH->DMAIER   = ETH_DMAIER_NISE | ETH_DMAIER_TIE | ETH_DMAIER_RIE;
//  
//  ETH->DMARDLAR = (uint32_t)descriptor_receive;
//  ETH->DMATDLAR = (uint32_t)descriptor_transmit;
//  
//  ETH->DMARPDR  = (uint32_t)descriptor_receive;
//  ETH->DMATPDR = (uint32_t)descriptor_transmit;
//  
//  ETH->MACA0LR  = 0x12345678;
//  ETH->MACA0HR  = 0x00000030;
//  
//  ETH->MACCR    = ETH_MACCR_TE | ETH_MACCR_RE | ETH_MACCR_FES | ETH_MACCR_DM | ETH_MACCR_IFG_64Bit | ETH_MACCR_BL_1;
//  
//  ETH->DMAOMR   = ETH_DMAOMR_SR;
//  ETH->DMAOMR  |= ETH_DMAOMR_ST;
//  
  
  ETH->DMAIER = ETH_DMAIER_NISE/* | ETH_DMAIER_AISE*/;
  ETH->DMAIER |= ETH_DMAIER_RIE | ETH_DMAIER_TIE;
  
  NVIC->IP[((uint32_t)(int32_t)ETH_IRQn)]               = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
  //NVIC_SetPriority (ETH_IRQn, (1<<__NVIC_PRIO_BITS) - 1);  /* set Priority for Systick Interrupt */
  NVIC->ISER[(((uint32_t)(int32_t)ETH_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)ETH_IRQn) & 0x1FUL));
  //NVIC_EnableIRQ(ETH_IRQn);
  return 0;
}


/**
 * @brief stm32eth initialization
 *
 * @retval none.
 *
 */
void stm32eth_init( void )
{
  eth_mac_init_rcc();
  eth_mac_init_gpio();
  eth_mac_init_desc();
  eth_mac_init_regs();
  
}

/**
 * @brief stm32eth deinitialization
 *
 * @retval none.
 *
 */
void stm32eth_deinit( void )
{
}


static int32_t eth_mac_read_frame(uint8_t *frame, uint32_t len)
{
  if((frame) && !(descriptor_receive_p->status & DESCRIPTOR_RECEIVE_STATUS_OWN))
  {
    memcpy(frame,(void*)(descriptor_receive_p->buffer_address),len);
    descriptor_receive_p->status = DESCRIPTOR_RECEIVE_STATUS_OWN;
    descriptor_receive_p = (ethernet_descriptor_t*)(descriptor_receive_p->buffer_next_address);
  } else return 0;
  return 1;
}

static int32_t eth_mac_send_frame(const uint8_t *frame, uint32_t len, uint32_t flags)
{
  if((frame) && !(descriptor_transmit_p->status & DESCRIPTOR_TRANSMIT_STATUS_OWN))
  {
    memcpy((void*)(descriptor_transmit_p->buffer_address),frame,len);
    descriptor_transmit_p->buffer_count = len & DESCRIPTOR_BUFFER_COUNT_TBS1;
    descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
    //descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_HARDWARE;
    //descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_IC;
    descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
    descriptor_transmit_p = (ethernet_descriptor_t*)(descriptor_transmit_p->buffer_next_address);
    
     /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if (ETH->DMASR & ETH_DMASR_TBUS)
    {
      /* Clear TBUS ETHERNET DMA flag */
      ETH->DMASR = ETH_DMASR_TBUS;
      /* Resume DMA transmission*/
      ETH->DMATPDR = 0;
    }
     /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if ((ETH->DMASR & ETH_DMASR_RBUS) != (u32)RESET)
    {
      /* Clear TBUS ETHERNET DMA flag */
      ETH->DMASR = ETH_DMASR_RBUS;
      /* Resume DMA transmission*/
      ETH->DMATPDR = 0;
    }
    
    
  } else return 0;
  return 1;
}

static uint32_t eth_mac_get_rx_frame_size(void)
{
  uint32_t len = 0;
  if(!(descriptor_receive_p->status & DESCRIPTOR_RECEIVE_STATUS_OWN))
  {
    len = ((descriptor_receive_p->status & DESCRIPTOR_RECEIVE_STATUS_FL) >> 16);
  }
  return len;
}

void stm32eth_send(void *buffer, unsigned long len)
{
  eth_mac_send_frame(buffer, len, 0);
}

void stm32eth_recv(void *buffer, unsigned long len)
{
  eth_mac_read_frame(buffer, len);
}

void stm32eth_next_buffer( void )
{
  if(!(descriptor_receive_p->status & DESCRIPTOR_RECEIVE_STATUS_OWN))
  {
    descriptor_receive_p->status = DESCRIPTOR_RECEIVE_STATUS_OWN;
    descriptor_receive_p = (ethernet_descriptor_t*)(descriptor_receive_p->buffer_next_address);
  }
}

uint32_t stm32eth_currient_buffer(void **address)
{
  *address = (void*)(descriptor_receive_p->buffer_address);
  return eth_mac_get_rx_frame_size();
}


void stm32eth_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
{
  ETH->MACMIIAR &= ~(ETH_MACMIIAR_MR | ETH_MACMIIAR_PA | ETH_MACMIIAR_MB | ETH_MACMIIAR_MW);
  ETH->MACMIIAR |= (phy_addr << 11) & ETH_MACMIIAR_PA;
  ETH->MACMIIAR |= (reg_addr << 6)  & ETH_MACMIIAR_MR;
  ETH->MACMIIAR |= ETH_MACMIIAR_MB; // read start
  
  if(data)
  {
    do {}
    while(ETH->MACMIIAR & ETH_MACMIIAR_MB > 0); // wait for read
    *data = ETH->MACMIIDR; // read data
  }
}

void stm32eth_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
{
  ETH->MACMIIAR &= ~(ETH_MACMIIAR_MR | ETH_MACMIIAR_PA | ETH_MACMIIAR_MB | ETH_MACMIIAR_MW);
  ETH->MACMIIAR |= (phy_addr << 11) & ETH_MACMIIAR_PA;
  ETH->MACMIIAR |= (reg_addr << 6)  & ETH_MACMIIAR_MR;
  ETH->MACMIIDR = data; // write data
  ETH->MACMIIAR |= ETH_MACMIIAR_MW | ETH_MACMIIAR_MB; // write start
  
  if(data)
  {
    do {}
    while(ETH->MACMIIAR & ETH_MACMIIAR_MB > 0); // wait for write
  }
}



void ETH_WKUP_IRQHandler( void )
{
  uint32_t status;
  status = ETH->DMASR;
  if(status & ETH_MACSR_PMTS)
  {
    if(_mac_event)
      _mac_event(ETH_MAC_EVENT_WAKEUP);
    (void)ETH->MACPMTCSR;
  }
}

/**
 * @brief Ethernet interrupt handler
 *
 */
static uint32_t ETH_IRQHandler_stat = 0;
void ETH_IRQHandler( void )
{
  uint32_t status;
  status = ETH->DMASR;
  ETH_IRQHandler_stat++;
  /**
   * @brief ethernet interrupt handler
   */
  //if(status & ETH_DMASR_NIS)
  {
    if(status & ETH_DMASR_TS)
    {
      ETH->DMASR = ETH_DMASR_TS;
      //if(status & ETH_DMASR_TPS)
      {
        if(_mac_event)
          _mac_event(ETH_MAC_EVENT_TX_FRAME);
      }
    }
    if(status & ETH_DMASR_RS)
    {
      ETH->DMASR = ETH_DMASR_RS;
      ETH->DMAIER &= ~ETH_DMAIER_RIE;
      //if(status & ETH_DMASR_RPS)
      {
        if(_mac_event)
          _mac_event(ETH_MAC_EVENT_RX_FRAME);
        if(0)
        {
          ethernet_descriptor_t *temp;
          {
            temp = (ethernet_descriptor_t *)ETH->DMACHRDR;            
            if(!(temp->status & DESCRIPTOR_RECEIVE_STATUS_OWN))
            {
              temp->buffer_count              = DESCRIPTOR_BUFFER_COUNT_RCH | ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
              temp->status                    = DESCRIPTOR_RECEIVE_STATUS_OWN;
            }
          }
        }
        
      }
      ETH->DMAIER |= ETH_DMAIER_RIE;
    }
    /**
     * @brief reset interrupt
     */
    ETH->DMASR = ETH_DMASR_NIS;
  }
  
  /**
   * @brief ethernet interrupt handler
   */
  if(status & ETH_DMASR_AIS)
  {
    /**
     * @brief reset interrupt
     */
    status |= ETH_DMASR_AIS;
  }
}


void stm32eth_set_callback(stm32eth_callback_t cb)
{
  _mac_event = cb;
}