//******************************************************************************
// Имя файла    :       extio.c
// Заголовок    :
// Автор        :       Вахрушев Д.В.
// Дата         :       19.02.2016
//
//------------------------------------------------------------------------------
//
//
//******************************************************************************
#include "extio.h"
#include "main.h"
#include "stm32f4xx.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

struct leds_list_s {
  GPIO_TypeDef          *gpio;
  int                    pin;
  int                    time;
  int                    timeout; //Для отслеживания приема в Web
  enum extio_led_mode_e  mode;
  enum extio_led_mode_e  state;
};

static struct leds_list_s leds[] =
{
#if (MKPSH10 != 0)
  {.gpio=GPIOA,.pin=12,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth1
  {.gpio=GPIOB,.pin= 7,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth2
  {.gpio=GPIOC,.pin=13,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth3
  {.gpio=GPIOC,.pin= 1,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth4
  {.gpio=GPIOB,.pin= 9,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
  {.gpio=GPIOE,.pin= 5,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth6
  {.gpio=GPIOB,.pin= 8,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth7

  {.gpio=GPIOG,.pin= 3,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.1
  {.gpio=GPIOD,.pin= 5,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.2
  {.gpio=GPIOG,.pin= 2,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS232

  {.gpio=GPIOE,.pin= 6,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //MC (?)
#endif
#if (IMC_FTX_MC != 0)
  #if (PIXEL !=0)
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth1
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth2
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth3
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth4
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth6
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth7
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.2
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.1  // Not connect in Pixel
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS232
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //MC (?)
  #else
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth1
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth2
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth3
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth4
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth6
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth7

    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.2
    {.gpio=GPIOA,.pin= 8,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.1
    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS232

    {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //MC (?)
  #endif
#endif
#if (UTD_M != 0)
  {.gpio=GPIOB,.pin= 9,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth1
  {.gpio=GPIOE,.pin= 1,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth2
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth3
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth4
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth6
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth7

  {.gpio=GPIOD,.pin=12,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.2
  {.gpio=GPIOD,.pin=13,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS485.1
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY RS232

  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //MC (?)
#endif
#if (IIP != 0)
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth1
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth2
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth3
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth4
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth5
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth6
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //RDY Eth7

  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //LED1
  {.gpio=NULL ,.pin= 0,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //LED2
#endif
};

static void extio_task(void *args);

void extio_gpio_out(GPIO_TypeDef *gpio, uint8_t pin, uint8_t set )
{
  if (gpio == NULL)
    return;
  if(set)
  {
    gpio->BSRRL = (1<<pin);
  }
  else
  {
    gpio->BSRRH = (1<<pin);
  }
}

void extio_gpio_init(GPIO_TypeDef *gpio, uint8_t pin, uint8_t mode, uint8_t otype, uint8_t ospeed, uint8_t pupd, uint8_t af, uint8_t lock  )
{
  unsigned char reg=0;
  gpio->LCKR &= ~(1<<pin);
  gpio->MODER &= ~(3<<(pin<<1));
  gpio->MODER |=  (mode<<(pin<<1));
  gpio->OTYPER &= ~(1<<pin);
  gpio->OTYPER |=  (otype<<pin);
  gpio->OSPEEDR &= ~(3<<(pin<<1));
  gpio->OSPEEDR |=  (ospeed<<(pin<<1));
  gpio->PUPDR &= ~(3<<(pin<<1));
  gpio->PUPDR |=  (pupd<<(pin<<1));
  if(pin>7){pin-=8;reg=1;}
  gpio->AFR[reg] &=  ~(0xF<<(pin<<2));
  gpio->AFR[reg] |=   ((0xF&(af))<<(pin<<2));
  gpio->LCKR |= (lock<<pin);
}

#if (UTD_M != 0)
  #include "snmp_client.h"
#endif
void extio_init(void)
{
  RCC->AHB1RSTR |=   RCC_AHB1RSTR_GPIOARST |
                     RCC_AHB1RSTR_GPIOBRST |
                     RCC_AHB1RSTR_GPIOCRST |
                     RCC_AHB1RSTR_GPIODRST |
                     RCC_AHB1RSTR_GPIOERST
#if (UTD_M == 0)
                       |
                     RCC_AHB1RSTR_GPIOFRST |
                     RCC_AHB1RSTR_GPIOGRST |
                     RCC_AHB1RSTR_GPIOHRST |
                     RCC_AHB1RSTR_GPIOIRST
#endif
#ifdef LQFP_144
                       |
                     RCC_AHB1RSTR_GPIOJRST |
                     RCC_AHB1RSTR_GPIOKRST
#endif
                       ;

  RCC->AHB1RSTR &= ~(RCC_AHB1RSTR_GPIOARST |
                     RCC_AHB1RSTR_GPIOBRST |
                     RCC_AHB1RSTR_GPIOCRST |
                     RCC_AHB1RSTR_GPIODRST |
                     RCC_AHB1RSTR_GPIOERST
#if (UTD_M == 0)
                       |
                     RCC_AHB1RSTR_GPIOFRST |
                     RCC_AHB1RSTR_GPIOGRST |
                     RCC_AHB1RSTR_GPIOHRST |
                     RCC_AHB1RSTR_GPIOIRST
#endif
#ifdef LQFP_144
                       |
                     RCC_AHB1RSTR_GPIOJRST |
                     RCC_AHB1RSTR_GPIOKRST
#endif
                       ) ;

  RCC->AHB1ENR |=    RCC_AHB1ENR_GPIOAEN |
                     RCC_AHB1ENR_GPIOBEN |
                     RCC_AHB1ENR_GPIOCEN |
                     RCC_AHB1ENR_GPIODEN |
                     RCC_AHB1ENR_GPIOEEN
#if ((UTD_M == 0) && (IIP == 0))
                       |
                     RCC_AHB1ENR_GPIOFEN |
                     RCC_AHB1ENR_GPIOGEN |
                     RCC_AHB1ENR_GPIOHEN |
                     RCC_AHB1ENR_GPIOIEN
#endif
#ifdef LQFP_144
                       |
                     RCC_AHB1ENR_GPIOJEN |
                     RCC_AHB1ENR_GPIOKEN
#endif
                       ;

#ifdef USE_SWITCH
  // only for  MKPSH
  SetEthOnPins();   //Установить пины с пассивным уровнем = 1 для Ethernet
#endif

#if (MKPSH10 != 0)
  GPIO_INIT(GPIOA,12,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth1
  GPIO_INIT(GPIOB, 7,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth2
  GPIO_INIT(GPIOC,13,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth3
  GPIO_INIT(GPIOC, 1,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth4
  GPIO_INIT(GPIOB, 9,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth5
  GPIO_INIT(GPIOE, 5,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth6
  GPIO_INIT(GPIOB, 8,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_Eth7
  GPIO_INIT(GPIOG, 2,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_RS232
  GPIO_INIT(GPIOG, 3,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_RS485.1
  GPIO_INIT(GPIOD, 5,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // RDY_RS485.2


  GPIO_INIT(GPIOE, 6,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // MС

  GPIO_INIT(GPIOC, 8,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // MODEM

  GPIO_INIT(GPIOF, 7,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // CPU_3.3V1
  GPIO_INIT(GPIOG,11,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_PU,0,0); // CPU_3.3V2
  GPIO_INIT(GPIOC, 9,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // CPU_3.3V3
  GPIO_INIT(GPIOA,11,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // CPU_3.3V4
  GPIO_INIT(GPIOD, 4,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // CPU_3.3V5
#endif

//  /// Only for IMX FTX
//  GPIO_INIT(GPIOA,8,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // VD Rezhim 1
//  GPIO_INIT(GPIOC,9,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // VD Rezhim 2
#if (IMC_FTX_MC != 0)
  #ifndef PIXEL
    GPIO_INIT(GPIOA,8,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // VD Rezhim 1
    GPIO_INIT(GPIOC,9,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // VD Rezhim 2
  #else
    GPIO_INIT(GPIOD,0,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);    //  Clean Drive mode
    GPIO_INIT(GPIOD,1,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);    // power Cam
    GPIO_INIT(GPIOD,2,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);    // power Cam
    GPIO_INIT(GPIOD,3,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);    //  X\Y  set
    GPIO_INIT(GPIOD,8 ,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);    //  PWM move
    GPIO_INIT(GPIOC,11,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);   // VD svobodnogo

    GPIO_INIT(GPIOE,11,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);   // PWM move
    GPIO_INIT(GPIOE,12,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);   // move forward/back
    GPIO_INIT(GPIOE,13,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);   // Light 1
    GPIO_INIT(GPIOE,14,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0);   // Light 2
  #endif
#endif

#if (UTD_M != 0) // !!! + IIP
  //Сигнализация передачи по RS485
  GPIO_INIT(GPIOD,13,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // LED1
  GPIO_INIT(GPIOD,12,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // LED2
  GPIO_INIT(GPIOD,14,MODE_OUT,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // Red
  //Питание RS485 входы
  GPIO_INIT(GPIOA,12,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // !RS485_ON1
  GPIO_INIT(GPIOD,15,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // !RS485_ON2
  //Питание WiFi входы
  GPIO_INIT(GPIOD, 6,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // PWR_WIFI1
  GPIO_INIT(GPIOD, 7,MODE_IN,OTYPE_PP,OSPEED_VHS,PUPD_N,0,0); // PWR_WIFI2
  //Индикация состояния Ethernet портов 2,3
  InitEthStateGpio();
#endif

//  for(int i=0;i<sizeof(leds)/sizeof(leds[0]);i++)
//  {
//    extio_gpio_out(leds[i].gpio, leds[i].pin, 1 );
//  }

//  xTaskCreate(extio_task,
//              (int8_t *)"ExtIO",
//              configMINIMAL_STACK_SIZE * 2,
//              NULL,LED_TASK_PRIO,
//              NULL);
}

typedef struct sGPIO_Pin {
  GPIO_TypeDef  * Gpio;
  uint8_t         Pin;  //= GPIO_Pin - 1
} SGPIO_Pin;

/*
 EXTIO_CPU_33_V1,  //SWITCH_1
  EXTIO_CPU_33_V2,  //SWITCH_2
  EXTIO_CPU_33_V3,  //Модем
  EXTIO_CPU_33_V4,  //RS485_2
  EXTIO_CPU_33_V5,  //RS485_1
  */
struct sGPIO_Pin aEXTIO_LedPin[EXTIO_Count] = {

  { (GPIO_TypeDef *)1,  0 },    //Всегда 1
  { NULL ,  0 },
  { NULL ,  0 },

  //EXTIO_CPU_33_V4,  //RS485_2
#if (IMC_FTX_MC != 0)
  { NULL ,  0 },
#endif
 //EXTIO_CPU_33_V5,  //RS485_1

#if (IMC_FTX_MC != 0)
  #if PIXEL
    { NULL ,  0 },    // нужно задать ножку включения внешнего питания
  #else
    { GPIOA, 11 }
  #endif
#endif
};
// состояние  ножки
int extio_read_idx(enum extio_in_e pin)
{
  if (pin >= EXTIO_Count)
    return -1;
  if (aEXTIO_LedPin[pin].Gpio == NULL)
    return -1;
  if (aEXTIO_LedPin[pin].Gpio == (GPIO_TypeDef *)1)
    return 1;
  uint32_t value = 0;
  value = aEXTIO_LedPin[pin].Gpio->IDR;
  return (value >> aEXTIO_LedPin[pin].Pin)&1;
}

/** **/
int extio_led(enum extio_led_e led, enum extio_led_mode_e mode)
{
#if (IMC_FTX_MC != 0)
  if ((led >= EXTIO_ETH3) && (led != EXTIO_RS485_1)) {
    return 0;
  }
#endif
  switch(leds[led].mode = leds[led].state = mode)
  {
    case EXTIO_LED_ON:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 1 );
        leds[led].time = 0;
      }
      break;
    case EXTIO_LED_OFF:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 0 );
        leds[led].time = 0;
      }
      break;
    case EXTIO_LED_BLINK:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 1 );
        leds[led].state = EXTIO_LED_ON;
        leds[led].time = DEFAULT_LED_BLINK_TIME;
      }
      break;
    case EXTIO_LED_BLINK_FAST:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 1 );
        leds[led].state = EXTIO_LED_ON;
        leds[led].time = DEFAULT_LED_BLINK_FAST_TIME;
      }
      break;
    case EXTIO_LED_BLINK_ONCE:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 1 );
        leds[led].state = EXTIO_LED_ON;
        leds[led].time = DEFAULT_LED_BLINK_ONCE_TIME;
      }
      break;
    case EXTIO_LED_BLINK_ONCE_ON:
      {
        extio_gpio_out(leds[led].gpio, leds[led].pin, 0 );
        leds[led].state = EXTIO_LED_OFF;
        leds[led].time = DEFAULT_LED_BLINK_ONCE_TIME;
        leds[led].timeout = DEFAULT_LED_ON_TIMEOUT;
      }
      break;
    default:
      return -1;

  }
  return 0;
}

int extio_led_blinked(enum extio_led_e led) {
  //Значение таймаута (= наличие приема)
  return leds[led].timeout;
}

void extio_task(void *args)
{
  vTaskDelay(5000);
  while(1)
  {
    for(int i=0;i<sizeof(leds)/sizeof(leds[0]);i++)
    {
      if(leds[i].state == EXTIO_LED_OFF)
      {
        extio_gpio_out(leds[i].gpio, leds[i].pin, 0 );
      }
      else
      {
        extio_gpio_out(leds[i].gpio, leds[i].pin, 1 );
      }
      switch(leds[i].mode)
      {
        case EXTIO_LED_ON:
          {
            extio_gpio_out(leds[i].gpio, leds[i].pin, 1 );
            leds[i].time = 0;
          }
          break;
        case EXTIO_LED_OFF:
          {
            extio_gpio_out(leds[i].gpio, leds[i].pin, 0 );
            leds[i].time = 0;
          }
          break;
        case EXTIO_LED_BLINK:
          {
            if(leds[i].time<=0)
            {
              leds[i].time = DEFAULT_LED_BLINK_TIME;
              leds[i].state = (leds[i].state)?EXTIO_LED_OFF:EXTIO_LED_ON;
            }
          }
          break;
        case EXTIO_LED_BLINK_FAST:
          {
            if(leds[i].time<=0)
            {
              leds[i].time = DEFAULT_LED_BLINK_FAST_TIME;
              leds[i].state = (leds[i].state)?EXTIO_LED_OFF:EXTIO_LED_ON;
            }
          }
          break;
        case EXTIO_LED_BLINK_ONCE:
          {
            if(leds[i].time<=0)
            {
              leds[i].mode = leds[i].state = EXTIO_LED_OFF;
              leds[i].time = 0;
            }
          }
          break;
        case EXTIO_LED_BLINK_ONCE_ON:
          {
            if(leds[i].time<=0)
            {
              leds[i].mode = leds[i].state = EXTIO_LED_ON;
              leds[i].time = 0;
            }
          }
          break;
      }
      if(leds[i].time)
            leds[i].time-=DEFAULT_LED_SUB;
      if(leds[i].timeout)
            leds[i].timeout-=DEFAULT_LED_SUB;
    }
    vTaskDelay(DEFAULT_LED_SUB);
  }
}

void gpio_init_pp(GPIO_TypeDef *gpio, int pin)
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

void gpio_set_pp(GPIO_TypeDef *gpio, int pin, int enabled)
{
  if(enabled)
  {
    gpio->BSRRL = 1<<pin;
  }
  else
  {
    gpio->BSRRH = 1<<pin;
  }
}

