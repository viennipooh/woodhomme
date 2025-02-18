#include "string.h"

#include "eth_mac.h"
#include "stm32eth_regs.h"



#define ETH_MAC_DRV_VERSION    DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */

/* Driver Version */
static const driver_version _driver_version = {
    ETH_MAC_API_VERSION,
    ETH_MAC_DRV_VERSION
};

/* Driver Capabilities */
static const eth_mac_capabilities _driver_capabilities = {
    1, /* 1 = IPv4 header checksum verified on receive */
    1, /* 1 = IPv6 checksum verification supported on receive */
    1, /* 1 = UDP payload checksum verified on receive */
    1, /* 1 = TCP payload checksum verified on receive */
    1, /* 1 = ICMP payload checksum verified on receive */
    1, /* 1 = IPv4 header checksum generated on transmit */
    1, /* 1 = IPv6 checksum generation supported on transmit */
    1, /* 1 = UDP payload checksum generated on transmit */
    1, /* 1 = TCP payload checksum generated on transmit */
    1, /* 1 = ICMP payload checksum generated on transmit */
    0, /* Ethernet Media Interface type */
    0, /* 1 = driver provides initial valid MAC address */
    0, /* 1 = callback event \ref ETH_MAC_EVENT_RX_FRAME generated */
    0, /* 1 = callback event \ref ETH_MAC_EVENT_TX_FRAME generated */
    0, /* 1 = wakeup event \ref ETH_MAC_EVENT_WAKEUP generated */
    0  /* 1 = Precision Timer supported */
};

static eth_mac_signalevent_t _mac_event = 0;

//
//  Registers
//
/* Private macro -------------------------------------------------------------*/

#define MODE_IN  0
#define MODE_OUT 1
#define MODE_AF  2
#define MODE_AN  3

#define OTYPE_PP 0
#define OTYPE_OD 1

#define OSPEED_LS 0
#define OSPEED_MS 1
#define OSPEED_HS 2
#define OSPEED_VHS 3

#define PUPD_N   0
#define PUPD_PU  1
#define PUPD_PD  2
#define PUPD_RES 3

#define GPIO_INIT(GPIO,PIN,MODE,OTYPE,OSPEED,PUPD,AF,LOCK) \
do{\
    unsigned char pin=0;\
    unsigned char reg=0;\
  RCC->AHB1ENR |= RCC_AHB1ENR_##GPIO##EN;\
  GPIO->LCKR &= ~(1<<PIN);\
  \
  GPIO->MODER &= ~(3<<(PIN<<1));\
  GPIO->MODER |=  (MODE<<(PIN<<1));\
  \
  GPIO->OTYPER &= ~(1<<PIN);\
  GPIO->OTYPER |=  (OTYPE<<PIN);\
  \
  GPIO->OSPEEDR &= ~(3<<(PIN<<1));\
  GPIO->OSPEEDR |=  (OSPEED<<(PIN<<1));\
  \
  GPIO->PUPDR &= ~(3<<(PIN<<1));\
  GPIO->PUPDR |=  (PUPD<<(PIN<<1));\
  \
  if( (pin=PIN,pin)>7 )\
  {pin-=8;reg=1;}\
  GPIO->AFR[reg] &=  ~(0xF<<(pin<<2));\
  GPIO->AFR[reg] |=   ((0xF&(AF))<<(pin<<2));\
  \
  GPIO->LCKR |= (LOCK<<PIN);\
}while(0);

/**
 * @brief Ethernet descriptors and buffers size
 */
#define ETHERNET_DESCRIPTOR_RECEIVE_COUNT       4
#define ETHERNET_DESCRIPTOR_TRANSMIT_COUNT      4

#define ETHERNET_PACKET_SIZE                    1522



/**
 * @brief Ethernet callback
 */
//static eth_phy_callback eth_phy_cb = 0;

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
//const uint8_t transmit_buffer_test[] = {
//  0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
//  0x01,0x02,0x03,0x04,0x05,0x06,
//  0x00,64,
//  0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,
//  0x00,0x00,0x03,0x00
//};

//
//  Functions
//
static int eth_mac_init_rcc()
{
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                  RCC_AHB1ENR_GPIOBEN |
                  RCC_AHB1ENR_GPIOCEN |
                  RCC_AHB1ENR_GPIOEEN |
                  RCC_AHB1ENR_GPIOGEN;

  RCC->AHB1RSTR |= RCC_AHB1RSTR_ETHMACRST;
  RCC->AHB1RSTR &= ~RCC_AHB1RSTR_ETHMACRST;

  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN |
                  RCC_AHB1ENR_GPIOBEN |
                  RCC_AHB1ENR_GPIOCEN |
                  RCC_AHB1ENR_GPIOGEN |
                  RCC_AHB1ENR_GPIOEEN;
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

#include "extio.h"
static int eth_mac_init_gpio( void )
{
  /**
   * GPIO init
   *
   */
#ifdef LQFP_144
  GPIO_INIT(GPIOA, 0,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_CRS
  GPIO_INIT(GPIOA, 3,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_COL
  GPIO_INIT(GPIOA, 7,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RX_DV

  GPIO_INIT(GPIOA, 1,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXCLK
  GPIO_INIT(GPIOC, 4,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD0
  GPIO_INIT(GPIOC, 5,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD1
  GPIO_INIT(GPIOB, 0,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD2
  GPIO_INIT(GPIOB, 1,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD3

  GPIO_INIT(GPIOC, 3,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXCLK
  GPIO_INIT(GPIOG,13,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD0
  GPIO_INIT(GPIOB,13,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD1
  GPIO_INIT(GPIOC, 2,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD2
  GPIO_INIT(GPIOE, 2,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD3

  GPIO_INIT(GPIOB,11,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXen

#endif
#ifdef LQFP_100
  GPIO_INIT(GPIOA, 0,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_CRS
  GPIO_INIT(GPIOA, 3,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_COL
  GPIO_INIT(GPIOA, 7,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RX_DV
 #if ((UTD_M != 0) || (IIP != 0))
  GPIO_INIT(GPIOB,10,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RX_ER
 #endif
  GPIO_INIT(GPIOA, 1,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXCLK
  GPIO_INIT(GPIOC, 4,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD0
  GPIO_INIT(GPIOC, 5,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD1
  GPIO_INIT(GPIOB, 0,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD2
  GPIO_INIT(GPIOB, 1,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_RXD3

  GPIO_INIT(GPIOC, 3,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXCLK
  GPIO_INIT(GPIOB,12,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD0
  GPIO_INIT(GPIOB,13,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD1
  GPIO_INIT(GPIOC, 2,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD2
  GPIO_INIT(GPIOE, 2,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXD3

  GPIO_INIT(GPIOB,11,MODE_AF,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MII_TXen

 #if ((UTD_M != 0) || (IIP != 0))
  GPIO_INIT(GPIOA, 8, MODE_AF, OTYPE_PP, OSPEED_VHS, PUPD_N,  0, 0); // CLKIN
  RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
 #endif
#endif
  return 0;
}

static int eth_mac_deinit_gpio( void )
{
  /**
   * GPIO deinit
   */
  GPIO_INIT(GPIOA, 0,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOA, 1,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOA, 3,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOA, 7,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB, 0,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB, 1,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB, 8,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB,10,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB,11,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB,12,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOB,13,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOC, 2,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOC, 3,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOC, 4,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  GPIO_INIT(GPIOC, 5,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0);
  return 0;
}


static int eth_mac_init_desc()
{
  uint32_t index = 0;

  for( index = 0 ; index < ETHERNET_DESCRIPTOR_TRANSMIT_COUNT ; index++ )
  {
    descriptor_transmit[index].status                    = DESCRIPTOR_TRANSMIT_STATUS_CTRL_TCH | DESCRIPTOR_TRANSMIT_STATUS_CTRL_IC;

    //if(_driver_capabilities->checksum_offload_tx_ip4)
    {
        descriptor_transmit[index].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_HARDWARE;
    }
    descriptor_transmit[index].buffer_count              = ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
    descriptor_transmit[index].buffer_address            = (uint32_t)transmit_bufer[index];

    if(index == ETHERNET_DESCRIPTOR_TRANSMIT_COUNT - 1)
    {
      descriptor_transmit[index].buffer_next_address       = (uint32_t)&descriptor_transmit[0];
      descriptor_transmit[index].status                 |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_TER;
    }
    else
    {
      descriptor_transmit[index].buffer_next_address       = (uint32_t)&descriptor_transmit[index+1];
    }
#if defined(ETHERNET_DESCRIPTOR_EXTENDED) && ETHERNET_DESCRIPTOR_EXTENDED == 1
    descriptor_transmit[index].status_extended           = 0;
    descriptor_transmit[index].reserved1                 = 0;
    descriptor_transmit[index].time_stamp_low            = 0;
    descriptor_transmit[index].time_stamp_high           = 0;
#endif
  }

  for( index = 0 ; index < ETHERNET_DESCRIPTOR_RECEIVE_COUNT ; index++ )
  {
    descriptor_receive[index].status                    = DESCRIPTOR_RECEIVE_STATUS_OWN;
    descriptor_receive[index].status                    |= DESCRIPTOR_RECEIVE_STATUS_FT;
    descriptor_receive[index].status                    |= DESCRIPTOR_RECEIVE_STATUS_IPHCE_TSV;
    descriptor_receive[index].status                    |= DESCRIPTOR_RECEIVE_STATUS_PCE_ESA;
    descriptor_receive[index].buffer_count              = DESCRIPTOR_BUFFER_COUNT_RCH | ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);

    if(index == ETHERNET_DESCRIPTOR_RECEIVE_COUNT - 1)
    {
      //descriptor_receive[index].buffer_count              = DESCRIPTOR_BUFFER_COUNT_RCH/* | DESCRIPTOR_BUFFER_COUNT_RER | */((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
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

//  for( index = 0 ; index <= sizeof(transmit_buffer_test); index++ )
//  {
//    transmit_bufer[0][index] = transmit_buffer_test[index];
//    transmit_bufer[1][index] = transmit_buffer_test[index];
//    transmit_bufer[2][index] = transmit_buffer_test[index];
//    transmit_bufer[3][index] = transmit_buffer_test[index];
//    transmit_bufer[4][index] = transmit_buffer_test[index];
//
//  }
//  descriptor_transmit[0].buffer_count = sizeof(transmit_buffer_test);
//  descriptor_transmit[0].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
//  //descriptor_transmit[0].status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
//
//  descriptor_transmit[1].buffer_count = sizeof(transmit_buffer_test);
//  descriptor_transmit[1].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
//  //descriptor_transmit[1].status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
//
//  descriptor_transmit[2].buffer_count = sizeof(transmit_buffer_test);
//  descriptor_transmit[2].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
//  //descriptor_transmit[2].status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
//
//  descriptor_transmit[3].buffer_count = sizeof(transmit_buffer_test);
//  descriptor_transmit[3].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
//  //descriptor_transmit[3].status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
//
//  descriptor_transmit[4].buffer_count = sizeof(transmit_buffer_test);
//  descriptor_transmit[4].status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
//  //descriptor_transmit[4].status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;

//
///* Bit definition for Ethernet DMA Interrupt Enable Register */
//#define ETH_DMAIER_NISE      ((uint32_t)0x00010000)  /* Normal interrupt summary enable */
//#define ETH_DMAIER_AISE      ((uint32_t)0x00008000)  /* Abnormal interrupt summary enable */
//#define ETH_DMAIER_ERIE      ((uint32_t)0x00004000)  /* Early receive interrupt enable */
//#define ETH_DMAIER_FBEIE     ((uint32_t)0x00002000)  /* Fatal bus error interrupt enable */
//#define ETH_DMAIER_ETIE      ((uint32_t)0x00000400)  /* Early transmit interrupt enable */
//#define ETH_DMAIER_RWTIE     ((uint32_t)0x00000200)  /* Receive watchdog timeout interrupt enable */
//#define ETH_DMAIER_RPSIE     ((uint32_t)0x00000100)  /* Receive process stopped interrupt enable */
//#define ETH_DMAIER_RBUIE     ((uint32_t)0x00000080)  /* Receive buffer unavailable interrupt enable */
//#define ETH_DMAIER_RIE       ((uint32_t)0x00000040)  /* Receive interrupt enable */
//#define ETH_DMAIER_TUIE      ((uint32_t)0x00000020)  /* Transmit Underflow interrupt enable */
//#define ETH_DMAIER_ROIE      ((uint32_t)0x00000010)  /* Receive Overflow interrupt enable */
//#define ETH_DMAIER_TJTIE     ((uint32_t)0x00000008)  /* Transmit jabber timeout interrupt enable */
//#define ETH_DMAIER_TBUIE     ((uint32_t)0x00000004)  /* Transmit buffer unavailable interrupt enable */
//#define ETH_DMAIER_TPSIE     ((uint32_t)0x00000002)  /* Transmit process stopped interrupt enable */
//#define ETH_DMAIER_TIE       ((uint32_t)0x00000001)  /* Transmit interrupt enable */
  return 0;
}

static int eth_mac_deinit_desc()
{
  return 0;
}

#ifdef LQFP_144
  #define ETH_MACMIIAR_CR_Div ETH_MACMIIAR_CR_Div102;
#endif
#ifdef LQFP_100
  #define ETH_MACMIIAR_CR_Div ETH_MACMIIAR_CR_Div62;
#endif


extern void __reboot__();
static int eth_mac_init_regs()
{

  /**
   * ETHERNET init
   */
  //
  RCC_ClocksTypeDef RCC_Clocks;

  RCC_GetClocksFreq(&RCC_Clocks);
  //Задать ETH->MACMIIAR в соответствии с частотой

  ETH->MACMIIAR = ETH_MACMIIAR_CR_Div;

  ETH->MACCR    = ETH_MACCR_IPCO | ETH_MACCR_FES | ETH_MACCR_IFG_64Bit; //LQFP100 = ETH_MACCR_IFG_96Bit
  ETH->MACFFR   = ETH_MACFFR_RA | ETH_MACFFR_PCF_ForwardPassedAddrFilter | ETH_MACFFR_PAM;

  // !!! //Переключение в дуплекс
  ETH->MACCR   |= ETH_MACCR_DM;
  // !!!

  ETH->DMARDLAR = (uint32_t)descriptor_receive;
  ETH->DMATDLAR = (uint32_t)descriptor_transmit;
  ETH->MACA0LR  = 0x12345678;
  ETH->MACA0HR  = 0x00000330;


    /* Enable transmit state machine of the MAC for transmission on the MII */
  ETH->MACCR   |= ETH_MACCR_TE;//ETH_MACTransmissionCmd(ENABLE);
  /* Flush Transmit FIFO */
  ETH->DMAOMR  |= ETH_DMAOMR_FTF | ETH_DMAOMR_TSF; // ETH_FlushTransmitFIFO();
#if ((UTD_M != 0) || (IIP != 0))
  volatile int cCount = 0;
  while (ETH->DMAOMR & ETH_DMAOMR_FTF) { // Wait Elush End
    if (++cCount >= 10000) {
      for (cCount = 0; cCount < 1000000; ++cCount) { }
      __reboot__();
    }
  }
#else
  while (ETH->DMAOMR & ETH_DMAOMR_FTF) { } // Wait Elush End
#endif

  /* Enable receive state machine of the MAC for reception from the MII */
  ETH->MACCR |= ETH_MACCR_RE;  //ETH_MACReceptionCmd(ENABLE);

  /* Start DMA transmission */
  ETH->DMAOMR |= ETH_DMAOMR_ST; //ETH_DMATransmissionCmd(ENABLE);
  /* Start DMA reception */
  ETH->DMAOMR |= ETH_DMAOMR_SR; //ETH_DMAReceptionCmd(ENABLE);

  ETH->DMABMR   = ETH_DMABMR_USP | ETH_DMABMR_RDP_1Beat | ETH_DMABMR_PBL_32Beat; //LQFP100 = <Нет>

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

static int eth_mac_deinit_regs()
{
  NVIC->ICER[(((uint32_t)(int32_t)ETH_IRQn) >> 5UL)] = (uint32_t)(1UL << (((uint32_t)(int32_t)ETH_IRQn) & 0x1FUL));
  ETH->DMAIER &= ~(ETH_DMAIER_NISE | ETH_DMAIER_RIE | ETH_DMAIER_TIE);
  return 0;
}


static driver_version eth_mac_get_version(void)
{
  return _driver_version;
}

static eth_mac_capabilities eth_mac_get_capabilities(void)
{
  return _driver_capabilities;
}

#if ((UTD_M != 0) || (IIP != 0))
static int32_t eth_mac_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data);
static void eth_mac_init_phy() {
  //
  volatile int i;
  for (i = 0; i < 200000; ++i) { }
  GPIOC->BSRRH = GPIO_Pin_9; //Сброс PHY - начало

  GPIO_INIT(GPIOC, 9, MODE_OUT, OTYPE_PP, OSPEED_VHS, PUPD_N, 11, 0); // RESET_PHYLAN

  GPIO_INIT(GPIOB,10,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // PHYAD0   0
  GPIO_INIT(GPIOA, 1,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // PHYAD1   0
  GPIO_INIT(GPIOB, 1,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // PHYAD2   0

  GPIO_INIT(GPIOC, 4,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MODE0    0
  GPIO_INIT(GPIOC, 5,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MODE1    0
  GPIO_INIT(GPIOA, 3,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // MODE2    0

  GPIO_INIT(GPIOB, 0,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,11,0); // RMIISEL  0

  GPIOB->BSRRH = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_10; // 000 000 0
  GPIOA->BSRRH = GPIO_Pin_1;
  GPIOA->BSRRL = GPIO_Pin_3;
  GPIOC->BSRRL = GPIO_Pin_4 | GPIO_Pin_5;

  for (i = 0; i <  50000; ++i) { }
  GPIOC->BSRRL = GPIO_Pin_9; //Сброс PHY - конец

  GPIO_INIT(GPIOC, 1, MODE_AF, OTYPE_PP, OSPEED_VHS, PUPD_N, 11, 0); // MDC
  GPIO_INIT(GPIOA, 2, MODE_AF, OTYPE_PP, OSPEED_VHS, PUPD_N, 11, 0); // MDIO
  for (i = 0; i <  10000; ++i) { }

  ETH->MACMIIAR = ETH_MACMIIAR_CR_Div;

//  int32_t cRes;
//  cRes = eth_mac_phy_write(0, 0, 0x8000);
//  cRes = eth_mac_phy_write(0, 0, 0x3300);

}
uint8_t aRegIndexes[] = { 0, 1, 2, 3, 4, 5, 6, 17, 18, 26, 27, 29, 30, 31 };
uint16_t aRegValues[sizeof(aRegIndexes)];

static int32_t eth_mac_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data);
void eth_mac_phy_read_regs() {
  for (int i = 0; i < sizeof(aRegIndexes); ++i) {
    eth_mac_phy_read(0, aRegIndexes[i], &aRegValues[i]);
  }
}
#endif

static int32_t eth_mac_initialize(eth_mac_signalevent_t cb_event)
{
  eth_mac_init_rcc();
#if ((UTD_M != 0) || (IIP != 0))
  eth_mac_init_desc();
  eth_mac_init_phy();
  eth_mac_init_gpio();
#else
  eth_mac_init_gpio();
  eth_mac_init_desc();
#endif
  eth_mac_init_regs();

  _mac_event = cb_event;

  return DRIVER_OK;
}

static int32_t eth_mac_uninitialize(void)
{
  eth_mac_deinit_rcc();
  eth_mac_deinit_gpio();
  eth_mac_deinit_desc();
  eth_mac_deinit_regs();

  _mac_event = 0;

  return DRIVER_OK;
}

static int32_t eth_mac_power_control(power_state state)
{
  switch (state)
  {
    case POWER_OFF:
      {
      }
      break;
    case POWER_LOW:
      {
      }
      break;
    case POWER_FULL:
      {
      }
      break;
    default:
      return DRIVER_ERROR_UNSUPPORTED;
  }
  return DRIVER_OK;
}

static int32_t eth_mac_get_mac_address(eth_mac_addr *ptr_addr)
{
  if(ptr_addr)
  {
    ptr_addr->b[5] = (uint8_t)((ETH->MACA0HR >> 8 ) & 0xFF);
    ptr_addr->b[4] = (uint8_t)((ETH->MACA0HR >> 0 ) & 0xFF);
    ptr_addr->b[3] = (uint8_t)((ETH->MACA0LR >> 24) & 0xFF);
    ptr_addr->b[2] = (uint8_t)((ETH->MACA0LR >> 16) & 0xFF);
    ptr_addr->b[1] = (uint8_t)((ETH->MACA0LR >> 8 ) & 0xFF);
    ptr_addr->b[0] = (uint8_t)((ETH->MACA0LR >> 0 ) & 0xFF);
  } else return DRIVER_ERROR_PARAMETER;
  return DRIVER_OK;
}

static int32_t eth_mac_set_mac_address(const eth_mac_addr *ptr_addr)
{
  if(ptr_addr)
  {
    ETH->MACA0HR = ((((uint32_t)(ptr_addr->b[5])) << 8) | ((uint32_t)(ptr_addr->b[4])));
    ETH->MACA0LR = ((((uint32_t)(ptr_addr->b[3])) << 24) | (((uint32_t)(ptr_addr->b[2])) << 16) | (((uint32_t)(ptr_addr->b[1])) << 8) | (((uint32_t)(ptr_addr->b[0])) << 0));
  } else return DRIVER_ERROR_PARAMETER;
  return DRIVER_OK;
}

static int32_t eth_mac_set_address_filter(const eth_mac_addr *ptr_addr, uint32_t num_addr)
{
  return DRIVER_ERROR_UNSUPPORTED;
}

#include "info.h"
#ifdef USE_STP
  #include "stp.h"
#else
  #include "ethernet.h"
#endif
#include "portmacro.h"
#include "def.h"
extern void vTaskDelay( portTickType xTicksToDelay );

uint32_t gArpTxCount0 = 0, gArpTxCount1 = 0;

static int32_t eth_mac_send_frame(const uint8_t *frame, uint32_t len, uint32_t flags)
{
  struct eth_hdr* ethhdr = (struct eth_hdr *)frame;
  if((frame) && !(descriptor_transmit_p->status & DESCRIPTOR_TRANSMIT_STATUS_OWN))
  {
    memcpy((void*)(descriptor_transmit_p->buffer_address),frame,len);
    descriptor_transmit_p->buffer_count = len & DESCRIPTOR_BUFFER_COUNT_TBS1;
    descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_FS | DESCRIPTOR_TRANSMIT_STATUS_CTRL_LS;
#if (ENABLE_TAIL_TAG)
    if (IsTailTag(0)) {
      descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_DP;
    }
#endif
    descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_CIC_ALL_HARDWARE;
    //descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_CTRL_IC;
    descriptor_transmit_p->status |= DESCRIPTOR_TRANSMIT_STATUS_OWN;
    descriptor_transmit_p = (ethernet_descriptor_t*)(descriptor_transmit_p->buffer_next_address);

     /* When Tx Buffer unavailable flag is set: clear it and resume transmission */
    if (ETH->DMASR & ETH_DMASR_TBUS)
    {
      if (ethhdr->type == PP_HTONS(ETHTYPE_ARP)) { //0x0608
        ++gArpTxCount0;
      }
      /* Clear TBUS ETHERNET DMA flag */
      ETH->DMASR = ETH_DMASR_TBUS;
      /* Resume DMA transmission*/
      ETH->DMATPDR = 0;
    } else {
      vTaskDelay(1);
      if (ETH->DMASR & ETH_DMASR_TBUS)
      {
        if (ethhdr->type == PP_HTONS(ETHTYPE_ARP)) { //0x0608
          ++gArpTxCount1;
        }
        /* Clear TBUS ETHERNET DMA flag */
        ETH->DMASR = ETH_DMASR_TBUS;
        /* Resume DMA transmission*/
        ETH->DMATPDR = 0;
      } else {
        vTaskDelay(1);
        if (ETH->DMASR & ETH_DMASR_TBUS)
        {
          /* Clear TBUS ETHERNET DMA flag */
          ETH->DMASR = ETH_DMASR_TBUS;
          /* Resume DMA transmission*/
          ETH->DMATPDR = 0;
        } else {
          return DRIVER_ERROR_PARAMETER;
        }
      }
    }
  } else {
    return DRIVER_ERROR_PARAMETER;
  }
  return DRIVER_OK;
}
static int32_t eth_mac_read_frame(uint8_t *frame, uint32_t len)
{
  if((frame) && !(descriptor_receive_p->status & DESCRIPTOR_RECEIVE_STATUS_OWN))
  {
    memcpy(frame,(void*)(descriptor_receive_p->buffer_address),len);
    descriptor_receive_p->status = DESCRIPTOR_RECEIVE_STATUS_OWN;
//    descriptor_receive_p->status                    = DESCRIPTOR_RECEIVE_STATUS_OWN;
//    descriptor_receive_p->status                    |= DESCRIPTOR_RECEIVE_STATUS_FT;
//    descriptor_receive_p->status                    |= DESCRIPTOR_RECEIVE_STATUS_IPHCE_TSV;
//    descriptor_receive_p->status                    |= DESCRIPTOR_RECEIVE_STATUS_PCE_ESA;
//    descriptor_receive_p->buffer_count              = DESCRIPTOR_BUFFER_COUNT_RCH | ((/*ETHERNET_PACKET_SIZE*/0 & 0x00001FFF) << 16) | ((ETHERNET_PACKET_SIZE & 0x00001FFF) << 0);
//
    descriptor_receive_p = (ethernet_descriptor_t*)(descriptor_receive_p->buffer_next_address);
  } else return DRIVER_ERROR_PARAMETER;

  return DRIVER_OK;
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

static int32_t eth_mac_get_rx_frame_time(eth_mac_time *time)
{
  return DRIVER_ERROR_UNSUPPORTED;
}

static int32_t eth_mac_get_tx_frame_time(eth_mac_time *time)
{
  return DRIVER_ERROR_UNSUPPORTED;
}

static int32_t eth_mac_control(uint32_t control, uint32_t arg)
{
  switch (control)
  {
    case ETH_MAC_CONFIGURE:
      {
        switch (arg & ETH_MAC_SPEED_Msk)
        {
          case ETH_MAC_SPEED_10M:
            {
              ETH->MACCR &= ~ETH_MACCR_FES;
            }
            break;
          case ETH_SPEED_100M:
            {
              ETH->MACCR |= ETH_MACCR_FES;
            }
            break;
          default:
            return DRIVER_ERROR_UNSUPPORTED;
        }

        switch (arg & ETH_MAC_DUPLEX_Msk)
        {
          case ETH_MAC_DUPLEX_HALF:
            {
              ETH->MACCR &= ~ETH_MACCR_DM;
            }
            break;
          case ETH_MAC_DUPLEX_FULL:
            {
              ETH->MACCR |= ETH_MACCR_DM;
            }
            break;
          default:
            return DRIVER_ERROR_UNSUPPORTED;
        }

        if (arg & ETH_MAC_LOOPBACK)
        {
          ETH->MACCR |= ETH_MACCR_LM;
        }
        else
        {
          ETH->MACFFR &=~ETH_MACCR_LM;
        }

        if ((arg & ETH_MAC_CHECKSUM_OFFLOAD_RX) ||
            (arg & ETH_MAC_CHECKSUM_OFFLOAD_TX))
        {
          return DRIVER_ERROR_UNSUPPORTED;
        }

        if (!(arg & ETH_MAC_ADDRESS_BROADCAST))
        {
          ETH->MACFFR |= ETH_MACFFR_BFD;
        }
        else
        {
          ETH->MACFFR &=~ETH_MACFFR_BFD;
        }

        if (arg & ETH_MAC_ADDRESS_MULTICAST)
        {
          ETH->MACFFR |= ETH_MACFFR_PAM;
        }
        else
        {
          ETH->MACFFR &=~ETH_MACFFR_PAM;
        }

        if (arg & ETH_MAC_ADDRESS_ALL)
        {
          ETH->MACFFR |= ETH_MACFFR_RA;
        }
        else
        {
          ETH->MACFFR &=~ETH_MACFFR_RA;
        }
      }
      break;

    case ETH_MAC_CONTROL_TX:
      {
        switch (arg)
        {
          case 0:
            {
              ETH->MACCR &=~ETH_MACCR_TE;
            }
            break;
          case 1:
            {
              ETH->MACCR |= ETH_MACCR_TE;
            }
            break;
          default:
            return DRIVER_ERROR_UNSUPPORTED;
        }
      }
      break;

    case ETH_MAC_CONTROL_RX:
      {
        switch (arg)
        {
          case 0:
            {
              ETH->MACCR &=~ETH_MACCR_RE;
            }
            break;
          case 1:
            {
              ETH->MACCR |= ETH_MACCR_RE;
            }
            break;
          default:
            return DRIVER_ERROR_UNSUPPORTED;
        }
      }
      break;

    case ETH_MAC_FLUSH:
      {
        if (arg & ETH_MAC_FLUSH_RX)
        {
          // flush rx buffer
          return DRIVER_ERROR_UNSUPPORTED;
        }
        if (arg & ETH_MAC_FLUSH_TX)
        {
          // flush tx buffer
          return DRIVER_ERROR_UNSUPPORTED;
        }
      }
      break;

    case ETH_MAC_SLEEP:
      {
        return DRIVER_ERROR_UNSUPPORTED;
      }
      break;

    case ETH_MAC_VLAN_FILTER:
      {
        return DRIVER_ERROR_UNSUPPORTED;
      }
      break;

    default:
      return DRIVER_ERROR_UNSUPPORTED;
  }
  return DRIVER_OK;
}

static int32_t eth_mac_control_timer(uint32_t control, eth_mac_time *time)
{
  return DRIVER_ERROR_UNSUPPORTED;
}

static int32_t eth_mac_phy_read(uint8_t phy_addr, uint8_t reg_addr, uint16_t *data)
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
  else return DRIVER_ERROR_PARAMETER;

  return DRIVER_OK;
}

static int32_t eth_mac_phy_write(uint8_t phy_addr, uint8_t reg_addr, uint16_t data)
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
  else return DRIVER_ERROR_PARAMETER;
  return DRIVER_OK;
}

// End ETH MAC Interface

eth_mac ETH_MAC_(0) =
{
    eth_mac_get_version,
    eth_mac_get_capabilities,
    eth_mac_initialize,
    eth_mac_uninitialize,
    eth_mac_power_control,
    eth_mac_get_mac_address,
    eth_mac_set_mac_address,
    eth_mac_set_address_filter,
    eth_mac_send_frame,
    eth_mac_read_frame,
    eth_mac_get_rx_frame_size,
    eth_mac_get_rx_frame_time,
    eth_mac_get_tx_frame_time,
    eth_mac_control_timer,
    eth_mac_control,
    eth_mac_phy_read,
    eth_mac_phy_write
};



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
        if(0){
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
