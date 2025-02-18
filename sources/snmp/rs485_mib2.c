//******************************************************************************
// Имя файла    :       rs485_mib2.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       04.02.2019
//
//------------------------------------------------------------------------------
/**
База данных MIB2 параметров портов RS485

**/

#include "rs485_mib2.h"
#include "settings.h"
#include "mode.h"
extern struct settings_s *rsettings;
extern struct mode_struct_s * GetMode(int iModeIdx);

static s16_t getRS485Param(struct snmp_node_instance * instance, void* value, int iPort) {
  struct mode_struct_s *st = GetMode(iPort);
  u32_t * uint_ptr = (u32_t*) value;
  u32_t cValue;
  switch (instance->node->oid) {
  case 1:
    cValue = rsettings->uart[iPort].baudrate;
    *uint_ptr = cValue;
    break;
  case 2:
    cValue = rsettings->uart[iPort].parity;
    *uint_ptr = cValue;
    break;
  case 3:
    cValue = rsettings->uart[iPort].stopbits;
    *uint_ptr = cValue;
    break;
  case 4:
    switch (iPort) {
    case 0:
      cValue = (extio_read_idx(EXTIO_CPU_33_V5)>0)?0:1;
      break;
    case 1:
      cValue = (extio_read_idx(EXTIO_CPU_33_V4)>0)?0:1;
      break;
    default:
      cValue = 0;
      break;
    }
    *uint_ptr = cValue;
    break;
  case 5:
    cValue = (st->state == MODE_STATE_CONNECTED) ? 1:0;
    *uint_ptr = cValue;
    break;
  case 6:
    switch (iPort) {
    case 0:
      cValue = (extio_led_blinked(EXTIO_RS485_1)>0) ? 1:0;
      break;
    case 1:
      cValue = (extio_led_blinked(EXTIO_RS485_2)>0) ? 1:0;
      break;
    default:
      cValue = 0;
      break;
    }
    *uint_ptr = cValue;
    break;
  default:
    return 0;
  }
  return sizeof(*uint_ptr);
}

static s16_t getSettingsRS485Port1(struct snmp_node_instance * instance, void* value) {
  return getRS485Param(instance, value, 0);
}

static s16_t getSettingsRS485Port2(struct snmp_node_instance * instance, void* value) {
  return getRS485Param(instance, value, 1);
}

extern bool IsBaudrateRight(uint32_t iBaudrate);
#include "log.h"

static snmp_err_t setRS485Param(struct snmp_node_instance* instance, u16_t len, void *value, int iPort) {
#ifdef RS485_ENABLE_SETTINGS
  u32_t * uint_ptr = (u32_t*) value;
  uint32_t cValue = *uint_ptr;
  bool cChanged = false;
  switch (instance->node->oid) {
  case 1: //Скорость
    if (IsBaudrateRight(cValue)) {
      if (cValue != rsettings->uart[iPort].baudrate) {
        rsettings->uart[iPort].baudrate = cValue;
        SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)(iPort + 10), snSpeed, cValue);
        cChanged = true;
      } else {
        return SNMP_ERR_WRONGVALUE;
      }
    } else {
      return SNMP_ERR_WRONGVALUE; //Не соответствует таблице скоростей
    }
    break;
  case 2: //Четность
    if (cValue <= 2) {
      if (cValue != rsettings->uart[iPort].parity) {
        rsettings->uart[iPort].parity = (enum settings_uart_parity)cValue;
        SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)(iPort + 10), snParity, cValue);
        cChanged = true;
      } else {
        return SNMP_ERR_WRONGVALUE;
      }
    } else {
      return SNMP_ERR_WRONGVALUE; //Не соответствует settings_uart_parity
    }
    break;
  case 3: //Стоп биты
    if ((cValue > 0) && (cValue <= 2)) {
      if (cValue != rsettings->uart[iPort].stopbits) {
        rsettings->uart[iPort].stopbits = (enum settings_uart_stopbits)cValue;
        SaveSettingEvent(etSetting, ecParamChangePort, (enum ePortName)(iPort + 10), snStopBits, cValue);
        cChanged = true;
      } else {
        return SNMP_ERR_WRONGVALUE;
      }
    } else {
      return SNMP_ERR_WRONGVALUE; //Не соответствует settings_uart_stopbits
    }
    break;
  default:
    return SNMP_ERR_NOSUCHINSTANCE;
  }
  if (cChanged) {
    return SNMP_ERR_NOERROR;
  }
  return SNMP_ERR_NOERROR;
#else
  return SNMP_ERR_NOACCESS;
#endif
}

static snmp_err_t setSettingsRS485Port1(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setRS485Param(instance, len, value, 0);
}

static snmp_err_t setSettingsRS485Port2(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setRS485Param(instance, len, value, 1);
}

//rs485 ports
static const struct snmp_scalar_node aRS485Port1Param[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, setSettingsRS485Port1),
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, setSettingsRS485Port1),
  SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, setSettingsRS485Port1),
  SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port1, NULL, NULL)
};

static const struct snmp_node* const settings_rs485_port1[] = {
  &aRS485Port1Param[0].node.node,
  &aRS485Port1Param[1].node.node,
  &aRS485Port1Param[2].node.node,
  &aRS485Port1Param[3].node.node,
  &aRS485Port1Param[4].node.node,
  &aRS485Port1Param[5].node.node,
};

static const struct snmp_scalar_node aRS485Port2Param[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, setSettingsRS485Port2),
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, setSettingsRS485Port2),
  SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, setSettingsRS485Port2),
  SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_ONLY  , SNMP_ASN1_TYPE_INTEGER, getSettingsRS485Port2, NULL, NULL)
};

static const struct snmp_node* const settings_rs485_port2[] = {
  &aRS485Port2Param[0].node.node,
  &aRS485Port2Param[1].node.node,
  &aRS485Port2Param[2].node.node,
  &aRS485Port2Param[3].node.node,
  &aRS485Port2Param[4].node.node,
  &aRS485Port2Param[5].node.node,
};

static const struct snmp_tree_node settings_rs485_port1_node = SNMP_CREATE_TREE_NODE(1, settings_rs485_port1);
static const struct snmp_tree_node settings_rs485_port2_node = SNMP_CREATE_TREE_NODE(2, settings_rs485_port2);

static const struct snmp_node* const settings_rs485_node[] = {
  &settings_rs485_port1_node.node,
  &settings_rs485_port2_node.node,
};

static const struct snmp_tree_node rs485_mib2_root = SNMP_CREATE_TREE_NODE(7, settings_rs485_node);


