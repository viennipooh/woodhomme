#include "main.h"
#include "ks8993.h"

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

#undef GPIO_INIT
#define GPIO_INIT(GPIO,PIN,MODE,OTYPE,OSPEED,PUPD,AF,LOCK) \
do{\
    unsigned char pin=PIN;\
    unsigned char reg=0;\
  RCC->AHB1ENR |= RCC_AHB1ENR_##GPIO##EN;\
  GPIO->LCKR &= ~(1<<pin);\
  \
  GPIO->MODER &= ~(3<<(pin<<1));\
  GPIO->MODER |=  (MODE<<(pin<<1));\
  \
  GPIO->OTYPER &= ~(1<<pin);\
  GPIO->OTYPER |=  (OTYPE<<pin);\
  \
  GPIO->OSPEEDR &= ~(3<<(pin<<1));\
  GPIO->OSPEEDR |=  (OSPEED<<(pin<<1));\
  \
  GPIO->PUPDR &= ~(3<<(pin<<1));\
  GPIO->PUPDR |=  (PUPD<<(pin<<1));\
  \
  if( (pin)>7 )\
  {pin-=8;reg=1;}\
  GPIO->AFR[reg] &=  ~(0xF<<(pin<<2));\
  GPIO->AFR[reg] |=   ((0xF&(AF))<<(pin<<2));\
  \
  GPIO->LCKR |= (LOCK<<PIN);\
}while(0);

/**
typedef GPIO::PIN::PA0   Mii_CRS;
typedef GPIO::PIN::PA1   Mii_RXCLK;
typedef GPIO::PIN::PA2   switch_PV31;
typedef GPIO::PIN::PA3   Mii_Col;
typedef GPIO::PIN::PA4   switch_PV23;
typedef GPIO::PIN::PA5   Wifi_SPI_Clk;
typedef GPIO::PIN::PA6   Wifi_SPI_Miso;
typedef GPIO::PIN::PA7   Mii_RXdv;
typedef GPIO::PIN::PA8   switch_P3TXQ2;
typedef GPIO::PIN::PA9   switch_P2TXQ2;
typedef GPIO::PIN::PA10  switch_P1TXQ2;
typedef GPIO::PIN::PA11  switch_P3PP;
typedef GPIO::PIN::PA12  switch_P2PP;

typedef GPIO::PIN::PB0   Mii_RXd2;
typedef GPIO::PIN::PB1   Mii_RXd3;

typedef GPIO::PIN::PB5   Wifi_SPI_Mosi;
typedef GPIO::PIN::PB6   Wifi_Reset;

typedef GPIO::PIN::PB8   Mii_TXD3;
typedef GPIO::PIN::PB9   Wifi_Intr;
typedef GPIO::PIN::PB10  Mii_RXer;
typedef GPIO::PIN::PB11  Mii_TXen;
typedef GPIO::PIN::PB12  Mii_TXD0;
typedef GPIO::PIN::PB13  Mii_TXD1;
typedef GPIO::PIN::PB14  switch_Led10;
typedef GPIO::PIN::PB15  switch_Led23;

typedef GPIO::PIN::PC0   switch_FFlow2;
typedef GPIO::PIN::PC1   switch_PV32;
typedef GPIO::PIN::PC2   Mii_TXD2;
typedef GPIO::PIN::PC3   Mii_TXCLK;
typedef GPIO::PIN::PC4   Mii_RXd0;
typedef GPIO::PIN::PC5   Mii_RXd1;
typedef GPIO::PIN::PC6   LVDS1_TX;
typedef GPIO::PIN::PC7   LVDS1_RX;
typedef GPIO::PIN::PC8   LVDS1_RE;
typedef GPIO::PIN::PC9   switch_P11Pen;
typedef GPIO::PIN::PC10  mEther_SCLK;
typedef GPIO::PIN::PC11  mEther_Miso;
typedef GPIO::PIN::PC12  mEther_Mosi;

typedef GPIO::PIN::PD0   mEther_SlaveSelect;
typedef GPIO::PIN::PD1   switch_P1PP;
typedef GPIO::PIN::PD2   switch_P1Tagin;
typedef GPIO::PIN::PD3   switch_P2Tagin;
typedef GPIO::PIN::PD4   switch_P3Tagin;
typedef GPIO::PIN::PD5   switch_P3Tagirm;
typedef GPIO::PIN::PD6   switch_P2Tagirm;
typedef GPIO::PIN::PD7   switch_P1Tagirm;
typedef GPIO::PIN::PD8   switch_Led22;
typedef GPIO::PIN::PD9   switch_Led21;
typedef GPIO::PIN::PD10  switch_Led20;
typedef GPIO::PIN::PD11  switch_Led33;
typedef GPIO::PIN::PD12  switch_Led32;
typedef GPIO::PIN::PD13  switch_Led31;
typedef GPIO::PIN::PD14  switch_Led30;
typedef GPIO::PIN::PD15  switch_PRSV;

typedef GPIO::PIN::PE7   switch_Led12;
typedef GPIO::PIN::PE8   switch_Led13;
typedef GPIO::PIN::PE9   switch_reset;
typedef GPIO::PIN::PE10  switch_modesel0;
typedef GPIO::PIN::PE11  switch_modesel1;
typedef GPIO::PIN::PE12  switch_modesel2;
typedef GPIO::PIN::PE13  switch_modesel3;
typedef GPIO::PIN::PE14  switch_S0;
typedef GPIO::PIN::PE15  switch_S1;

typedef GPIO::PIN::PF3   kn_5;
typedef GPIO::PIN::PF4   kn_4;
typedef GPIO::PIN::PF5   kn_2;
typedef GPIO::PIN::PF6   kn_1;
typedef GPIO::PIN::PF7   kn_6;
typedef GPIO::PIN::PF8   kn_7;
typedef GPIO::PIN::PF9   kn_3;
typedef GPIO::PIN::PF10  switch_VMdis;
typedef GPIO::PIN::PF11  switch_FFlow1;
typedef GPIO::PIN::PF12  switch_PV21;
typedef GPIO::PIN::PF13  switch_PV13;
typedef GPIO::PIN::PF14  switch_PV12;
typedef GPIO::PIN::PF15  switch_Disan3;

typedef GPIO::PIN::PG0   mEther_FDPLX;
typedef GPIO::PIN::PG1   switch_Led11;
typedef GPIO::PIN::PG2   switch_PRSEL1;
typedef GPIO::PIN::PG3   switch_PRSEL0;
typedef GPIO::PIN::PG4   switch_PBase1;
typedef GPIO::PIN::PG5   switch_PBase2;
typedef GPIO::PIN::PG6   switch_PBase0;
typedef GPIO::PIN::PG7   switch_P31Pen;
typedef GPIO::PIN::PG8   switch_P21Pen;
typedef GPIO::PIN::PG9   Wifi_CS;
typedef GPIO::PIN::PG10  mEther_inter;
typedef GPIO::PIN::PG11  mEther_wr;
typedef GPIO::PIN::PG12  mEther_Reset;
typedef GPIO::PIN::PG13  mEther_cs;
typedef GPIO::PIN::PG14  mEther_rd;
**/

#define kn_5_gpio    GPIOF
#define kn_5_pin     3

#define kn_6_gpio    GPIOF
#define kn_6_pin     6

#define PV21_gpio    GPIOF
#define PV21_pin     12

#define PV13_gpio    GPIOF
#define PV13_pin     13

#define PV12_gpio    GPIOF
#define PV12_pin     14

#define PV31_gpio    GPIOA
#define PV31_pin     2

#define PV23_gpio    GPIOA
#define PV23_pin     4

#define PV32_gpio    GPIOC
#define PV32_pin     1

#define kn_5            GPIOF,3
#define kn_6            GPIOF,6

#define PV21            GPIOF,12
#define PV13            GPIOF,13
#define PV12            GPIOF,14
#define PV31            GPIOF,2
#define PV23            GPIOF,4
#define PV32            GPIOF,1

#define DISAN3          GPIOF,15
  
#define MIIS0           GPIOE,14
#define MIIS1           GPIOE,15
  
#define MODESEL0        GPIOE,10
#define MODESEL1        GPIOE,11
#define MODESEL2        GPIOE,12
#define MODESEL3        GPIOE,13

#define MRXD3           GPIOB,1
#define MRXD2           GPIOB,0
      
#define MRESET          GPIOE,9
#define P1_1PEN         GPIOC,9

#define FFLOW2          GPIOC,0
#define VMDIS           GPIOF,10
#define FFLOW1          GPIOF,11

#define PRSEL1          GPIOG,2
#define PRSEL0          GPIOG,3

#define PBASE2          GPIOG,5
#define PBASE1          GPIOG,4
#define PBASE0          GPIOG,3

#define P3_TXQ2         GPIOA,8
#define P2_TXQ2         GPIOA,9
#define P1_TXQ2         GPIOA,10

#define P3_PP           GPIOA,11
#define P2_PP           GPIOA,12
#define P1_PP           GPIOD,1


#define P1_TAGINS       GPIOD,2
#define P2_TAGINS       GPIOD,3
#define P3_TAGINS       GPIOD,4
#define P3_TAGRM        GPIOD,5
#define P2_TAGRM        GPIOD,6
#define P1_TAGRM        GPIOD,7

#define PRSV            GPIOD,15

#define DISAN3          GPIOF,15
  

#define LED13           GPIOE,8
#define LED12           GPIOE,7
#define LED11           GPIOG,1
#define LED10           GPIOB,14

#define LED23           GPIOB,15
#define LED22           GPIOD,8
#define LED21           GPIOD,9
#define LED20           GPIOD,10

#define LED33           GPIOD,11
#define LED32           GPIOD,12
#define LED31           GPIOD,13
#define LED30           GPIOD,14

#define MRXD3           GPIOB,1
#define MRXD2           GPIOB,0
#define MRXD1           GPIOC,5
#define MRXD0           GPIOC,4     

#define MCOL            GPIOA,3



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
static inline void gpio_high(GPIO_TypeDef *gpio, int pin)
{
  gpio->BSRRL |= (1<<pin);
}
static inline void gpio_low(GPIO_TypeDef *gpio, int pin)
{
  gpio->BSRRH |= (1<<pin);
}

int ks8993_reset_on( void )
{
  gpio_init_pp(MRESET);
  gpio_low(MRESET);
  return 0;
}
int ks8993_reset_off( void )
{
  gpio_init_pp(MRESET);
  gpio_high(MRESET);
  return 0;
}

int ks8993_select_group(ks8993_vlan_group_t group)
{
  /* группа настройки объединения портов */
  gpio_init_pp(PV21);
  gpio_init_pp(PV13);
  gpio_init_pp(PV12);
  gpio_init_pp(PV31);
  gpio_init_pp(PV23);
  gpio_init_pp(PV32);
  switch(group)
  {
/********************************************
 *                  3 (MCU)
 *                /  \
 *               1    2
 ********************************************/
    case VLAN_G13_G23:
      {
        // Порт 1 объеденен с портом 3
        gpio_high(PV13);
        gpio_low(PV12);
        // Порт 2 объеденен с портом 3
        gpio_high(PV23);
        gpio_low(PV21);
        // Порт 3 объеденен с портоами 1 и 2
        gpio_high(PV32);
        gpio_high(PV31);
      }
      break;
/********************************************
 *                  3 (MCU)
 *                /   
 *               1 -  2
 ********************************************/
    case VLAN_G13_G12:
      {
        // Порт 1 объеденен с портами 2,3
        gpio_high(PV13);
        gpio_high(PV12);
        // Порт 2 объеденен с портом 1
        gpio_low(PV23);
        gpio_high(PV21);
        // Порт 3 объеденен с портом 1
        gpio_low(PV32);
        gpio_high(PV31);
      }
      break;
/********************************************
 *                  3 (MCU)
 *                   \
 *               1 -  2
 ********************************************/    
    case VLAN_G12_G23: 
      {
        // Порт 1 объеденен с портом 2
        gpio_low(PV13);
        gpio_high(PV12);
        // Порт 2 объеденен с портами 1,3
        gpio_high(PV23);
        gpio_high(PV21);
        // Порт 3 объеденен с портом 1
        gpio_low(PV32);
        gpio_high(PV31);
      } 
      break;
/********************************************
 *                  3 (MCU)
 *                /  \
 *               1 -  2
 ********************************************/
    case VLAN_G123:
    default:
      {
        // Порт 1 объеденен с портом 2,3
        gpio_high(PV13);
        gpio_high(PV12);
        // Порт 2 объеденен с портом 1,3
        gpio_high(PV23);
        gpio_high(PV21);
        // Порт 3 объеденен с портоами 1 и 2
        gpio_high(PV32);
        gpio_high(PV31);
      }
      break;
  };
  return 0;
}


int ks8993_select_mode_mii(ks8993_mii_mode_t mode)
{
  gpio_init_pp(MIIS0);
  gpio_init_pp(MIIS1);
  switch(mode)
  {
    case MII_DISABLE:
      {
        gpio_low(MIIS0);
        gpio_low(MIIS1);
      }
      break;
    case MII_FORWARD:
    case MII_EXT_PHY_MODE:
      {
        gpio_low(MIIS0);
        gpio_high(MIIS1);
      }
      break;
    case MII_SNI:
      {
        gpio_high(MIIS0);
        gpio_high(MIIS1);
      }
      break;
    case MII_REVERSE:
    case MII_EXT_MAC_MODE:
    default:
      {
        gpio_high(MIIS0);
        gpio_low(MIIS1);
      }
      break;
  }
  return 0;
}

int ks8993_select_mode_led(ks8993_led_mode_t mode)
{
  gpio_init_pp(MODESEL0);
  gpio_init_pp(MODESEL1);    
  gpio_init_pp(MODESEL2);
  gpio_init_pp(MODESEL3); 
  switch(mode)
  {
    case LED_MODE_1:
      {
        gpio_high(MODESEL0);
        gpio_low(MODESEL1);
        gpio_low(MODESEL2);
        gpio_low(MODESEL3);
      }
      break;
    case LED_MODE_2:
      {
        gpio_low(MODESEL0);
        gpio_high(MODESEL1);
        gpio_low(MODESEL2);
        gpio_low(MODESEL3);
      }
      break;
    case LED_MODE_3:
      {
        gpio_high(MODESEL0);
        gpio_high(MODESEL1);
        gpio_low(MODESEL2);
        gpio_low(MODESEL3);
      }
      break;
    case LED_MODE_0_EXT:
      {
        gpio_low(MODESEL0);
        gpio_low(MODESEL1);
        gpio_high(MODESEL2);
        gpio_high(MODESEL3);
      }
      break;
    case LED_MODE_2_EXT:
      {
        gpio_low(MODESEL0);
        gpio_high(MODESEL1);
        gpio_high(MODESEL2);
        gpio_high(MODESEL3);
      }
      break;
    case LED_MODE_3_EXT:
      {
        gpio_high(MODESEL0);
        gpio_high(MODESEL1);
        gpio_low(MODESEL2);
        gpio_high(MODESEL3);
      }
      break;
    case LED_MODE_0:
    default:
      {
        gpio_low(MODESEL0);
        gpio_low(MODESEL1);
        gpio_low(MODESEL2);
        gpio_low(MODESEL3);
      }
      break;
  }
  return 0;
}

int ks8993_select_flow_control(ks8993_port_t port,ks8993_flow_control_t mode)
{
  switch(port)
  {
    case PORT_1:
      {
        gpio_init_pp(FFLOW1);
        if(mode == ENABLE_FLOW_CONTROL)
        {
          gpio_low(FFLOW1);
        }
        else
        {
          gpio_high(FFLOW1);
        }
      }
      break;
    case PORT_2:
      {
        gpio_init_pp(FFLOW2);
        if(mode == ENABLE_FLOW_CONTROL)
        {
          gpio_low(FFLOW2);
        }
        else
        {
          gpio_high(FFLOW2);
        }
      }
      break;
    case PORT_3:
      {
        gpio_init_pp(LED12);
        if(mode == ENABLE_FLOW_CONTROL)
        {
          gpio_low(LED12);
        }
        else
        {
          gpio_high(LED12);
        }
      }
      break;
    default:
      return -1;
  }
  return 0;
}


int ks8993_select_duplex(ks8993_port_t port,ks8993_duplex_t mode)
{
  switch(port)
  {
    case PORT_1:
      {
        gpio_init_pp(MRXD1);
        if(mode == HALF_DUPLEX)
        {
          gpio_low(MRXD1);
        }
        else
        {
          gpio_high(MRXD1);
        }
      }
      break;
    case PORT_2:
      {
        gpio_init_pp(MRXD2);
        if(mode == HALF_DUPLEX)
        {
          gpio_low(MRXD2);
        }
        else
        {
          gpio_high(MRXD2);
        }
      }
      break;
    case PORT_3:
      {
        gpio_init_pp(MRXD3);
        if(mode == HALF_DUPLEX)
        {
          gpio_low(MRXD3);
        }
        else
        {
          gpio_high(MRXD3);
        }
      }
      break;
    default:
      return -1;
  }
  return 0;
}
int ks8993_select_speed(ks8993_port_t port,ks8993_speed_t mode)
{
  switch(port)
  {
    case PORT_1:
      {
        gpio_init_pp(LED30);
        if(mode == SPEED_10)
        {
          gpio_low(LED30);
        }
        else
        {
          gpio_high(LED30);
        }
      }
      break;
    case PORT_2:
      {
        gpio_init_pp(LED31);
        if(mode == SPEED_10)
        {
          gpio_low(LED31);
        }
        else
        {
          gpio_high(LED31);
        }
      }
      break;
    case PORT_3:
      {
        gpio_init_pp(LED32);
        if(mode == SPEED_10)
        {
          gpio_low(LED32);
        }
        else
        {
          gpio_high(LED32);
        }
      }
      break;
    default:
      return -1;
  }
  return 0;
}


int ks8993_select_autonegotiation(ks8993_port_t port,ks8993_autonegotiation_t mode)
{
  switch(port)
  {
    case PORT_1:
      {
        gpio_init_pp(MRXD0);
        if(mode == AUTO_ENABLE)
        {
          gpio_low(MRXD0);
        }
        else
        {
          gpio_high(MRXD0);
        }
      }
      break;
    case PORT_2:
      {
        gpio_init_pp(MCOL);
        if(mode == AUTO_ENABLE)
        {
          gpio_low(MCOL);
        }
        else
        {
          gpio_high(MCOL);
        }
      }
      break;
    case PORT_3:
      {
        gpio_init_pp(DISAN3);
        if(mode == AUTO_ENABLE)
        {
          gpio_low(DISAN3);
        }
        else
        {
          gpio_high(DISAN3);
        }
      }
      break;
    default:
      return -1;
  }
  return 0;
}
int ks8993_select_flow_control_in_auto(int enable)
{
  gpio_init_pp(LED13);
  if(enable)
  {
    gpio_high(LED13);
  }
  else
  {
    gpio_low(LED13);
  }
  return 0;
}
int ks8993_select_buffer_per_port_allocation(int enable)
{
  gpio_init_pp(LED10);
  if(enable)
  {
    gpio_high(LED10); // adaptive mode
  }
  else
  {
    gpio_low(LED10); // static mode 170 buffers per port
  }
  return 0;
}
int ks8993_select_mac_table_aging(int enable)
{
  gpio_init_pp(LED23);
  if(enable)
  {
    gpio_high(LED23); // aging enable, 5 minutes
  }
  else
  {
    gpio_low(LED23); // aging disable
  }
  return 0;
}
int ks8993_select_back_pressure_at_reset(int enable)
{
  gpio_init_pp(LED22);
  if(enable)
  {
    gpio_high(LED22); // back pressure enable at reset time
  }
  else
  {
    gpio_low(LED22); // disable
  }
  return 0;
}
int ks8993_select_back_off_hf_at_reset(int enable)
{
  gpio_init_pp(LED21);
  if(enable)
  {
    gpio_high(LED21); // aggressive back off enable in half duplex at reset time
  }
  else
  {
    gpio_low(LED21); // disable
  }
  return 0;
}
int ks8993_select_no_excessive_collision_drop_at_reset(int enable)
{
  gpio_init_pp(LED20);
  if(enable)
  {
    gpio_high(LED20); // no excessive collision drop enable at reset time
  }
  else
  {
    gpio_low(LED20); // disable
  }
  return 0;
}

  
//  
//  gpio_init_pp(P1_1PEN);
//
//  gpio_init_pp(PRSEL1);
//  gpio_init_pp(PRSEL0);
//
//  gpio_init_pp(PRSV);
//  gpio_init_pp(VMDIS);
//  gpio_init_pp(PBASE2);
//  gpio_init_pp(PBASE1);
//  gpio_init_pp(PBASE0);
//
//  gpio_init_pp(P3_TXQ2);
//  gpio_init_pp(P2_TXQ2);
//  gpio_init_pp(P1_TXQ2);
//
//  gpio_init_pp(P3_PP);
//  gpio_init_pp(P2_PP);
//  gpio_init_pp(P1_PP);
//
//
//  gpio_init_pp(P1_TAGINS);
//  gpio_init_pp(P2_TAGINS);
//  gpio_init_pp(P3_TAGINS);
//  gpio_init_pp(P3_TAGRM);
//  gpio_init_pp(P2_TAGRM);
//  gpio_init_pp(P1_TAGRM);
//static void delay(uint32_t pause)
//{
//  volatile uint32_t temp=0;
//  while(temp < pause)
//  {
//    temp++;
//  }
//}

static void ks8993_init_gpio(void)
{
  
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | 
                  RCC_AHB1ENR_GPIOBEN | 
                  RCC_AHB1ENR_GPIOCEN | 
                  RCC_AHB1ENR_GPIODEN | 
                  RCC_AHB1ENR_GPIOEEN | 
                  RCC_AHB1ENR_GPIOFEN | 
                  RCC_AHB1ENR_GPIOGEN;

}

//static void ks8993_init_settings()
//{
//
//  ks8993_select_mode_mii(MII_EXT_MAC_MODE);
//
//  ks8993_select_mode_led(LED_MODE_1); 
//
//  ks8993_select_duplex(PORT_1,FULL_DUPLEX);
//  ks8993_select_duplex(PORT_2,FULL_DUPLEX);
//  ks8993_select_duplex(PORT_3,FULL_DUPLEX);
////  ks8993_select_duplex(PORT_1,HALF_DUPLEX);
////  ks8993_select_duplex(PORT_2,HALF_DUPLEX);
////  ks8993_select_duplex(PORT_3,HALF_DUPLEX);
//  
//  ks8993_select_speed(P1,SPEED_100);
//  ks8993_select_speed(P2,SPEED_100);
//  ks8993_select_speed(P3,SPEED_100);
//  
//  ks8993_select_autonegotiation(P1,AUTO_ENABLE);
//  ks8993_select_autonegotiation(P2,AUTO_ENABLE);
//  ks8993_select_autonegotiation(P3,AUTO_ENABLE);
//  
//  ks8993_select_flow_control_in_auto(1);
//  
//}

int ks8993_init( void )
{
  ks8993_init_gpio();
  
//  // Выбор конфигурации объединения каналов
//  ks8993_select_group(VLAN_G123); 
//  
//  // Сбросить свитч в режим настройки, RESET = 0 
//  ks8993_reset_on(); 
//  
//  delay(100000); 
//  
//  // Вывести настройки на выводы
//  ks8993_init_settings();
//  
//  
//  delay(100000);
//  
//  // Восстановить работу свитча, RESET = 1
//  // Применение настроек
//  ks8993_reset_off();
//  delay(1000);

  return 0;
}

int ks8993_deinit( void )
{
  return 0;
}