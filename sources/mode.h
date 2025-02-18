//******************************************************************************
// Имя файла    :       mode.h
// Заголовок    :       
// Автор        :       Вахрушев Д.В.
// Дата         :       15.02.2016
//
//------------------------------------------------------------------------------
//      
//      
//******************************************************************************
#ifndef __MODE_H_
#define __MODE_H_

#include "usart.h"
#include "dma.h"

#include <stdint.h>
#include <string.h>
#include "ip.h"

#include "lwip/sockets.h"
#include "FreeRTOS.h"
#include "task.h"

#include "settings.h"
#if (IIP != 0)
  #include "power_src.h"
#endif

#define MODE_OVER_TCP_CRC_CONTROL ( 0 ) //Контроль CRC в режиме Modbus over TCP

#define MODE_THREAD_PRIO ( tskIDLE_PRIORITY + 2 )

/* Работа RS485 пин разрешения передачи в 1 и в 0 запрет передачи */
#define DE_EN(st,en) {if(en && st->gpio_re)\
                     {st->gpio_re->BSRRL |= (1<<st->pin_re);}\
                     else\
                     {st->gpio_re->BSRRH |= (1<<st->pin_re);};}

//Пока Mikrotik только в УТД-М
#if (UTD_M != 0)
  #define MODE_USE_MIKROTIK //Используется Mikrotik (1 минута на старт)
#endif

//Определения Modbus TCP сервера
#define MB_SERVER_REG_START ( 0 ) //Адрес первого регистра
#define MB_SERVER_BASE_REG_NUM ( 2 ) //Количество регистров в базовой части

#if (IIP == 0)
  #define MODE_TCP_KEEPALIVE  //Механизм Keepalive в протоколе TCP
  #define MODE_DEBUG  //Использование отладочных блоков для Keepalive
#endif

enum mode_state_e {
  MODE_STATE_NONE = 0,
  MODE_STATE_LISTEN,
  MODE_STATE_CONNECTING,
  MODE_STATE_CONNECTED,
  MODE_STATE_RESTART,     //Рестарт сокета(ов) в соединении
  MODE_STATE_CLOSING,
  MODE_STATE_CLOSED,
};

//Последний TCP-заголовок
#define MODBUS_TCP_BUFFER_MAX   0x100
#define MODBUS_TCP_MAX_PDU_SIZE MODBUS_TCP_BUFFER_MAX
struct modbus_tcp_udp_header_s {
  uint16_t tid;
  uint16_t proto;
  uint16_t len;
  uint8_t  uid;
  uint8_t buffer[MODBUS_TCP_BUFFER_MAX];
};

// Регистры сервера Modbus TCP
enum eMbError { //Стандартные ошибки Modbus
  meNone      = 0x00, //- Нет ошибки
  meBadFunc   = 0x01, //- Принятый код функции не может быть обработан.
  meBadAddr   = 0x02, //- Адрес данных, указанный в запросе, недоступен.
  meBadData   = 0x03, //- Значение, содержащееся в поле данных запроса, является недопустимой величиной.
  meNoRecover = 0x04, //- Невосстанавливаемая ошибка имела место, пока ведомое устройство пыталось выполнить затребованное действие.
  meTimeNeed  = 0x05, //- Ведомое устройство приняло запрос и обрабатывает его, но это требует много времени.
  meBusy      = 0x06, //- Ведомое устройство занято обработкой команды.
  meCannot    = 0x07, //- Ведомое устройство не может выполнить программную функцию, заданную в запросе.
                      //  Этот код возвращается для неуспешного программного запроса, использующего функции с номерами 13 или 14.
  meMemCrcErr = 0x08, //- Ведомое устройство при чтении расширенной памяти обнаружило ошибку контроля четности.
};

#define GW_STATE_SERVER_ADDR  (0xff)
enum eMbServerError { //Типы ошибок Modbus, обнаруживаемые сервером
  mseNoError  = 0x00, //Ошибки нет
  mseCrc      = 0x11, //Ошибка CRC
  mseLength   = 0x12, //Ошибка размера кадра
  mseExtFunc  = 0x13, //Необрабатываемая функция
};

#pragma pack(1)
#if (IIP != 0)

  #define CHANNEL_SERVER_SIZE (sizeof(struct channel_server_s) / 2)
  #define MB_SERVER_REG_NUM (MB_SERVER_BASE_REG_NUM + 2 + (CHANNEL_SERVER_SIZE * POWER_CHANNEL_NUM))

struct channel_server_s {
  union {
    struct {
      //0 - Регистр состояния
      uint16_t  Out_K       : 1;  //Состояние выходного реле
      uint16_t  Channel_I   : 1;  //Наличие тока потребления
      uint16_t  Error       : 4;  //Ошибка
      uint16_t  Reserv      :10;  //Резерв
      //1, 2, 3 Значения
      uint16_t  Voltage        ;  //Напряжение с точностью 0,1В
      uint16_t  Current        ;  //Ток с точностью 0,01А
      uint16_t  VReserv        ;  //Резерв
    };
    uint16_t  Data[4];
  };
};
#else
  #define MB_SERVER_REG_NUM ( MB_SERVER_BASE_REG_NUM ) //Количество регистров
#endif

struct modbus_server_s {
  union {
    struct {
      //0 = SR1 Регистр состояния UART
      uint16_t  UartPower   : 1;  //Подключение питания UART
      uint16_t  UartTx      : 1;  //Передача UART
      uint16_t  UartRx      : 1;  //Прием UART
      uint16_t  StateReserv :13;  //Резерв
      //1 = ER1 Регистр ошибок UART
      uint16_t  UartError   : 8;  //Ошибка связи
      uint16_t  UartErrValue: 4;  //Значение по ошибке связи
      uint16_t  DiagReserv  : 4;  //Резерв
#if (IIP != 0)
      //2 = SR_I1 Регистр состояния источника
      uint16_t  In_K        : 1;  //Состояние входного реле
      uint16_t  Channel_K   : 4;  //Состояние выходного реле каналов 1-4
      uint16_t  Channel_I   : 4;  //Наличие потребления в канале (ток > 0)
      uint16_t  IP_Error    : 4;  //Ошибка источника
      uint16_t  IP_Reserv3  : 3;  //Резерв
      //3 = Резерв
      uint16_t  IP_Reserv      ;  //Резерв
      //Состояния каналов
      //4 .. 19 (4 х 4)
      struct channel_server_s aChannel[POWER_CHANNEL_NUM];
#endif
    };
    uint16_t  Data[MB_SERVER_REG_NUM];
  };
};
#pragma pack()

#ifdef MODE_DEBUG  //Использование отладочных блоков
// + Отладочные
struct s_ModeDebug {
  uint32_t  RecvCount, TryRecvCount, NoRecvCount, RecvErrCount,
            TryConnectCount,
            CloseCount, ClosedCount, RestartCount, 
            Recv1ByteCount, Send1ByteCount;
  uint8_t   Err;
  uint32_t  BindCount, AcceptCount, AcceptRestartCount, ConnectedRecvCount;
  
};
// - Отладочные
#endif

struct mode_struct_s {
  USART_TypeDef         *usart;
  GPIO_TypeDef          *gpio_re, *gpio_led;
  int                   pin_re, pin_led;
  GPIO_TypeDef          *gpio_tx, *gpio_rx;
  int                   pin_tx, pin_rx;
  enum ip_port_mode_e   mode;
  enum mode_state_e     state;
  int                   dma_send,
                        dma_recv;
  int                   csock,
                        ssock;
  uint16_t              cport,
                        sport;
  uint16_t              timeout;
  volatile int          gTimeout;
  int32_t               autoclose;
  int32_t               counter;
  int                   idle;
  ip_addr_t             ip;
#define MODE_STRUCT_MAX_MSG_SIZE  0x120
  uint8_t               tbuffer[MODE_STRUCT_MAX_MSG_SIZE];
  uint8_t               rbuffer[MODE_STRUCT_MAX_MSG_SIZE];
  uint8_t               rs485;
  int32_t               tblen;
  int32_t               rblen;
  struct modbus_tcp_udp_header_s  last_modbus_tcp_udp_header;
  struct modbus_server_s  modbus_server_state;
#ifdef MODE_DEBUG  //Использование отладочных блоков
  struct s_ModeDebug    ModeDebug;
#endif
  uint32_t              count; //Счетчик для контроля наличия кадров передачи
};

#define MODE_TX_COUNTER_VALUE (2000)

#pragma pack(1)

//Поллинг RS485-MODBUS +
enum eMBResult {
  mbrNone,    //Пустой (нет результата)
  mbrRxGood,  //Успешный прием
  mbrTimeout, //Таймаут (нет кадра)
  mbrCRC,     //Ошибка CRC
  mbrModbus,  //Ошибка MODBUS
  mbrCount    //Количество в перечислении
};
  
extern char aErrorMsg[][30];

typedef struct sMBParam { //Параметры поллинга MODBUS
  uint8_t   Chan;     //Канал RS485
  uint8_t   Addr;     //Адрес
  uint8_t   Func;     //Функция
  uint16_t  Start;    //Начальный регистр
  uint16_t  Number;   //Количество регистров
  uint16_t  Period;   //Период опроса
  int16_t   Error;    //Ошибка
  uint16_t  rxCount;  //Принятых байт
  uint8_t   rBuffer[MODE_STRUCT_MAX_MSG_SIZE];
} SMBParam;
//Поллинг RS485-MODBUS -

int mode_init(struct mode_struct_s*);
extern void init_uart_port(struct mode_struct_s * ipMode, struct uart_port_s * ipPort);

#pragma pack()

#endif //__MODE_H_


