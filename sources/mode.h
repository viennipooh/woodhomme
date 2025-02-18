//******************************************************************************
// ��� �����    :       mode.h
// ���������    :       
// �����        :       �������� �.�.
// ����         :       15.02.2016
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

#define MODE_OVER_TCP_CRC_CONTROL ( 0 ) //�������� CRC � ������ Modbus over TCP

#define MODE_THREAD_PRIO ( tskIDLE_PRIORITY + 2 )

/* ������ RS485 ��� ���������� �������� � 1 � � 0 ������ �������� */
#define DE_EN(st,en) {if(en && st->gpio_re)\
                     {st->gpio_re->BSRRL |= (1<<st->pin_re);}\
                     else\
                     {st->gpio_re->BSRRH |= (1<<st->pin_re);};}

//���� Mikrotik ������ � ���-�
#if (UTD_M != 0)
  #define MODE_USE_MIKROTIK //������������ Mikrotik (1 ������ �� �����)
#endif

//����������� Modbus TCP �������
#define MB_SERVER_REG_START ( 0 ) //����� ������� ��������
#define MB_SERVER_BASE_REG_NUM ( 2 ) //���������� ��������� � ������� �����

#if (IIP == 0)
  #define MODE_TCP_KEEPALIVE  //�������� Keepalive � ��������� TCP
  #define MODE_DEBUG  //������������� ���������� ������ ��� Keepalive
#endif

enum mode_state_e {
  MODE_STATE_NONE = 0,
  MODE_STATE_LISTEN,
  MODE_STATE_CONNECTING,
  MODE_STATE_CONNECTED,
  MODE_STATE_RESTART,     //������� ������(��) � ����������
  MODE_STATE_CLOSING,
  MODE_STATE_CLOSED,
};

//��������� TCP-���������
#define MODBUS_TCP_BUFFER_MAX   0x100
#define MODBUS_TCP_MAX_PDU_SIZE MODBUS_TCP_BUFFER_MAX
struct modbus_tcp_udp_header_s {
  uint16_t tid;
  uint16_t proto;
  uint16_t len;
  uint8_t  uid;
  uint8_t buffer[MODBUS_TCP_BUFFER_MAX];
};

// �������� ������� Modbus TCP
enum eMbError { //����������� ������ Modbus
  meNone      = 0x00, //- ��� ������
  meBadFunc   = 0x01, //- �������� ��� ������� �� ����� ���� ���������.
  meBadAddr   = 0x02, //- ����� ������, ��������� � �������, ����������.
  meBadData   = 0x03, //- ��������, ������������ � ���� ������ �������, �������� ������������ ���������.
  meNoRecover = 0x04, //- ������������������� ������ ����� �����, ���� ������� ���������� �������� ��������� ������������� ��������.
  meTimeNeed  = 0x05, //- ������� ���������� ������� ������ � ������������ ���, �� ��� ������� ����� �������.
  meBusy      = 0x06, //- ������� ���������� ������ ���������� �������.
  meCannot    = 0x07, //- ������� ���������� �� ����� ��������� ����������� �������, �������� � �������.
                      //  ���� ��� ������������ ��� ����������� ������������ �������, ������������� ������� � �������� 13 ��� 14.
  meMemCrcErr = 0x08, //- ������� ���������� ��� ������ ����������� ������ ���������� ������ �������� ��������.
};

#define GW_STATE_SERVER_ADDR  (0xff)
enum eMbServerError { //���� ������ Modbus, �������������� ��������
  mseNoError  = 0x00, //������ ���
  mseCrc      = 0x11, //������ CRC
  mseLength   = 0x12, //������ ������� �����
  mseExtFunc  = 0x13, //���������������� �������
};

#pragma pack(1)
#if (IIP != 0)

  #define CHANNEL_SERVER_SIZE (sizeof(struct channel_server_s) / 2)
  #define MB_SERVER_REG_NUM (MB_SERVER_BASE_REG_NUM + 2 + (CHANNEL_SERVER_SIZE * POWER_CHANNEL_NUM))

struct channel_server_s {
  union {
    struct {
      //0 - ������� ���������
      uint16_t  Out_K       : 1;  //��������� ��������� ����
      uint16_t  Channel_I   : 1;  //������� ���� �����������
      uint16_t  Error       : 4;  //������
      uint16_t  Reserv      :10;  //������
      //1, 2, 3 ��������
      uint16_t  Voltage        ;  //���������� � ��������� 0,1�
      uint16_t  Current        ;  //��� � ��������� 0,01�
      uint16_t  VReserv        ;  //������
    };
    uint16_t  Data[4];
  };
};
#else
  #define MB_SERVER_REG_NUM ( MB_SERVER_BASE_REG_NUM ) //���������� ���������
#endif

struct modbus_server_s {
  union {
    struct {
      //0 = SR1 ������� ��������� UART
      uint16_t  UartPower   : 1;  //����������� ������� UART
      uint16_t  UartTx      : 1;  //�������� UART
      uint16_t  UartRx      : 1;  //����� UART
      uint16_t  StateReserv :13;  //������
      //1 = ER1 ������� ������ UART
      uint16_t  UartError   : 8;  //������ �����
      uint16_t  UartErrValue: 4;  //�������� �� ������ �����
      uint16_t  DiagReserv  : 4;  //������
#if (IIP != 0)
      //2 = SR_I1 ������� ��������� ���������
      uint16_t  In_K        : 1;  //��������� �������� ����
      uint16_t  Channel_K   : 4;  //��������� ��������� ���� ������� 1-4
      uint16_t  Channel_I   : 4;  //������� ����������� � ������ (��� > 0)
      uint16_t  IP_Error    : 4;  //������ ���������
      uint16_t  IP_Reserv3  : 3;  //������
      //3 = ������
      uint16_t  IP_Reserv      ;  //������
      //��������� �������
      //4 .. 19 (4 � 4)
      struct channel_server_s aChannel[POWER_CHANNEL_NUM];
#endif
    };
    uint16_t  Data[MB_SERVER_REG_NUM];
  };
};
#pragma pack()

#ifdef MODE_DEBUG  //������������� ���������� ������
// + ����������
struct s_ModeDebug {
  uint32_t  RecvCount, TryRecvCount, NoRecvCount, RecvErrCount,
            TryConnectCount,
            CloseCount, ClosedCount, RestartCount, 
            Recv1ByteCount, Send1ByteCount;
  uint8_t   Err;
  uint32_t  BindCount, AcceptCount, AcceptRestartCount, ConnectedRecvCount;
  
};
// - ����������
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
#ifdef MODE_DEBUG  //������������� ���������� ������
  struct s_ModeDebug    ModeDebug;
#endif
  uint32_t              count; //������� ��� �������� ������� ������ ��������
};

#define MODE_TX_COUNTER_VALUE (2000)

#pragma pack(1)

//������� RS485-MODBUS +
enum eMBResult {
  mbrNone,    //������ (��� ����������)
  mbrRxGood,  //�������� �����
  mbrTimeout, //������� (��� �����)
  mbrCRC,     //������ CRC
  mbrModbus,  //������ MODBUS
  mbrCount    //���������� � ������������
};
  
extern char aErrorMsg[][30];

typedef struct sMBParam { //��������� �������� MODBUS
  uint8_t   Chan;     //����� RS485
  uint8_t   Addr;     //�����
  uint8_t   Func;     //�������
  uint16_t  Start;    //��������� �������
  uint16_t  Number;   //���������� ���������
  uint16_t  Period;   //������ ������
  int16_t   Error;    //������
  uint16_t  rxCount;  //�������� ����
  uint8_t   rBuffer[MODE_STRUCT_MAX_MSG_SIZE];
} SMBParam;
//������� RS485-MODBUS -

int mode_init(struct mode_struct_s*);
extern void init_uart_port(struct mode_struct_s * ipMode, struct uart_port_s * ipPort);

#pragma pack()

#endif //__MODE_H_


