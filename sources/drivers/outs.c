/* #include "extio.h"
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
    { NULL ,  0 },
  #else
    { GPIOA, 11 }
  #endif
#endif
};


static struct leds_list_s Outs[] =
{
    {.gpio=GPIOC ,.pin= 11,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //LED SVOBODA
    {.gpio=GPIOD ,.pin= 0 ,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, //Rele 1 clean cam
    {.gpio=GPIOD ,.pin= 1 ,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // Power cam termo
    {.gpio=GPIOE ,.pin= 13,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // L1
    {.gpio=GPIOE ,.pin= 14,.time=0,.timeout=0,.mode=EXTIO_LED_OFF}, // L2
} */