/**
  ******************************************************************************
  * @file    shdsl.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    20.05.2015
  * @brief   --
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; Copyright (C) 2014 by  Dmitry Vakhrushev (vdv.18@mail.ru) </center></h2>
  ******************************************************************************
  */ 

#include "shdsl.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"

#include "stm32f4xx.h"

#include "uart.h"
#include "timer.h"
#include "timer_hw_defined.h"
    
#include "leds.h"
#include "buttons.h"

static shdsl_cfg_t main_cfg, main_cfg_new;
static shdsl_stat_t main_stat;
static uint8_t main_cfg_update = 0;
static uint8_t main_cfg_update_complete = 0;

// просмотр состояния соединения // работа модема
static uint32_t modem_connect_txrx = 0;

static err_hw_t shdsl_hw_init(void);
static err_hw_t shdsl_off();
static err_hw_t shdsl_on();
static unsigned int shdsl_enabled = 0;
static unsigned int shdsl_init_en = 0;

static timer_t timer_recieve_timerout = -1;
static timer_t timer_shdsl_status_update = 0;
static timer_t timer_shdsl_restart_rs232_lost = -1;

typedef enum shdsl_cmd_list_e {
  SHDSL_CMD_NONE        =       0,
  SHDSL_CMD_CFG         =       1,
  SHDSL_CMD_INFO        =       2,
  SHDSL_CMD_STAT        =       3,
  SHDSL_CMD_RATE        =       4,
  SHDSL_CMD_MASTER      =       5,
  SHDSL_CMD_SLAVE       =       6,
  SHDSL_CMD_TCPAM       =       7,
  SHDSL_CMD_PBO_FORCED  =       8,
  SHDSL_CMD_PBO_NORMAL  =       9,
  SHDSL_CMD_SYNC        =       10,
  SHDSL_CMD_PLESIO      =       11,
  SHDSL_CMD_PLESIO_REF  =       12,
} shdsl_cmd_list_t;

typedef struct shdsl_cmd_str_s {
  shdsl_cmd_list_t      cmd;
  const char            *str;
  uint32_t              value;
  //int                   len;
} shdsl_cmd_str_t;



const char write_cfg_str[]           = "cfg";
const char write_info_str[]          = "info";
const char write_stat_str[]          = "stat";
const char write_rate_str[]          = "rate";
const char write_master_str[]        = "master";
const char write_slave_str[]         = "slave";
static shdsl_cmd_str_t cmd_str_arr[] = {
  { 
    .cmd = SHDSL_CMD_NONE,
    .str = "",
  },
  { 
    .cmd = SHDSL_CMD_CFG,
    .str = "cfg",
  },
  { 
    .cmd = SHDSL_CMD_INFO,
    .str = "info",
  },
  { 
    .cmd = SHDSL_CMD_STAT,
    .str = "stat",
  },
  { 
    .cmd = SHDSL_CMD_RATE,
    .str = "rate",
    .value = 0,
  },
  { 
    .cmd = SHDSL_CMD_MASTER,
    .str = "master",
  },
  { 
    .cmd = SHDSL_CMD_SLAVE,
    .str = "slave",
  },
  { 
    .cmd = SHDSL_CMD_TCPAM,
    .str = "tcpam",
    .value = 0,
  },
  { 
    .cmd = SHDSL_CMD_PBO_FORCED,
    .str = "pbo-forced",
    .value = 0,
  },
  { 
    .cmd = SHDSL_CMD_PBO_NORMAL,
    .str = "pbo-normal",
  },
  { 
    .cmd = SHDSL_CMD_SYNC,
    .str = "sync",
  },
  { 
    .cmd = SHDSL_CMD_PLESIO,
    .str = "plesio",
  },
  { 
    .cmd = SHDSL_CMD_PLESIO_REF,
    .str = "plesio-ref",
  },
};

static shdsl_cmd_list_t write_commands[10];
static int write_commands_index = 0;
static void buffer_command_init(){
  for(int i=0;i<sizeof(write_commands)/sizeof(shdsl_cmd_list_t);i++)
  {
    write_commands[i] = SHDSL_CMD_NONE;
  }
}
static void bufer_command_delete(){
  if(write_commands_index>0)
    write_commands[write_commands_index--]=SHDSL_CMD_NONE;
}
static int bufer_command_count(){
  return write_commands_index;
}
static shdsl_cmd_list_t buffer_command_get()
{
  if(write_commands_index>0){
    return write_commands[write_commands_index-1];
  } else return SHDSL_CMD_NONE;
}
static void bufer_command_put(shdsl_cmd_list_t cmd, uint32_t value)
{
  int j=0;
  int index=0;
  for(index=0;index<write_commands_index;index++)
  {
    if(write_commands[index] == cmd)
      return;
  }
  if(write_commands_index >= sizeof(write_commands)/sizeof(shdsl_cmd_list_t))
  {
    write_commands[sizeof(write_commands)/sizeof(shdsl_cmd_list_t)-1] = cmd;
  }
  else
  {
    write_commands[write_commands_index++] = cmd;
  }
  for(j=0;j<=sizeof(cmd_str_arr)/sizeof(shdsl_cmd_str_t);j++)
  {
    if(write_commands[write_commands_index-1] == cmd_str_arr[j].cmd)
    {
       cmd_str_arr[j].value = value;
       goto finish;
    }
  }
finish:
}

static shdsl_cmd_list_t write_cmd = SHDSL_CMD_NONE;
static uint32_t write_cmd_rate = 0x00;
const uint32_t shdsl_rate_select[16] = {
  192,
  1024,
  2048,
  3072,
  4096,
  5120,
  6144,
  7168,
  8192,
  9216,
  10240,
  11264,
  12288,
  13312,
  14336,
  15360,
};


static uint32_t write_cmd_tcpam = 0x00;
const uint32_t shdsl_tcpam_select[4] = {
  16,
  32,
  64,
  128,
};

static uint32_t write_cmd_pbo_forced = 0x00;

static void shdsl_rate_led( uint32_t rate ){
  for(int i=0;i<sizeof(shdsl_rate_select);i++){
    if(shdsl_rate_select[i] >= rate){
      leds_set(i&(0x0F));
      break;
    }
  }
}
err_hw_t shdsl_rate( uint32_t rate )
{
  write_cmd = SHDSL_CMD_RATE;
  bufer_command_put(SHDSL_CMD_RATE,rate);
  write_cmd_rate = rate;
  shdsl_rate_led(rate);
  return HW_SUCCESS;
}

err_hw_t shdsl_pbo( uint32_t value )
{
  write_cmd = SHDSL_CMD_RATE;
  if(value >= 0 && value <=30)
  {
    bufer_command_put(SHDSL_CMD_PBO_FORCED, value);
  }
  else
  {
    bufer_command_put(SHDSL_CMD_PBO_NORMAL, 0);
  }
  write_cmd_pbo_forced = value;
  return HW_SUCCESS;
}

err_hw_t shdsl_tcpam( uint32_t tcpam )
{
  write_cmd = SHDSL_CMD_RATE;
  bufer_command_put(SHDSL_CMD_TCPAM, tcpam);
  write_cmd_tcpam = tcpam;
  return HW_SUCCESS;
}

err_hw_t shdsl_sync( uint32_t value )
{
  switch(value)
  {
  case 0:bufer_command_put(SHDSL_CMD_SYNC, 0);
    break;
  case 1:bufer_command_put(SHDSL_CMD_PLESIO, 0);
    break;
  case 2:bufer_command_put(SHDSL_CMD_PLESIO_REF, 0);
    break;
  default:bufer_command_put(SHDSL_CMD_SYNC, 0);
    break;
  }
  return HW_SUCCESS;
}
err_hw_t shdsl_annex( uint32_t value )
{
  return HW_SUCCESS;
}
err_hw_t shdsl_mode( uint32_t mode )
{
  if(mode){
    write_cmd = SHDSL_CMD_MASTER;
    bufer_command_put(SHDSL_CMD_MASTER,0);
  } else {
    write_cmd = SHDSL_CMD_SLAVE;
    bufer_command_put(SHDSL_CMD_SLAVE,0);
  }
  return HW_SUCCESS;
}

static void timer_shdsl_cmd_update_callback( timer_t timer )
{
  const shdsl_cmd_list_t repeat_cmd[] = {SHDSL_CMD_CFG,SHDSL_CMD_INFO,SHDSL_CMD_STAT};
  if(timer == timer_shdsl_restart_rs232_lost)
  {
//    while (1)
//    {
//      NVIC_SystemReset();
//    }
  }
  if(timer == timer_shdsl_status_update)
  {
    int length = 0;
    static uint8_t  cmd_index = 0;
    char buffer[200];
    memset(buffer,0,100);
    if(modem_connect_txrx<10) modem_connect_txrx++;
//    if(modem_connect_txrx>3)
//    {
//      if(timer_enable(timer_shdsl_restart_rs232_lost) != TIMER_OK)
//      {
//        timer_shdsl_restart_rs232_lost 
//          = timer_create(TIMER_REPEAT_START, TIMER_SECOND(60), timer_shdsl_cmd_update_callback);
//      }
//      //ksz8895fmq_port_pwr_en(1,1);// Отключить порт
//    }
//    else
//    {
//      if(timer_enable(timer_shdsl_restart_rs232_lost) == TIMER_OK)
//      {
//        timer_delete(&timer_shdsl_restart_rs232_lost);
//      }
//      //ksz8895fmq_port_pwr_en(1,0);// Включить порт
//    }
    write_cmd = buffer_command_get();
    if(write_cmd == SHDSL_CMD_NONE){
      length = sprintf(buffer,"%s\r\n",cmd_str_arr[repeat_cmd[cmd_index]].str);
      //memcpy(buffer,cmd_str_arr[cmd_index].str,cmd_str_arr[cmd_index].len);
      //uart_write(buffer,cmd_str_arr[cmd_index].len);
      cmd_index++;
      if(cmd_index >= sizeof(repeat_cmd)/sizeof(shdsl_cmd_list_t)){
        cmd_index = 0;
      }
    } else {
      if(bufer_command_count()>0){
        write_cmd = buffer_command_get();
        switch(write_cmd){
          case SHDSL_CMD_RATE:
            length += sprintf((char*)&buffer[length],"%s %d ",cmd_str_arr[write_cmd].str,cmd_str_arr[write_cmd].value);
            break;
          case SHDSL_CMD_PBO_FORCED:
            length += sprintf((char*)&buffer[length],"%s %d ",cmd_str_arr[write_cmd].str,cmd_str_arr[write_cmd].value);
            break;
          case SHDSL_CMD_PBO_NORMAL:
            length += sprintf((char*)&buffer[length],"%s",cmd_str_arr[write_cmd].str);
            break;
          case SHDSL_CMD_TCPAM:
            length += sprintf((char*)&buffer[length],"%s%d ",cmd_str_arr[write_cmd].str,cmd_str_arr[write_cmd].value);
            break;
          case SHDSL_CMD_SYNC:
          case SHDSL_CMD_PLESIO:
          case SHDSL_CMD_PLESIO_REF:
          case SHDSL_CMD_MASTER:
          case SHDSL_CMD_SLAVE:
            length += sprintf((char*)&buffer[length],"%s ",cmd_str_arr[write_cmd].str);
            break;
        };
        bufer_command_delete();
      }
      length += sprintf((char*)&buffer[length],"\r\n");
    }
    uart_write((uint8_t*)buffer, length);
  }
  else if(timer == timer_recieve_timerout)
  {
    char *value[100];
    char buffer[512],*bufer_index;
    int set = 0; 
    int length = uart_buffer_rx_len();
    int length_index,space_index,space_cnt;
    if(uart_get_rx_buffer((uint8_t *)buffer) != HW_SUCCESS) return;
    modem_connect_txrx = 0;
    bufer_index = buffer;
    length_index = length;
    space_cnt = 0;
    set = 1;
    while(length_index-->0){
      if(*bufer_index == ' ' || 
         *bufer_index == '\t' || 
         *bufer_index == '=' || 
         *bufer_index == '\r' || 
         *bufer_index == '\n' || 
         *bufer_index == ':' || 
         *bufer_index == '>' || 
         *bufer_index == '<' || 
         *bufer_index == '/' || 
         *bufer_index == '\\' || 
         *bufer_index == '|' || 
         *bufer_index == 0)
      {
        *bufer_index = 0x00;
        set = 1;
      } else {
        *bufer_index = toupper(*bufer_index);
        if(set && (*bufer_index!=0)){
          set = 0;
          value[space_cnt++] = bufer_index;
        }
      }
      bufer_index++;
    }
    length_index = length;
    space_index = 0;
    while(space_index<space_cnt){
      if(strcmp(value[space_index], "MASTER") == 0){
          main_cfg.master = 1;
      } else if(strcmp(value[space_index] , "SLAVE") == 0){
          main_cfg.master = 0;
      } else if(strcmp(value[space_index] , "TCPAM16") == 0){
          main_cfg.tcpam = 16;
      } else if(strcmp(value[space_index] , "TCPAM32") == 0){
          main_cfg.tcpam = 32;
      } else if(strcmp(value[space_index] , "TCPAM64") == 0){
          main_cfg.tcpam = 64;
      } else if(strcmp(value[space_index] , "TCPAM128") == 0){
          main_cfg.tcpam = 128;
      } else if(strcmp(value[space_index] , "RATE") == 0){
        int rate = 0;
        if(sscanf(value[space_index+1],"%d",&rate)>0){
          shdsl_rate_led(rate);
          main_cfg.rate = rate;
          space_index++;
        }
      } else if(strcmp(value[space_index] , "ANNEX") == 0){
        if(strcmp(value[space_index+1] , "A") == 0){
          main_cfg.annex = 0;
          space_index++;
        } else if(strcmp(value[space_index+1] , "B") == 0) {
          main_cfg.annex = 1;
          space_index++;
        }
      }  else if(strcmp(value[space_index] , "DSL_LINK") == 0){
        if(strcmp(value[space_index+1] , "ONLINE") == 0){
          main_stat.dsl_link = 1;
          
          //ksz8895fmq_port_tx_en(1,1);
          //ksz8895fmq_port_rx_en(1,1);
          ksz8895fmq_port_pwr_en(1,0); // Включить порт
          
          space_index++;
        } else if(strcmp(value[space_index+1] , "OFFLINE") == 0) {
          main_stat.dsl_link = 0;
          
          //ksz8895fmq_port_tx_en(1,0);
          //ksz8895fmq_port_rx_en(1,0);
          ksz8895fmq_port_pwr_en(1,1);// Отключить порт
          
          space_index++;
        }
      } else if(strcmp(value[space_index] , "DSL_LINK") == 0){
        if(strcmp(value[space_index+1] , "ONLINE") == 0){
          main_stat.dsl_link = 1;
          space_index++;
        } else if(strcmp(value[space_index+1] , "OFFLINE") == 0) {
          main_stat.dsl_link = 0;
          space_index++;
        }
      } else if(strcmp(value[space_index] , "CONFIGURATION") == 0){
        if(strcmp(value[space_index+1] , "COMPLETE") == 0){
          main_cfg_update_complete = 1;
          space_index++;
        }
      } else if(strcmp(value[space_index], "PBO-NORMAL") == 0){
        main_cfg.pbo=64;
      } else if(strcmp(value[space_index], "PBO-FORCED") == 0){
        int pbo = 0;
        if(sscanf(value[space_index+1],"%d",&pbo)>0){
          main_cfg.pbo=pbo;
          space_index++;
        }
      } else if(strcmp(value[space_index], "SYNC") == 0){
        main_cfg.sync=0;
      } else if(strcmp(value[space_index], "PLESIO") == 0){
        main_cfg.sync=1;
      } else if(strcmp(value[space_index], "PLESIO-REF") == 0){
        main_cfg.sync=2;
      }
      space_index++;         
    }
    uart_buffer_flush(uart_buffer_rx_len());
    timer_delete( &timer_recieve_timerout );
  }
}

static void timer_shdsl_init_callback(timer_t timer)
{
  shdsl_on();
  
  timer_shdsl_status_update = 
    timer_create(TIMER_REPEAT_START, TIMER_SECOND(3), timer_shdsl_cmd_update_callback);
  
  buffer_command_init();
}

err_hw_t shdsl_set_cfg( shdsl_cfg_t *cfg )
{
  memcpy(&main_cfg_new, cfg, sizeof(shdsl_cfg_t));
  main_cfg_update = 1;
  return HW_SUCCESS;
}

err_hw_t shdsl_get_cfg( shdsl_cfg_t *cfg )
{
  
  if(modem_connect_txrx > 3){
    memset(&main_cfg,0,sizeof(shdsl_cfg_t));
  }
  memcpy(cfg, &main_cfg, sizeof(shdsl_cfg_t));
  return HW_SUCCESS;
}


err_hw_t shdsl_get_stat( shdsl_stat_t *stat )
{
  if(modem_connect_txrx > 3){
    memset(&main_stat,0,sizeof(shdsl_stat_t));
  }
  memcpy(stat, &main_stat, sizeof(shdsl_stat_t));
  return HW_SUCCESS;
}

void uart_tx_complete_callback()
{
  //uart_bufer_flush(0);
}

void uart_rx_callback(uint8_t *buff, uint32_t len)
{
  /**
   * Прием данных по таймуту
   */
  if(len > 0){
    if(timer_enable( timer_recieve_timerout ) == TIMER_OK)
    {
      timer_reset( timer_recieve_timerout );
    }
    else 
    {
      timer_recieve_timerout = timer_create(TIMER_REPEAT_START, TIMER_MILLISECOND(10), timer_shdsl_cmd_update_callback);
    }
  }
}

/**
 * @brief shdsl initialization
 *
 * @retval none.
 *
 */
err_hw_t shdsl_init( void )
{
  if( shdsl_init_en == 1 ) return HW_ERROR;
  shdsl_init_en = 1;
  
  shdsl_hw_init();
  uart_set_callback(uart_tx_complete_callback,uart_rx_callback);
  uart_init();
  
  shdsl_off();
  
  timer_create(TIMER_ONE_SHOT_START, TIMER_SECOND(3), timer_shdsl_init_callback);
  
  return HW_SUCCESS;
}

err_hw_t shdsl_on()
{
  GPIOC->ODR |= GPIO_ODR_ODR_8;
  leds_set(LED_PWR);
  shdsl_enabled = 1;
  return HW_SUCCESS;
}

err_hw_t shdsl_off()
{
  GPIOC->ODR &= ~GPIO_ODR_ODR_8;
  leds_clr(LED_PWR);
  shdsl_enabled = 0;
  return HW_SUCCESS;
}


/**
 * @brief shdsl deinitialization
 *
 * @retval none.
 *
 */
err_hw_t shdsl_deinit( void )
{
  if( shdsl_init_en == 0 ) return HW_ERROR;
  shdsl_init_en = 0;
  {
    shdsl_off();
  }
  timer_delete(&timer_shdsl_status_update);
  return HW_SUCCESS;
}

/**
 * @brief shdsl command
 *
 * @param[in]    shdsl_cmd_t 		-	shdsl command.
 * @param[inout] ...  		                -	shdsl command parameters.
 *
 * @retval none.
 *
 */
err_hw_t shdsl_cmd( shdsl_cmd_t cmd, ... )
{
  va_list args;
  err_hw_t retval = HW_SUCCESS;
  if( shdsl_init_en == 0 ) return HW_ERROR;
  va_start(args,cmd);
  
  switch( cmd ) {
    case SHDSL_CMD_UPDATE:
      {
        /* unsigned int en = va_arg(args,unsigned int); */
        retval = HW_SUCCESS;
      }
      break;
    default:
      {
        retval = HW_ERROR;
      }
      break;
  };
  
  va_end(args);
  return retval;
}


err_hw_t shdsl_rate_set(uint32_t rate)
{
  return HW_SUCCESS;
}

//const uint32_t shdsl_rate_select[16];
//static void shdsl_rate_led( uint32_t rate ){
//  for(int i=0;i<sizeof(shdsl_rate_select);i++){
//    if(speed[i] >= rate){
//      leds_set(i&(0x0F));
//      break;
//    }
//  }
//}
timer_t shdsl_timer_set_rate = -1;
void shdsl_timer_set_rate_callback(timer_t _timer){
  if( shdsl_timer_set_rate == _timer ){
    shdsl_rate(main_cfg.rate);
    timer_delete(&shdsl_timer_set_rate);
  }
}
timer_t timer_reset_back_buttons = -1;

void button_select_callback(buttons_t button,buttons_switch_t state){
  //state BUTTON_ON;
  timer_reset(timer_reset_back_buttons);
  switch(button){
    case BUTTON_1:
      {
        if(state == BUTTON_ON){
        }else{
        }
      }
      break;
    case BUTTON_2:
      {
        if(state == BUTTON_ON){
          if(main_cfg.master)
          {
            for(int i=0;i<sizeof(shdsl_rate_select)/sizeof(shdsl_rate_select[0]);i++){
              if(shdsl_rate_select[i] >= main_cfg.rate){
                if(i >= sizeof(shdsl_rate_select)/sizeof(shdsl_rate_select[0])-1){
                  leds_clr(0x0F);
                  leds_set((0)&(0x0F));
                  main_cfg.rate = shdsl_rate_select[0];
                } else {
                  leds_clr(0x0F);
                  leds_set((i+1)&(0x0F));
                  main_cfg.rate = shdsl_rate_select[i+1];
                }
                if(!(timer_enable(shdsl_timer_set_rate))){
                  shdsl_timer_set_rate = timer_create(TIMER_ONE_SHOT_START, TIMER_MILLISECOND(3000), shdsl_timer_set_rate_callback);
                }
                break;
              }
            }
          }
        }else{
        }
      }
      break;
  }
}

struct sm_enter_edit_s {
  buttons_t button;
  buttons_switch_t state;
};
/***
 * Вход в режим изменения скорости
 */
static int enter_edit_speed_en = 0;
static struct sm_enter_edit_s enter_edit_speed[] = {
  {BUTTON_1,BUTTON_ON},
  {BUTTON_2,BUTTON_ON},
  {BUTTON_1,BUTTON_OFF},
  {BUTTON_2,BUTTON_OFF},
};
/***
 * Вход в режим изменения типа соединения
 */
static int enter_edit_mode_en = 0;
struct sm_enter_edit_s enter_edit_mode[] = {
  {BUTTON_2,BUTTON_ON},
  {BUTTON_1,BUTTON_ON},
  {BUTTON_2,BUTTON_OFF},
  {BUTTON_1,BUTTON_OFF},
};

static void button_callback(buttons_t button,buttons_switch_t state);
static void timer_reset_back_buttons_callback(timer_t _timer){
  if(_timer == timer_reset_back_buttons)
  {
      enter_edit_mode_en = 0;
      enter_edit_speed_en = 0;
      buttons_callback(button_callback,button_callback);
  }
}

void button_callback(buttons_t button,buttons_switch_t state){
  //state BUTTON_ON;
  // Доступ к изменению скорости
  if(button == enter_edit_speed[enter_edit_speed_en].button &&
     state  == enter_edit_speed[enter_edit_speed_en].state)
  {
    enter_edit_speed_en++;
    if(enter_edit_speed_en >= sizeof(enter_edit_speed)/sizeof(struct sm_enter_edit_s))
    {
      enter_edit_speed_en = 0;
      timer_reset_back_buttons = timer_create(TIMER_ONE_SHOT_START,TIMER_SECOND(10),timer_reset_back_buttons_callback);
      buttons_callback(button_select_callback,button_select_callback);
    }
  } else enter_edit_speed_en = 0;
  
  // Доступ к изменению режимов
  if(button == enter_edit_mode[enter_edit_mode_en].button &&
     state  == enter_edit_mode[enter_edit_mode_en].state)
  {
    enter_edit_mode_en++;
    if(enter_edit_mode_en >= sizeof(enter_edit_mode)/sizeof(struct sm_enter_edit_s))
    {
      enter_edit_mode_en = 0;
      timer_reset_back_buttons = timer_create(TIMER_ONE_SHOT_START,TIMER_SECOND(10),timer_reset_back_buttons_callback);
      buttons_callback(button_select_callback,button_select_callback);
    }
  } else enter_edit_mode_en = 0;
}

void shdsl_timer_callback(timer_t _timer){
  {
    if(modem_connect_txrx>3){
      static uint8_t shift_led = 1;
      leds_clr(0x0F);
      leds_set((shift_led)&(0x0F));
      if(shift_led&0x08) shift_led = 1;
      else shift_led <<=1;
      if(shift_led == 0) shift_led = 1;
    } else if(main_stat.dsl_link == 0) {
      static uint8_t blink_led = 1;
      if(blink_led){
        for(int i=0;i<sizeof(shdsl_rate_select)/sizeof(shdsl_rate_select[0]);i++){
          if(shdsl_rate_select[i] >= main_cfg.rate){
            leds_clr(0x0F);
            leds_set((i)&(0x0F));
            break;
          }
        }
      } else {
        leds_clr(0x0F);
      }
      blink_led = !blink_led;
    } else {
      for(int i=0;i<sizeof(shdsl_rate_select)/sizeof(shdsl_rate_select[0]);i++){
        if(shdsl_rate_select[i] >= main_cfg.rate){
          leds_clr(0x0F);
          leds_set((i)&(0x0F));
          break;
        }
      }
    }
  }
  {
    static int shdsl_blink = 0x00;
    if( shdsl_enabled )
    {
      if( main_cfg.master )
      {
          leds_set(LED_PWR);
      }
      else
      {
        static int shdsl_timer_pwr_blink = 0;
        if(shdsl_timer_pwr_blink>3)
        {
          shdsl_timer_pwr_blink = 0;
          if(shdsl_blink)
          {
            shdsl_blink=0;
            leds_set(LED_PWR);
          }
          else
          {
            shdsl_blink=1;
            leds_clr(LED_PWR);
          }
        }
        shdsl_timer_pwr_blink++;
      }
    }
    else
    {
      leds_clr(LED_PWR);
    }
  }
}

err_hw_t shdsl_hw_init()
{
  /**
   * RCC init
   */
  RCC->AHB1ENR  |= RCC_AHB1ENR_GPIOCEN;
  /**
   * GPIO init
   *
   *  vkl_shdsl- GPIOC8
   */
  GPIOC->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR8;
  GPIOC->MODER   |= GPIO_MODER_MODER8_0;
  
  GPIOC->ODR &= ~GPIO_ODR_ODR_8;
  buttons_callback(button_callback,button_callback);
  //timer_create();
  //leds_system_timer = 
  timer_create(TIMER_REPEAT_START, TIMER_MILLISECOND(100), shdsl_timer_callback);
  return HW_SUCCESS;
}