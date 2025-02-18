//******************************************************************************
// Имя файла    :       mode.c
// Заголовок    :       
// Автор        :       Вахрушев Д.В.
// Дата         :       15.02.2016
//
//------------------------------------------------------------------------------
/**
Архитектура модуля следующая, в зависимости от режима запускаются две задачи
с разными названиями для идентификации при отладке.
Одна задача отвечает за прием данных из блокирующего сокета.
Другая задача отвечает за передачу данных в сокет, за прием данных от UART
В коде происходит самостоятельное управление CTS/RTS (DE), для нужд RS485 
приемо-передатчика.

Работа с сокетами потоко-безопасна, это основная причина использования сокетов.
Изначально проектировалось под RAW API 
(непотокобезопасно, есть моменты мешающие моментальной отправке данных)

Использование таймера для точного отсчета времени прекращения потока данных от 
USART.

**/
//   
//      
//******************************************************************************

#include "mode.h"
#include "usart.h"
#include "mbcrc.h"
#include <stdint.h>
#include "ip.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "lwip/sys.h"
#include "string.h"
#include "FreeRTOS.h"
#include "task.h"

#include "extio.h"

#define DEFAULT_AUTO_CLOSE      1

static int dma_tc = 0;
static int dma_ht = 0;
static int dma_err = 0;
void dma_callback( DMA_TypeDef          *dma, 
                   DMA_Stream_TypeDef   *stream, 
                   enum dma_event_e     event,
                   void *arg )
{
  switch(event)
  {
    case DMA_TRANSFER_COMPLETE:
      {
        dma_tc++;
      }
      break;
    case DMA_TRANSFER_HALF:
      {
        dma_ht++;
      }
      break;
    case DMA_TRANSFER_ERROR:
      {
        dma_err++;
      }
      break;
  }
}

void usart_callback(USART_TypeDef *usart,enum usart_event_e event, void *arg)
{
  struct mode_struct_s *st = (struct mode_struct_s *)arg;
  switch(event)
  {
    case USART_EVENT_TC:
      {
        DE_EN(st,0);
        if (!st->idle)
          usart_set_rx_enable(st->usart, 1);
      }
      break;
    case USART_EVENT_IDLE:
      {
        if(st->timeout)
        {
          st->counter=0;
        }
        else
        {
          st->tblen = MODE_STRUCT_MAX_MSG_SIZE - dma_counter(st->dma_recv);
          dma_transfer_stop(st->dma_recv);
        }
      }
      break;
  }
}


static int mst_i = 0;
static struct mode_struct_s *mst[2];
void TIM7_IRQHandler()
{
  if(TIM7->SR & TIM_SR_UIF)
  {
    for(int i=0;i<mst_i;i++)
    {
      if(mst[i]->counter>=0)
      {
        mst[i]->counter++;
        if(mst[i]->timeout<=mst[i]->counter)
        {
          mst[i]->tblen = MODE_STRUCT_MAX_MSG_SIZE - dma_counter(mst[i]->dma_recv);
          dma_transfer_stop(mst[i]->dma_recv);
          mst[i]->counter = -1;
        }
      }
    }
    TIM7->SR = 0;
  }
}

static void mode_timer_init(struct mode_struct_s *st)
{
  if(mst_i>=2) return;
  st->counter = -1;
  mst[mst_i++] = st;
  if( (RCC->APB1ENR & RCC_APB1ENR_TIM7EN) ||
      (TIM7->CR1 & TIM_CR1_CEN) )
  {
    return;
  }
  RCC->APB1ENR |= RCC_APB1ENR_TIM7EN;
  TIM7->PSC = 84-1;
  TIM7->ARR = 1000;
  
  TIM7->CR1 = 0;
  TIM7->CR1 |= TIM_CR1_ARPE;
  TIM7->CR1 = TIM_CR1_ARPE;
  
  TIM7->DIER = TIM_DIER_UIE;
  
    
  NVIC->IP[((uint32_t)(int32_t)TIM7_IRQn)]
    = (uint8_t)(((1<<__NVIC_PRIO_BITS) - 1 << (8U - __NVIC_PRIO_BITS)) & (uint32_t)0xFFUL);
  NVIC->ISER[(((uint32_t)(int32_t)TIM7_IRQn) >> 5UL)]
    = (uint32_t)(1UL << (((uint32_t)(int32_t)TIM7_IRQn) & 0x1FUL));
  
  
  TIM7->CR1 |= TIM_CR1_CEN;
}

void mode_timer_deinit() {
  NVIC->ICER[(((uint32_t)(int32_t)TIM7_IRQn) >> 5UL)]
    = (uint32_t)(1UL << (((uint32_t)(int32_t)TIM7_IRQn) & 0x1FUL));
}

static int mode_repack_tcp_rtu(struct mode_struct_s *st);

static void mode_led_uart_blink(struct mode_struct_s  *st)
{
#if ((UTD_M != 0) || (IIP != 0))
  if(st->usart == USART1)
  {
    extio_led(EXTIO_RS485_1,EXTIO_LED_BLINK_ONCE_ON); // подмигнем
  }
  if(st->usart == USART6)
  {
    extio_led(EXTIO_RS485_2,EXTIO_LED_BLINK_ONCE_ON); // подмигнем
  }
#else
  if(st->usart == USART1)
  {
 #if (MKPSH10 != 0)
    extio_led(EXTIO_RS485_2,EXTIO_LED_BLINK_ONCE_ON); // подмигнем
 #endif
 #if (IMC_FTX_MC != 0)
    extio_led(EXTIO_RS485_1,EXTIO_LED_BLINK_ONCE_ON); // подмигнем
 #endif
  }
  if(st->usart == UART5)
  {
    extio_led(EXTIO_RS485_1,EXTIO_LED_BLINK_ONCE_ON); // подмигнем
  }
#endif
}

enum nc_state {
  NC_NONE,
  NC_WRITE,
  NC_LISTEN,
  NC_CONNECT,
  NC_CLOSE
};


extern int GetSocketState(int s);
extern err_t GetSocketError(int s);
extern int GetSocketConnError(int s);
extern int GetSocketTcpState(int s);
extern bool gApplying;

void InitServerRegisters(struct mode_struct_s  * st) {
  //Заполнить регистры Modbus TCP сервера 
  st->modbus_server_state.StateReserv   = 0;
  st->modbus_server_state.DiagReserv    = 0;
  st->modbus_server_state.UartError     = 0;
  st->modbus_server_state.UartErrValue  = 0;
  st->modbus_server_state.UartPower     = 0;
  st->modbus_server_state.UartTx        = 0;
  st->modbus_server_state.UartRx        = 0;
  
}

extern struct mode_struct_s * GetMode(int iModeIdx);
#if (IIP != 0)
  extern void Fill_IP_MBServer(struct modbus_server_s * ipMBServer);
#endif
void FillServerRegisters(struct mode_struct_s  * st) {
  //Заполнить регистры Modbus TCP сервера 
  st->modbus_server_state.StateReserv   = 0;
  st->modbus_server_state.DiagReserv    = 0;
//  st->modbus_server_state.UartError     = 0; //Заполняются при приеме с UART
//  st->modbus_server_state.UartErrValue  = 0;
#if ((MKPSH10 != 0) || (IIP != 0))
  if (st == GetMode(0))
    st->modbus_server_state.UartPower = ((extio_read_idx(EXTIO_CPU_33_V5)==0) ? 1:0);
  if (st == GetMode(1))
    st->modbus_server_state.UartPower = ((extio_read_idx(EXTIO_CPU_33_V4)==0) ? 1:0);
#endif
#if (IMC_FTX_MC != 0)
  st->modbus_server_state.UartPower = ((extio_read_idx(EXTIO_CPU_33_V5)==0) ? 1:0);
#endif
  // !!! Неверно вычисляются значения
//  st->modbus_server_state.UartTx    = ((st->state == MODE_STATE_CONNECTED) ? 1:0);
//  st->modbus_server_state.UartRx    = ((extio_led_blinked(EXTIO_RS485_1)>0) ? 1:0);
#if (IIP != 0)
  if (st == GetMode(0))
    Fill_IP_MBServer(&st->modbus_server_state);
#endif
  
}

static uint16_t modbus_gw_crc16(char* data, int32_t len);

void CreateMBServerAnswer(struct mode_struct_s  * st) {
  FillServerRegisters(st);
  int8_t cRegNum = MB_SERVER_REG_NUM;
#if (IIP != 0)
  if (st != GetMode(0)) {
    cRegNum = MB_SERVER_BASE_REG_NUM;
  }
#endif
  uint8_t cErr = 0, cSize;
  st->tbuffer[0] = st->rbuffer[0];
  st->tbuffer[1] = st->rbuffer[1];
  uint16_t * cpReg = (uint16_t *)&st->rbuffer[2];
  uint16_t cAddr = PP_HTONS(*cpReg);
  uint16_t cRegs;
  if (st->rbuffer[1] == 0x03) { //
    //Сформировать ответ
    if (cAddr < cRegNum) {
      cpReg = (uint16_t *)&st->rbuffer[4];
      cRegs = PP_HTONS(*cpReg);
      if ((cAddr + cRegs) <= cRegNum) {
        uint8_t * cpTbuffer = &st->tbuffer[3];
        for (int i = 0; i < cRegs; ++i) {
          *cpTbuffer++ = (st->modbus_server_state.Data[i] >> 8) & 0x0ff;
          *cpTbuffer++ = st->modbus_server_state.Data[i] & 0x0ff;
        }
      } else { //Ошибка, не поддерживаемый адрес регистра
        cErr = 0x02;
      }
    } else { //Ошибка, не поддерживаемый адрес регистра
      cErr = 0x02;
    }
  } else { //Ошибка, не поддерживаемый код функции
    cErr = 0x01;
  }
  if (cErr == 0) {
    st->tbuffer[2] = cRegs << 1;
    cSize = 3 + st->tbuffer[2];
    //Регистры: сформировать
  } else {
    st->tbuffer[1] |= 0x80;
    st->tbuffer[2] = cErr;
    cSize = 3;
  }
  // + CRC16
  uint16_t cCRC = modbus_gw_crc16((char *)&st->tbuffer[0], cSize);
  st->tbuffer[cSize] = (cCRC >> 8) & 0x0ff;
  st->tbuffer[cSize + 1] = (cCRC) & 0x0ff;
  st->tblen = cSize + 2;
}

#include "semphr.h"
xSemaphoreHandle mode_timeout_mutex;

void OpenCSocket(struct mode_struct_s * st) {
  //Открыть новый сокет
  st->csock = socket(PF_INET, SOCK_STREAM, 0);
  st->state = MODE_STATE_CONNECTING;
  fcntl(st->csock, F_SETFL, O_NONBLOCK); //Неблокирующий сокет!
  int cOptValue = 1;
  setsockopt(st->csock, SOL_SOCKET, SO_REUSEADDR, &cOptValue, sizeof(cOptValue));
#ifdef MODE_TCP_KEEPALIVE  //Механизм Keepalive в протоколе TCP
  //Keepalive: Таймаут = 2сек; Интервал посылок = 1сек; Количество посылок = 2
  setsockopt(st->csock, SOL_SOCKET, SO_KEEPALIVE, &cOptValue, sizeof(cOptValue));
  cOptValue = 2;
  setsockopt(st->csock, IPPROTO_TCP, TCP_KEEPIDLE, &cOptValue, sizeof(cOptValue));
  setsockopt(st->csock, IPPROTO_TCP, TCP_KEEPCNT, &cOptValue, sizeof(cOptValue));
  cOptValue = 1;
  setsockopt(st->csock, IPPROTO_TCP, TCP_KEEPINTVL, &cOptValue, sizeof(cOptValue));
#endif
}

void CloseCSocket(struct mode_struct_s * st) {
  st->state = MODE_STATE_CLOSING;
  struct linger cLinger;
  cLinger.l_onoff = 1;
  cLinger.l_linger = 0;
  setsockopt(st->csock, SOL_SOCKET, SO_LINGER, &cLinger, sizeof(cLinger));
  close(st->csock);
  int cCount = 200;
#ifdef MODE_DEBUG  //Использование отладочных блоков
  while ((st->ModeDebug.Err = GetSocketTcpState(st->csock)) != CLOSED) {
#else
  while ((GetSocketTcpState(st->csock)) != CLOSED) {
#endif
    vTaskDelay(10);
    if (--cCount == 0)
      break;
  }
  st->state = MODE_STATE_CLOSED;
  st->csock = -1;
}

void CloseSSocket(struct mode_struct_s * st) {
  st->state = MODE_STATE_CLOSING;
  close(st->ssock);
  int cCount = 200;
#ifdef MODE_DEBUG  //Использование отладочных блоков
  while ((st->ModeDebug.Err = GetSocketTcpState(st->csock)) != CLOSED) {
#else
  while ((GetSocketTcpState(st->csock)) != CLOSED) {
#endif
    vTaskDelay(10);
    if (--cCount == 0)
      break;
  }
  st->state = MODE_STATE_CLOSED;
  st->ssock = -1;
}

int send_ctrl_frame(struct mode_struct_s *st) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
  ++st->ModeDebug.Send1ByteCount; //Передать контрольный кадр
#endif
  st->tbuffer[0] = 0xff;
  st->tblen = 1;
  int cLength = send(st->csock, st->tbuffer, st->tblen, 0);
  st->tblen = 0;
  return cLength;
}

void init_socket_address(struct sockaddr_in * ipSa,
                         uint16_t iPort, in_addr_t iAddr) {
  memset(ipSa, 0, sizeof(struct sockaddr_in)); // Подчищаем все в структуре
  ipSa->sin_family = AF_INET;
  ipSa->sin_port = htons(iPort); // htons(5000);//Порт прослушки
  ipSa->sin_addr.s_addr = iAddr; // Адрес прослушки любой
  ipSa->sin_len = sizeof(struct sockaddr_in);
}

static void mode_socket_recv_thread( void * arg )
{
  struct mode_struct_s  *st = (struct mode_struct_s *)arg;
  struct sockaddr_in    sa,ca,isa;
#ifdef MODE_DEBUG  //Использование отладочных блоков
  memset((void *)&st->ModeDebug, 0, sizeof(st->ModeDebug));
#endif
  int cConnectState = -1;
  u32_t addr_size;
  st->csock = -1;
  st->ssock = -1;
  if(st->gpio_re)
  {
    st->gpio_re->MODER &=~(3<<(st->pin_re<<1));
    st->gpio_re->MODER |= (1<<(st->pin_re<<1));
  }
  if(st->gpio_led)
  {
    st->gpio_led->MODER &=~(3<<(st->pin_led<<1));
    st->gpio_led->MODER |= (1<<(st->pin_led<<1));
  }
  DE_EN(st,0);
  usart_event(st->usart,USART_EVENT_TC  ,usart_callback);
  
  /**
   * Регистрируем usart
   */
  usart_event_arg(st->usart ,st);
  dma_event_arg(st->dma_recv,st);

  usart_set_dma_tx_enable(st->usart, 1);
  usart_set_dma_rx_enable(st->usart, 1);
  
  if(st->timeout)
  {
    mode_timer_init(st);
  }
  if(st->idle)
  {
    usart_event(st->usart,USART_EVENT_IDLE,usart_callback);
  } else {
    usart_event(st->usart,USART_EVENT_RXNE,usart_callback);
  }
  //dma_event(st->dma_recv,DMA_TRANSFER_COMPLETE,dma_callback);
  dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
//  dma_event(st->dma_send,DMA_TRANSFER_COMPLETE,dma_callback
  dma_event(st->dma_send,DMA_TRANSFER_COMPLETE,dma_callback);
  
  init_socket_address(&sa, st->cport, (st->ip.addr == IPADDR_BROADCAST) ? INADDR_ANY : st->ip.addr);
  init_socket_address(&ca, st->cport, st->ip.addr);
#ifdef MODE_USE_MIKROTIK //Используется Mikrotik (1 минута на старт)
  vTaskDelay(55000);
#endif

  switch(st->mode)
  {
    case SETTINGS_IP_PORT_MODBUS_GW:
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
    case SETTINGS_IP_PORT_TCP_SERVER:
      {
        st->ssock = socket(PF_INET, SOCK_STREAM, 0); // Создаем сокет
        init_socket_address(&sa, st->cport, (st->ip.addr == IPADDR_BROADCAST) ? INADDR_ANY : st->ip.addr);
        init_socket_address(&ca, st->cport, st->ip.addr);
        
        if (st->ssock >= 0) { //Сокет создан
          fcntl(st->ssock, F_SETFL, O_NONBLOCK); //Неблокирующий сокет!
          /**
           * Включаем прослушку для сервера
           */
          if (bind(st->ssock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
          {
            CloseCSocket(st);
            while(1); 
            // Что то не то произошло, сокет не забинден
            // При старте не должно быть проблем с bind, вероятность появления -> 0
            // поэтому сюда заваливаемся и не отвечаем
          }
          st->state = MODE_STATE_CONNECTING;
          /**
           * Слушаем сокет на подключения
           */ 
          listen(st->ssock,1);
        }
      }
      break;
    case SETTINGS_IP_PORT_MODBUS_GW_UDP:
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
      {
        st->csock = socket(PF_INET, SOCK_DGRAM, 0);
        init_socket_address(&sa, st->cport, (st->ip.addr == IPADDR_BROADCAST) ? INADDR_ANY : st->ip.addr);
        init_socket_address(&ca, st->cport, st->ip.addr);

        if (bind(st->csock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
        {
          CloseCSocket(st);
          while(1);
          // Что то не то произошло, соект не забинден
          // При старте не должно быть проблем с bind, вероятность появления -> 0
          // поэтому сюда заваливаемся и не отвечаем
        }
        st->state = MODE_STATE_CONNECTING;
        /**
         * Если необходимо прикрепить себе порт
         */
        if(0) 
        if(connect(st->csock,(struct sockaddr *)&ca,sizeof(struct sockaddr_in)) < 0)
        {
          CloseCSocket(st);
          while(1);
        }
      }
      break;
    case SETTINGS_IP_PORT_TCP_CLIENT:
      {
        OpenCSocket(st);
      }
      break;
    case SETTINGS_IP_PORT_UDP:
      {
        st->csock = socket(PF_INET, SOCK_DGRAM, 0);

        if (bind(st->csock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
        {
          CloseCSocket(st);
          while(1);
        }
        st->state = MODE_STATE_CONNECTING;
        /**
         * Если необходимо прикрепить себе порт
         */
        if(0) 
        if(connect(st->csock,(struct sockaddr *)&ca,sizeof(struct sockaddr_in)) < 0)
        {
          CloseCSocket(st);
          while(1);
        }
      }
      break;
    case SETTINGS_IP_PORT_SIGRAND:
      {
      }
      break;
    default:
      {
        while(1);
      }
      break;
  };
  
  switch(st->mode)
  {
    case SETTINGS_IP_PORT_MODBUS_GW:
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
    case SETTINGS_IP_PORT_TCP_SERVER:
      if( st->ssock < 0 )
      { //Не создан сокет
        while(1) {
          vTaskDelay(10);
        }
      }
      break;
    default:
      if( st->csock < 0 )
      { //Не создан сокет
        while(1)
          vTaskDelay(10);
      }
      break;
  }
  while(1)
  {
    switch(st->mode)
    {
      case SETTINGS_IP_PORT_TCP_SERVER:
        {
          /**
           * Ждем подтверждения соединения
           */
          addr_size     = sizeof(isa);
          st->state = MODE_STATE_LISTEN;
          st->csock = accept(st->ssock, (struct sockaddr*)&isa,&addr_size);
          st->state = MODE_STATE_CONNECTED;
          vTaskDelay(100);
          DE_EN(st,0);
          
          dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
          
          /**
           * Подсоединен
           */ 
          while(st->state == MODE_STATE_CONNECTED)
          {
            if((st->rblen = recv(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE,0)) <= 0)
            {
              CloseCSocket(st);
              st->state = MODE_STATE_LISTEN;
              dma_transfer_stop(st->dma_send);
              dma_transfer_stop(st->dma_recv);
              DE_EN(st,0);
              break;
            }
            DE_EN(st,1);
            dma_transfer(st->dma_send, st->rbuffer, st->rblen);
            dma_transfer_wait(st->dma_send);
          }
        }
        break;
      case SETTINGS_IP_PORT_MODBUS_GW:
      case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
        {
          if (st->state != MODE_STATE_CONNECTING) {
            if (st->state == MODE_STATE_RESTART) { //Рестарт сокета
              //Закрыть сокет
              st->state = MODE_STATE_CLOSING;
              if (st->csock >= 0) {
                CloseCSocket(st);
                dma_transfer_stop(st->dma_send);
                dma_transfer_stop(st->dma_recv);
                DE_EN(st, 0);
                st->csock = -1;
              }
              
              init_socket_address(&sa, st->cport, (st->ip.addr == IPADDR_BROADCAST) ? INADDR_ANY : st->ip.addr);
              init_socket_address(&ca, st->cport, st->ip.addr);

              if (st->ssock >= 0) {
                CloseSSocket(st);
                st->ssock = socket(PF_INET, SOCK_STREAM, 0); // Создаем сокет
                if (st->ssock >= 0) { //Сокет создан
                  fcntl(st->ssock, F_SETFL, O_NONBLOCK); //Неблокирующий сокет!
                  /**
                   * Включаем прослушку для сервера
                   */
                  while (st->state != MODE_STATE_CONNECTING) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                    ++st->ModeDebug.BindCount;
#endif
                    int cOptValue = 1;
                    setsockopt(st->ssock, SOL_SOCKET, SO_REUSEADDR, &cOptValue, sizeof(cOptValue));
                    if (bind(st->ssock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
                    {
                      vTaskDelay(1000);
                      st->state = MODE_STATE_RESTART;
                      continue;
                    } else {
                      st->state = MODE_STATE_CONNECTING;
                    }
                  }
                  st->state = MODE_STATE_CONNECTING;
                  /**
                   * Слушаем сокет на подключения
                   */ 
                  listen(st->ssock,1);
                } else { //Не создан сокет listen
                  while(1) {
                    vTaskDelay(10);
                  }
                }
              }
              vTaskDelay(5);
              st->state = MODE_STATE_LISTEN;
            } else {
              if (st->csock >= 0) {
                CloseCSocket(st);
                st->state = MODE_STATE_LISTEN;
              }
            }
          }
          /**
           * Ждем подтверждения соединения
           */
          st->state = MODE_STATE_LISTEN;
          addr_size     = sizeof(sa);
          int cCnt = 0;
          int cConnState = GetSocketState(st->ssock);
#ifdef MODE_DEBUG  //Использование отладочных блоков
          ++st->ModeDebug.AcceptCount;
#endif
          while ((st->csock = accept(st->ssock, (struct sockaddr*)&sa, &addr_size)) < 0) {
            cConnState = GetSocketState(st->ssock);
            if (cConnState != (int)NC_LISTEN) {
              st->state = MODE_STATE_RESTART;
            }
            if (st->state == MODE_STATE_RESTART) { //Рестарт сокета
#ifdef MODE_DEBUG  //Использование отладочных блоков
              ++st->ModeDebug.AcceptRestartCount;
#endif
              break;
            }
            vTaskDelay(100);
            cCnt += 100;
            if (cCnt >= 5000) {
              st->state = MODE_STATE_RESTART;
#ifdef MODE_DEBUG  //Использование отладочных блоков
              ++st->ModeDebug.AcceptRestartCount;
#endif
              break;
            }
          }
          if (st->state == MODE_STATE_RESTART) { //Рестарт сокета
            continue;
          }

          st->state = MODE_STATE_CONNECTED;
          DE_EN(st,0);
          
          dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
          
          int cTimeout;
          st->gTimeout = 5000;
          /**
           * Подсоединен, поймали соединение
           */ 
          while ( st->state == MODE_STATE_CONNECTED )
          {
#ifdef MODE_DEBUG  //Использование отладочных блоков
            ++st->ModeDebug.ConnectedRecvCount;
#endif
            if (xSemaphoreTake(mode_timeout_mutex, 200)) {
              cTimeout = st->gTimeout;
              xSemaphoreGive(mode_timeout_mutex);
            }
            if (cTimeout <= 0)
            {
              /**
               * Видимо соедиение закрылось, закрываем сокет, начинаем заново
               */
              CloseCSocket(st);
              st->state = MODE_STATE_LISTEN;
              DE_EN(st, 0);
              break;
            }
            /**
             * Ожидаем посылкки по TCP порту
             */
            if((st->rblen = recv(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE, MSG_DONTWAIT)) <= 0)
            {
              if (st->csock >= 0) { //Нормальный сокет TCP-соединения (state = NONE !!!)
                enum tcp_state cConnTcpState = (enum tcp_state)GetSocketTcpState(st->ssock);
                switch (cConnTcpState) {
                case FIN_WAIT_1:
                case FIN_WAIT_2:
                case CLOSE_WAIT:
                  st->state = MODE_STATE_RESTART;
                  continue;
                default:
                  break;
                }
              }
              vTaskDelay(5);
              if (xSemaphoreTake(mode_timeout_mutex, 200)) {
                st->gTimeout -= 5;
                cTimeout = st->gTimeout;
                xSemaphoreGive(mode_timeout_mutex);
              }
              if (cTimeout <= 0)
              {
                /**
                 * Видимо соедиение закрылось, закрываем сокет, начинаем заново
                 */
                CloseCSocket(st);
                st->state = MODE_STATE_LISTEN;
                DE_EN(st, 0);
                break;
              }
              continue;
            } else { //Принят кадр
              if (xSemaphoreTake(mode_timeout_mutex, 200)) {
                st->gTimeout = 5000;
                cTimeout = st->gTimeout;
                xSemaphoreGive(mode_timeout_mutex);
              }
              if (st->rblen == 1) { //Служебный кадр для поддержания соединения
#ifdef MODE_DEBUG  //Использование отладочных блоков
                ++st->ModeDebug.Recv1ByteCount;
#endif
                send_ctrl_frame(st); //Ответ на служебный кадр
                continue;
              }
            }
            /**
             * Приняли посылку по TCP
             * Парсим и формируем пакет для передачаи по RS485
             */
            if (!gApplying) { //Если не работает внутренний MODBUS Poll
              if(mode_repack_tcp_rtu(st))
              { // Если парсинг удался
                // Если адрес == 255, то это сервер Modbus TCP
#if ( MODE_OVER_TCP_CRC_CONTROL) //Контроль CRC в режиме Modbus over TCP
                if (st->rbuffer[0] == GW_STATE_SERVER_ADDR) {
                  CreateMBServerAnswer(st);
                } else
#else
                if ((st->mode == SETTINGS_IP_PORT_MODBUS_GW) &&
                    (st->rbuffer[0] == GW_STATE_SERVER_ADDR)) {
                  CreateMBServerAnswer(st);
                } else
#endif
                {
                  /* Если парсинг удался, отправляем в USART */
                  DE_EN(st, 1);
                  if (!st->idle)
                    usart_set_rx_enable(st->usart, 0);
                  /** * Отправляем, ожидаем конца передачи */ 
                  dma_transfer(st->dma_send, st->rbuffer, st->rblen);
                  dma_transfer_wait(st->dma_send);
                }
              }
            }
          }
        }
        break;
      case SETTINGS_IP_PORT_MODBUS_GW_UDP:
      case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
        {
          if (st->state == MODE_STATE_RESTART) {
            if (st->csock) {
              CloseCSocket(st);
            }
            st->csock = socket(PF_INET, SOCK_DGRAM, 0);
            if (bind(st->csock, (struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0)
            {
              CloseCSocket(st);
              break;
            }
            st->state = MODE_STATE_CONNECTING;
          } else {

          }

          st->state = MODE_STATE_CONNECTED;
          vTaskDelay(10);
          /* Ключик на прием, данных пока еше нет для передачи */
          DE_EN(st,0);
          /* Включаем DMA прием данных в буффер */
          dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
          /**
           * Подсоединен
           */ 
          st->count = 0;
          while(st->state == MODE_STATE_CONNECTED)
          {
            //st->rblen = recvfrom(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE, 0, (struct sockaddr *)&recvaddr, &socklen);
            st->rblen = recv(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE,0);
            if((st->rblen)<=0)
            {
              /* Ничего не получили, соединение закрываем */
              CloseCSocket(st);
              st->state = MODE_STATE_LISTEN;
              dma_transfer_stop(st->dma_send);
              dma_transfer_stop(st->dma_recv);
              /* Ключик на прием */
              DE_EN(st,0);
              break;
            }

            if(mode_repack_tcp_rtu(st))
            { // Если парсинг удался
              // Если адрес == 255, то это сервер Modbus TCP
#if ( MODE_OVER_TCP_CRC_CONTROL) //Контроль CRC в режиме Modbus over TCP
              if (st->rbuffer[0] == GW_STATE_SERVER_ADDR) {
                CreateMBServerAnswer(st);
              } else
#else
              if ((st->mode == SETTINGS_IP_PORT_MODBUS_GW_UDP) &&
                  (st->rbuffer[0] == GW_STATE_SERVER_ADDR)) {
                CreateMBServerAnswer(st);
              } else
#endif
              {
                /* Если парсинг удался, отправляем в USART */
                DE_EN(st, 1);
                if (!st->idle)
                  usart_set_rx_enable(st->usart, 0);
                /** * Отправляем, ожидаем конца передачи */ 
                dma_transfer(st->dma_send, st->rbuffer, st->rblen);
                dma_transfer_wait(st->dma_send);
                st->count = MODE_TX_COUNTER_VALUE + xTaskGetTickCount();
              }
            }
          }
        }
        break;
      case SETTINGS_IP_PORT_TCP_CLIENT:
        {
          if (ca.sin_addr.s_addr != st->ip.addr)
            ca.sin_addr.s_addr = st->ip.addr;
          if (!IsTrueAddr(ca.sin_addr.s_addr)) {
            vTaskDelay(3000);
            continue;
          }
//          init_socket_address(&sa, st->cport, (st->ip.addr == IPADDR_BROADCAST) ? INADDR_ANY : st->ip.addr);
          init_socket_address(&ca, st->sport, st->ip.addr);
          if (st->state == MODE_STATE_RESTART) { //Рестарт сокета
            //Закрыть сокет
#ifdef MODE_DEBUG  //Использование отладочных блоков
            ++st->ModeDebug.RestartCount;
#endif
            CloseCSocket(st);
            dma_transfer_stop(st->dma_send);
            dma_transfer_stop(st->dma_recv);
            DE_EN(st, 0);
            
            vTaskDelay(1000);
            //Открыть новый сокет
            OpenCSocket(st);
          }
          //Соединение сокета
          cConnectState = -1;
          int cTimeout = 5000;
          err_t err;
          while (cConnectState < 0) {
            if (ca.sin_addr.s_addr != st->ip.addr)
              ca.sin_addr.s_addr = st->ip.addr;
#ifdef MODE_DEBUG  //Использование отладочных блоков
            ++st->ModeDebug.TryConnectCount;
#endif
            cConnectState = connect(st->csock,(struct sockaddr *)&ca,sizeof(struct sockaddr_in));
            err = GetSocketConnError(st->csock);
            if (cConnectState >= 0) {
              st->state = MODE_STATE_CONNECTED;
              break;
            }
            if (err == ERR_INPROGRESS) {
              --cTimeout;
              if (cTimeout <= 0) {
                st->state = MODE_STATE_RESTART;
                break;
              }
              vTaskDelay(10);
              continue;
            }
            if ((err == ERR_ABRT) || (err == ERR_RST)) {
              vTaskDelay(2000);
              st->state = MODE_STATE_RESTART;
              break;
            }
            if (err == ERR_ALREADY) { //В процессе соединения
              vTaskDelay(50);
              continue;
            }
            if (err == ERR_ISCONN) {
              st->state = MODE_STATE_CONNECTED;
              break;
            }
            if (st->state != MODE_STATE_CONNECTING)
              break;
            vTaskDelay(10);
          }
          if (st->state != MODE_STATE_CONNECTED)
            continue;
          vTaskDelay(100);
          DE_EN(st,0);
          dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
          /**
          * Подсоединен: чтение входных кадров, передача в RS485
           */ 
          while(st->state == MODE_STATE_CONNECTED)
          {
#ifdef MODE_DEBUG  //Использование отладочных блоков
            ++st->ModeDebug.TryRecvCount;
#endif
            if ((st->rblen = recv(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE,0)) <= 0)
            {
#ifdef MODE_DEBUG  //Использование отладочных блоков
              ++st->ModeDebug.NoRecvCount;
#endif
              vTaskDelay(5);
              err = GetSocketConnError(st->csock);
              if (err != 0) {
                if (err == ERR_INPROGRESS) {
                  vTaskDelay(1);
                  continue;
                }
#ifdef MODE_DEBUG  //Использование отладочных блоков
                ++st->ModeDebug.RecvErrCount;
                st->ModeDebug.Err = err = GetSocketTcpState(st->csock);
#else
                err = GetSocketTcpState(st->csock);
#endif
                if ((err == ESTABLISHED) && (st->rblen == -1)) {
                  err = GetSocketError(st->csock);
                  if ((err == ECONNABORTED) || (err == ENOTCONN)) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                    ++st->ModeDebug.CloseCount;
#endif
                    st->state = MODE_STATE_RESTART;
                    break;
                  }
                }
                if (err == CLOSED) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                  ++st->ModeDebug.ClosedCount;
#endif
                  st->state = MODE_STATE_RESTART;
                  break;
                }
                if (err == CLOSE_WAIT) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                  ++st->ModeDebug.CloseCount;
#endif
                  st->state = MODE_STATE_RESTART;
                  break;
                }
              } else {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                st->ModeDebug.Err = err = GetSocketTcpState(st->csock);
#else
                err = GetSocketTcpState(st->csock);
#endif
                if ((err == ESTABLISHED) && (st->rblen == -1)) {
                  err = GetSocketError(st->csock);
                  if ((err == ECONNABORTED) || (err == ENOTCONN)) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                    ++st->ModeDebug.CloseCount;
#endif
                    st->state = MODE_STATE_RESTART;
                    break;
                  }
                }
                if (err == CLOSE_WAIT) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                  ++st->ModeDebug.CloseCount;
#endif
                  st->state = MODE_STATE_RESTART;
                  break;
                }
              }
              if (err == ERR_ABRT) {
                
                break;
              }
            } else {
              if (st->rblen == 1) {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                ++st->ModeDebug.Recv1ByteCount;
#endif
              } else {
#ifdef MODE_DEBUG  //Использование отладочных блоков
                ++st->ModeDebug.RecvCount;
#endif
                DE_EN(st,1);
                dma_transfer(st->dma_send, st->rbuffer, st->rblen);
                dma_transfer_wait(st->dma_send);
                vTaskDelay(2);
              }
            }
          }
        }
        break;
      case SETTINGS_IP_PORT_UDP:
        {
          st->state = MODE_STATE_CONNECTED;
          vTaskDelay(100);
          /* Ключик на прием, данных пока еше нет для передачи */
          DE_EN(st,0);
          /* Включаем DMA прием данных в буффер */
          dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
          /**
           * Подсоединен
           */ 
          while(st->state == MODE_STATE_CONNECTED)
          {
            st->rblen = recv(st->csock, st->rbuffer, MODE_STRUCT_MAX_MSG_SIZE,0);
            if((st->rblen)<=0)
            {
              /* Ничего не получили, соединение закрываем */
              CloseCSocket(st);
              st->state = MODE_STATE_LISTEN;
              dma_transfer_stop(st->dma_send);
              dma_transfer_stop(st->dma_recv);
              /* Ключик на прием */
              DE_EN(st,0);
              break;
            }
            /* Ключик на передачу */
            DE_EN(st,1);
            dma_transfer(st->dma_send, st->rbuffer, st->rblen);
            dma_transfer_wait(st->dma_send);
          }
        }
        break;
      case SETTINGS_IP_PORT_SIGRAND:
        {
//          static char cfg[100] = "cfg\r";
          static char disconnect[100] = "eth 10 full /auto\r";
          static char connect[100] = "eth 100 full /auto\r";
          if(st->state == MODE_STATE_CONNECTING)
          {
            strcpy((char *)st->rbuffer,connect);
            dma_transfer(st->dma_recv, (char *)st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            dma_transfer(st->dma_send, (char *)st->rbuffer, strlen((char *)st->rbuffer));
          }
          else
          if(st->state == MODE_STATE_CLOSING)
          {
            strcpy((char *)st->rbuffer,disconnect); // Отключение модема
            dma_transfer(st->dma_recv, (char *)st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            dma_transfer(st->dma_send, (char *)st->rbuffer, strlen((char *)st->rbuffer));
            st->state = MODE_STATE_CLOSED;
          }
          else
          if(st->state != MODE_STATE_CLOSING)
          {
            //strcpy(st->rbuffer,cfg);
            //dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            //dma_transfer(st->dma_send, st->rbuffer, strlen(st->rbuffer));
          }
          vTaskDelay(1000);
        }
        break;
      default:
        {
          while(1);
        }
        break;
    };
  }
}

//Параметры поллинга по умолчанию:
struct sMBParam gMBParam = {
  .Chan   = 1,      //Канал RS485
  .Addr   = 1,      //Адрес
  .Func   = 3,      //Функция
  .Start  = 0,      //Начальный регистр
  .Number = 1,      //Количество регистров
  .Period = 1000,   //Период опроса
  .Error  = 0,      //Ошибка
  .rxCount = 0,     //Принятых байт
//  .Chan   = 2,      //Канал RS485
//  .Addr   = 7,      //Адрес
//  .Func   = 3,      //Функция
//  .Start  = 40000,  //Начальный регистр
//  .Number = 40,     //Количество регистров
//  .Period = 1000,   //Период опроса
//  .Error  = 0,      //Ошибка
//  .rxCount = 0,     //Принятых байт
};

struct sMBParam * GetPollMBParam() {
  return &gMBParam;
}

int pack_rtu_regs(struct sMBParam * ipMBParam);

static int mode_repack_rtu_tcp_mberr(struct mode_struct_s *st);
static void mode_socket_send_thread( void * arg )
{
  struct mode_struct_s *st = (struct mode_struct_s *)arg;
  struct sockaddr_in ca;
  st->tblen = 0;
  st->rblen = 0;
  
  init_socket_address(&ca, st->cport, st->ip.addr);
  
  while(1)
  {
//    if(st->mode == SETTINGS_IP_PORT_SIGRAND)
//    {
//      int online = -1;
//      if(st->tblen)
//      {
//        int ind=0;
//        char *ret = NULL;
//        /* Проверяем содержание слова online или offline 
//           в модеме и управляем соответственно индикацией и 
//           перезагрузкой модема */
//        for(ind=0;ind<st->tblen;ind++)
//        {
//          if(st->tbuffer[ind] == 0)
//            st->tbuffer[ind] = 0x20;
//        }
//        
//        if(st->tbuffer[1] == 'S')
//        {
//          ret = NULL;
//        }
//        if(ret = strstr((char *)st->tbuffer, "Mbit/s"))
//        {
//          st->state = MODE_STATE_CONNECTED;
//        }
//        else
//        if(ret = strstr((char *)st->tbuffer, "online"))
//        {
//          online = 1;
//        }
//        else
//        if(ret = strstr((char *)st->tbuffer, "offline"))
//        {
//          online = 0;
//        }
//        
//        memset(st->tbuffer,0,MODE_STRUCT_MAX_MSG_SIZE);
//        
//        if(online == 1)
//        {
//          /* 
//           * Здесь добавим выключение индикации порта подключенного к модему
//           * для того, чтобы не нервировать пользователей индикацией, показывающей,
//           * в общем-то неверные данные.
//           */
//          //extio_ethernet_led(1);
//          if(st->state != MODE_STATE_CONNECTED)
//          {
//            st->state = MODE_STATE_CONNECTING;
//          }
//        }
//        else if(online == 0)
//        {
//          //extio_ethernet_led(0);
//          if(st->state == MODE_STATE_CONNECTED)
//          {
//            st->state = MODE_STATE_CLOSING;
//          }
//        }
//        else /* if(online != 0 || online != 1)*/
//        {
////          if(st->state == MODE_STATE_CONNECTED)
////          {
////            st->state = MODE_STATE_NONE;
////            st->state = MODE_STATE_CLOSING;
////          }
//        }
//        
//        st->tblen = 0;
//        dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
//        
//      }
//      else
//      {
//        vTaskDelay(10);
//      }
//    }
    if (gApplying) {
      if(st->tblen) {
        if (pack_rtu_regs(&gMBParam)) {
        }
        st->tblen = 0;
        dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
      }
      vTaskDelay(5);
      continue;
    }
    if( st->state == MODE_STATE_NONE )
    {
      st->tblen = 0;
      st->rblen = 0;
      vTaskDelay( 10 );
    }
    if( st->state == MODE_STATE_CONNECTED )
    {
      switch( st->mode )
      {
        case SETTINGS_IP_PORT_TCP_SERVER:
        case SETTINGS_IP_PORT_TCP_CLIENT:
          {
            if(st->tblen)
            {
              send(st->csock,st->tbuffer,st->tblen, 0);
              st->tblen = 0;
              mode_led_uart_blink(st);
              dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            }
            else
            {
              if (st->mode == SETTINGS_IP_PORT_TCP_CLIENT) {
                if (st->state == MODE_STATE_RESTART) {
                  continue;
                }
              }
              vTaskDelay(3);
            }
          }
          break;
        case SETTINGS_IP_PORT_MODBUS_GW:
        case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
          {
            //if(!(st->autoclose))
            {
              if (st->state != MODE_STATE_CONNECTED)
                break;
              if(st->tblen)
              {
                if (st->tbuffer[0] != GW_STATE_SERVER_ADDR)
                  mode_led_uart_blink(st);
                //Контроль формата Modbus кадров
                if(mode_repack_rtu_tcp_mberr(st))
                {
                  int cSendResult = send(st->csock,st->tbuffer,st->tblen, 0);
#if (MODE_OVER_TCP_CRC_CONTROL == 0) //Контроль CRC в режиме Modbus over TCP
                  if ((st->mode == SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP) ||
                      (st->mode == SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP)) {
                    if (xSemaphoreTake(mode_timeout_mutex, 200)) {
                      if ((cSendResult == -1) && (st->gTimeout > 0)) {
                        //Перезапуск сокета
                        st->gTimeout = 0;
                      } else {
                        st->gTimeout = 5000;
                      }
                      xSemaphoreGive(mode_timeout_mutex);
                    }
                  }
#endif
                }
                
                st->tblen = 0;
                dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
              }
              else
              {
                vTaskDelay(5);
              }
            } //else vTaskDelay(1);
          }
          break;
        case SETTINGS_IP_PORT_MODBUS_GW_UDP:
        case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
          {
            if (st->state != MODE_STATE_CONNECTED) {
              break;
            }
            if(st->tblen)
            {
              if (st->tbuffer[0] != GW_STATE_SERVER_ADDR)
                mode_led_uart_blink(st);
              //Контроль формата Modbus кадров
              if(mode_repack_rtu_tcp_mberr(st))
              {
                int cSendResult = send(st->csock,st->tbuffer,st->tblen, 0);
#if (MODE_OVER_TCP_CRC_CONTROL == 0) //Контроль CRC в режиме Modbus over TCP
                if ((st->mode == SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP) ||
                    (st->mode == SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP)) {
                  if (xSemaphoreTake(mode_timeout_mutex, 200)) {
                    if ((cSendResult == -1) && (st->gTimeout > 0)) {
                      //Перезапуск сокета
                      st->gTimeout = 0;
                    } else {
                      st->gTimeout = 5000;
                    }
                    xSemaphoreGive(mode_timeout_mutex);
                  }
                }
#endif
              }
                
              st->tblen = 0;
              dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            }
            else
            {
              vTaskDelay(5);
            }
            if (st->count) {
              if (xTaskGetTickCount() >= st->count) {
                st->count = 0;
              }
            }
          }
          break;
        case SETTINGS_IP_PORT_UDP:
          {
            if(st->tblen)
            {
              
              mode_led_uart_blink(st);
              
              sendto(st->csock, st->tbuffer, st->tblen, 0,(struct sockaddr *)&ca, sizeof(struct sockaddr_in));
              st->tblen = 0;
              dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
            }
            else
            {
              vTaskDelay(10);
            }
          }
          break;
        case SETTINGS_IP_PORT_SIGRAND:
          {
          }
          break;
        default:
          {
            while(1);
          }
          break;
      };
    }
    else
    {
      if (st->tblen) {
        mode_led_uart_blink(st);
        st->tblen = 0;
        dma_transfer(st->dma_recv, st->tbuffer, MODE_STRUCT_MAX_MSG_SIZE);
      }
      if (!gApplying) {
        st->tblen = 0;
        st->rblen = 0;
      }
      vTaskDelay(10);
    }
  }
}

int mode_init(struct mode_struct_s *st)
{
  static int index = 0;
  char buffer[60];
  st->state = MODE_STATE_NONE;
  st->tblen = 0;
  st->rblen = 0;
  
  DE_EN(st,0);
  
  st->autoclose = DEFAULT_AUTO_CLOSE;
  
  InitServerRegisters(st);
  
  switch(st->mode)
  {
    case SETTINGS_IP_PORT_MODBUS_GW:
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
    case SETTINGS_IP_PORT_MODBUS_GW_UDP:
    case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
      {
        sprintf(buffer,"MBgwTx-%d",index);
        sys_thread_new(buffer, mode_socket_recv_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        sprintf(buffer,"MBgwRx-%d",index++);
        sys_thread_new(buffer, mode_socket_send_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        mode_timeout_mutex = xSemaphoreCreateMutex();
      }
      break;
    case SETTINGS_IP_PORT_TCP_SERVER:
      {
        sprintf(buffer,"TCPsTx-%d",index);
        sys_thread_new(buffer, mode_socket_recv_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        sprintf(buffer,"TCPsRx-%d",index++);
        sys_thread_new(buffer, mode_socket_send_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
      }
      break;
    case SETTINGS_IP_PORT_TCP_CLIENT:
      {
        sprintf(buffer,"TCPcTx-%d",index);
        sys_thread_new(buffer, mode_socket_recv_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        sprintf(buffer,"TCPcRx-%d",index++);
        sys_thread_new(buffer, mode_socket_send_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
      }
      break;
    case SETTINGS_IP_PORT_UDP:
      {
        sprintf(buffer,"UDPt-%d",index);
        sys_thread_new(buffer, mode_socket_recv_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        sprintf(buffer,"UDPr-%d",index++);
        sys_thread_new(buffer, mode_socket_send_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
      }
      break;
    case SETTINGS_IP_PORT_SIGRAND:
      {
        /***************************************************
         * Управление светодиодом, изначально выключаем,   *
         * включаем только при почвлении отклика от модема *
         ***************************************************/
        //extio_ethernet_led( 0 );
        sprintf(buffer,"shdslTx-%d",index);
        sys_thread_new(buffer, mode_socket_recv_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
        sprintf(buffer,"shdslRx-%d",index++);
        sys_thread_new(buffer, mode_socket_send_thread, st, DEFAULT_THREAD_STACKSIZE, MODE_THREAD_PRIO);
      }
      break;
    default:
      {
        return -1;
      }
      break;
  };
  return 0;
}

static int mode_repack_tcp_rtu(struct mode_struct_s *st)
{
  if (st->rblen == 1) {
    return 0;
  }
  char *msg = (char*)st->rbuffer;
  int32_t *mlen = &st->rblen;
  uint16_t crc = 0;
  switch (st->mode) {
  case SETTINGS_IP_PORT_MODBUS_GW:
  case SETTINGS_IP_PORT_MODBUS_GW_UDP:
    /* разбираем кадр MBAP */
    st->last_modbus_tcp_udp_header.tid    =   (((uint16_t)(msg[0])<<8)|((uint16_t)(msg[1])));
    st->last_modbus_tcp_udp_header.proto  =   (((uint16_t)(msg[2])<<8)|((uint16_t)(msg[3])));
    st->last_modbus_tcp_udp_header.len    =   (((uint16_t)(msg[4])<<8)|((uint16_t)(msg[5])));
    st->last_modbus_tcp_udp_header.uid    =   (msg[6]);
    if(st->last_modbus_tcp_udp_header.len < 1 || st->last_modbus_tcp_udp_header.len >= MODBUS_TCP_MAX_PDU_SIZE)
    {
      return 0;
    }
    memcpy((st->last_modbus_tcp_udp_header.buffer),&(msg[7]),st->last_modbus_tcp_udp_header.len);
    /* Формируем кадр с CRC */
    memcpy(msg,&(msg[6]),st->last_modbus_tcp_udp_header.len);
    /* добавляем CRC */
    crc = modbus_gw_crc16(msg,st->last_modbus_tcp_udp_header.len);
    *mlen = st->last_modbus_tcp_udp_header.len;
    msg[*mlen] = (crc >> 8);
    msg[*mlen+1] = (crc & 0xFF);
    *mlen = st->last_modbus_tcp_udp_header.len+2;
    break;
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
    break;
  }
  return 1;
}

void rtu_to_tcp(struct mode_struct_s *st) {
  char *msg = (char*)st->tbuffer;
  int32_t *mlen = &st->tblen;
  char *src = &msg[*mlen - 2];
  char *dst = &msg[*mlen - 2 + 6];
  /* Пакуем данные */
  while(src >= msg) { *dst-- = *src--; }
  /* Пакуем заголовок */
  st->last_modbus_tcp_udp_header.len = *mlen - 2;
  msg[0] = st->last_modbus_tcp_udp_header.tid >> 8;
  msg[1] = st->last_modbus_tcp_udp_header.tid;
  msg[2] = st->last_modbus_tcp_udp_header.proto >> 8;
  msg[3] = st->last_modbus_tcp_udp_header.proto;
  msg[4] = st->last_modbus_tcp_udp_header.len >> 8;
  msg[5] = st->last_modbus_tcp_udp_header.len;
  *mlen += 6 - 2;
}

static int mode_repack_rtu_tcp_mberr(struct mode_struct_s *st)
{
  char *msg = (char*)st->tbuffer;
  int32_t *mlen = &st->tblen;
  uint16_t crc;
  crc = modbus_gw_crc16(msg,*mlen-2);
  bool cCRCright = (0 == (crc ^ (msg[*mlen-2]<<8 | msg[*mlen-1])));
  bool cErrMsg = (msg[1] > 0x80);
  switch (st->mode) {
  case SETTINGS_IP_PORT_MODBUS_GW:
  case SETTINGS_IP_PORT_MODBUS_GW_UDP:
    if(st->last_modbus_tcp_udp_header.uid == msg[0])
    {
      if(cCRCright) {
        //При чтении сервера - не очищаем регистр ошибок
        if (msg[0] != GW_STATE_SERVER_ADDR) {
          if (cErrMsg) { //Сообщение об ошибке
            st->modbus_server_state.UartError = msg[2];
          } else {
            st->modbus_server_state.UartError = mseNoError;
          }
          st->modbus_server_state.UartErrValue = 0;
        }
        rtu_to_tcp(st);
        return 1;
      } else { //Проверка ответа по формату кадров стандарта Modbus
        uint16_t cSize;
        if (cErrMsg) { //Сообщение об ошибке
          cSize = 3;
          st->modbus_server_state.UartError = msg[2];
          st->modbus_server_state.UartErrValue = 0;
        } else {
          switch (msg[1] & 0x7f) {
          case 1:   case 2:   case 3:   case 4:
            cSize = 3 + msg[2];
            break;
          case 5:   case 6:   case 15:   case 16:
            cSize = 6;
            break;
          default:
            st->modbus_server_state.UartError = mseCrc;
            st->modbus_server_state.UartErrValue = 0;
            rtu_to_tcp(st);
            return 1;
          }
        }
        if (cSize == (*mlen - 2)) { //Размер совпадает = это ошибка CRC
          st->modbus_server_state.UartError = mseCrc;
          st->modbus_server_state.UartErrValue = 0;
          return 0;
        }
        crc = modbus_gw_crc16(msg, cSize);
        cCRCright = (0 == (crc ^ (msg[cSize] << 8 | msg[cSize + 1])));
        if (cCRCright) {
          st->modbus_server_state.UartError = mseLength;
          st->modbus_server_state.UartErrValue = *mlen - 2 - cSize;
          *mlen = cSize + 2;
          rtu_to_tcp(st);
          return 1;
        } else {
        }
      }
    }
    break;
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_TCP:
  case SETTINGS_IP_PORT_MODBUS_RTU_OVER_UDP:
#if (MODE_OVER_TCP_CRC_CONTROL) //Контроль CRC в режиме Modbus over TCP,UDP
    if(cCRCright)
#endif
    {
      return 1;
    }
  }
  return 0;
}

//Поллинг RS485-MODBUS +
int pack_regs_rtu(struct sMBParam * ipMBParam) {
  struct mode_struct_s *st = GetMode(ipMBParam->Chan - 1);
  char *msg = (char*)st->rbuffer;
  int32_t *mlen = &st->rblen;
  uint16_t crc = 0;
  /* разбираем кадр MBAP */
  st->last_modbus_tcp_udp_header.uid    =   ipMBParam->Addr;
  st->last_modbus_tcp_udp_header.len    =   6;
  
  if(st->last_modbus_tcp_udp_header.len < 1 || st->last_modbus_tcp_udp_header.len >= MODBUS_TCP_MAX_PDU_SIZE)
  {
    return 0;
  }
  
  /* Формируем кадр с CRC */
  msg[0] = ipMBParam->Addr;
  msg[1] = ipMBParam->Func;
  msg[2] = (ipMBParam->Start >> 8) & 0xff;
  msg[3] = ipMBParam->Start & 0xff;
  msg[4] = (ipMBParam->Number >> 8) & 0xff;
  msg[5] = ipMBParam->Number & 0xff;
  
  /* добавляем CRC */
  crc = modbus_gw_crc16(msg, st->last_modbus_tcp_udp_header.len);
  *mlen = st->last_modbus_tcp_udp_header.len;
  msg[*mlen] = ((crc >> 8) & 0xFF);
  msg[*mlen + 1] = (crc & 0xFF);
  *mlen = st->last_modbus_tcp_udp_header.len + 2;
  return 1;
}

void PackAndSendRtuFrame(struct mode_struct_s *st) {
  //Сформировать кадр MODBUS и передать
  if(pack_regs_rtu(&gMBParam)) {
    /* Если парсинг удался, отправляем в USART */
    DE_EN(st, 1);
    /** * Отправляем, ожидаем конца передачи */ 
    dma_transfer(st->dma_send, st->rbuffer, st->rblen);
    dma_transfer_wait(st->dma_send);
  }
  //Обнулить прием
  gMBParam.Error = mbrNone;
  gMBParam.rxCount = 0;
  memset((uint8_t *)&gMBParam.rBuffer[0], 0, sizeof(gMBParam.rBuffer));
}

extern struct mode_struct_s * GetMode(int iModeIdx);
//Сообщения об ошибках. Массив редактируется совместно с enum eMBResult
char aErrorMsg[mbrCount][30] = {
  "- -",
  "Кадр принят",
  "Таймаут (нет кадра)",
  "Ошибка CRC",
  "Ошибка MODBUS"
};

int pack_rtu_regs(struct sMBParam * ipMBParam)
{
  uint16_t crc;
  struct mode_struct_s *st = NULL;
  st = GetMode(ipMBParam->Chan - 1);
  char *msg = (char*)st->tbuffer;
  int32_t *mlen = &st->tblen;
  crc = modbus_gw_crc16(msg,*mlen-2);
  int cCount = msg[2];
  int cFunc = msg[1];
  if(0 == (crc ^ (msg[*mlen-2]<<8 | msg[*mlen-1])))
  {
    if (cFunc & 0x80) { //Ошибка по MODBUS протоколу
      ipMBParam->Error = mbrModbus; //0x8000 | cCount;
      ipMBParam->rxCount = 0;
      cCount = 0;
      return 1;
    }
    char *src = &msg[*mlen-3];
    char *dst = (char *)&ipMBParam->rBuffer[*mlen-3 - 3];
    /* Пакуем данные */
    while(src >= (msg + 3)){*dst-- = *src--;}
    dst = (char *)&ipMBParam->rBuffer[0];
    char cBuf;
    for (int i = 0; i < cCount; i += 2) {
      cBuf = dst[i]; dst[i] = dst[i + 1];
      dst[i + 1] = cBuf;
    }
    ipMBParam->rxCount = cCount;
    ipMBParam->Error = mbrRxGood;
    return 1;
  } else {
    ipMBParam->rxCount = 0;
    ipMBParam->Error = mbrCRC;
    return 1;
  }
//  return 0;
}
//Поллинг RS485-MODBUS -

static uint16_t modbus_gw_crc16(char* data, int32_t len)
{
    return mbcrc((unsigned char *)data,len);
}

