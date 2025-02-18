//******************************************************************************
// Имя файла    :       power_mib2.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       07.12.2018
//
//------------------------------------------------------------------------------
/**
База данных MIB2 параметров питания МКПШ

**/

#include "power_mib2.h"
#include "vcc_ctrl.h"


static s16_t getDevicePower(struct snmp_node_instance * instance, void* value) {
  u32_t * uint_ptr = (u32_t*) value;
  float cVcc = 0;
  u32_t cVcc10;
  switch (instance->node->oid) {
  case 1:
    cVcc = GetAverageVcc();
    cVcc10 = cVcc * 10;
    *uint_ptr = cVcc10;
    break;
  case 2:
    cVcc10 = (u32_t)GetVccZone();
    if (cVcc10 >=10) //!!! Зависит от состава eVccState
      cVcc10 -= 7;
    *uint_ptr = cVcc10;
    break;
  default:
    return 0;
  }
  return sizeof(*uint_ptr);
}

//Для организации записи критических уставок в МКПШ-10, IMC-FTX

#include "info.h"
#define KW_NUM  (4)
// !!! Ключ
//uint32_t gSerialKey[KW_NUM]   = {0x2da00e75, 0x0ed93a17, 0x52334aa8, 0x4f3dd294 };
uint32_t gSerialKey[KW_NUM]   = {765464181, 249117207, 1379093160, 1329451668 };
//uint32_t gHardwareKey[KW_NUM] = {0x5338a244, 0x16db0a7e, 0x5c38b905, 0x191e809a };
uint32_t gHardwareKey[KW_NUM] = {1396220484, 383453822, 1547221253, 421429402 };
uint8_t gSKIdx = 0, gHKIdx = 0; //Индекс текущего слова для проверки ключа
// !!!

static snmp_err_t setCritical(struct snmp_node_instance* instance, u16_t len, void *value) {
  u32_t * uint_ptr = (u32_t*) value;
  uint32_t cValue = *uint_ptr;
  snmp_err_t cError = SNMP_ERR_NOERROR;
  switch (instance->node->oid) {
  case 1: //Ключ
    if (gSKIdx < KW_NUM) {
      if (cValue == gSerialKey[gSKIdx]) {
        ++gSKIdx;
        cError = SNMP_ERR_NOERROR;
        break;
      } else {
        gSKIdx = 0;
      }
    } else {
      gSKIdx = 0;
    }
    if (gHKIdx < KW_NUM) {
      if (cValue == gHardwareKey[gHKIdx]) {
        ++gHKIdx;
        cError = SNMP_ERR_NOERROR;
        break;
      } else {
        gHKIdx = 0;
      }
    } else {
      gHKIdx = 0;
    }
    cError = SNMP_ERR_WRONGVALUE;
    break;
  case 2: //Зав. номер
    if (gSKIdx == KW_NUM) {
      gSKIdx = 0;
      if (WriteHWSetting(cValue, instance->node->oid) == otp_result_ok) {
        cError = SNMP_ERR_NOERROR;
        break;
      } else {
        cError = SNMP_ERR_RESOURCEUNAVAILABLE; //Нет свободных блоков в OTP
      }
    } else {
      gSKIdx = 0;
      cError = SNMP_ERR_COMMITFAILED;
    }
    break;
  case 3: //Версия HW
    // !!! Для проверки записи
    if (gHKIdx == KW_NUM) {
      gHKIdx = 0;
      if (WriteHWSetting(cValue, instance->node->oid) == otp_result_ok) {
        cError = SNMP_ERR_NOERROR;
        break;
      } else {
        cError = SNMP_ERR_RESOURCEUNAVAILABLE; //Нет свободных блоков в OTP
      }
    } else {
      gHKIdx = 0;
      cError = SNMP_ERR_COMMITFAILED;
    }
    break;
#if ((MKPSH10 != 0) || (IMC_FTX_MC != 0))
  case 4: //Версия HW + MAC[0, 1]
    // !!! Для проверки записи
    if (gHKIdx == KW_NUM) {
      gHKIdx = 0;
      if (WriteHWSetting(cValue, instance->node->oid) == otp_result_ok) {
        cError = SNMP_ERR_NOERROR;
        break;
      } else {
        cError = SNMP_ERR_RESOURCEUNAVAILABLE; //Нет свободных блоков в OTP
      }
    } else {
      gHKIdx = 0;
      cError = SNMP_ERR_COMMITFAILED;
    }
    break;
#endif
  case 5: //Коррекция времени
    if (cValue != rsettings->TimeCorr) {
      rsettings->TimeCorr = cValue;
      settings_save();
      cError = SNMP_ERR_NOERROR;
    } else {
      cError = SNMP_ERR_WRONGVALUE;
    }
    break;
  case 6: //Интервал коррекции времени
    if (cValue != rsettings->IntervalCorr) {
      rsettings->IntervalCorr = cValue;
      settings_save();
      cError = SNMP_ERR_NOERROR;
    } else {
      cError = SNMP_ERR_WRONGVALUE;
    }
    break;
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
    cError = SNMP_ERR_NOTWRITABLE;
    break;
  default:
    cError = SNMP_ERR_NOSUCHINSTANCE;
    break;
  }
  return cError;
}

static s16_t getCritical(struct snmp_node_instance * instance, void* value) {
  u32_t * uint_ptr = (u32_t*) value;
  u32_t cValue;
  switch (instance->node->oid) {
  case 5:
    cValue = rsettings->TimeCorr;
    *uint_ptr = cValue;
    break;
  case 6:
    cValue = rsettings->IntervalCorr;
    *uint_ptr = cValue;
    break;
  default:
    return 0;
  }
  return sizeof(*uint_ptr);
}

//Параметры Vcc
static const struct snmp_scalar_node aDevicePower[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_INTEGER, getDevicePower, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_INTEGER, getDevicePower, NULL, NULL)
};

static const struct snmp_node* const settings_device_power[] = {
  &aDevicePower[0].node.node,
  &aDevicePower[1].node.node,
};

static const struct snmp_tree_node power_mib2_root = SNMP_CREATE_TREE_NODE(6, settings_device_power);


//Критические параметры записи: Версия аппаратуры, Заводской номер
static const struct snmp_scalar_node aCritical[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_WRITE_ONLY , SNMP_ASN1_TYPE_INTEGER, NULL, NULL, setCritical),  //Ключ
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_WRITE_ONLY , SNMP_ASN1_TYPE_INTEGER, NULL, NULL, setCritical),  //Зав. номер
  SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_WRITE_ONLY , SNMP_ASN1_TYPE_INTEGER, NULL, NULL, setCritical),  //Версия HW
  SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_WRITE_ONLY , SNMP_ASN1_TYPE_INTEGER, NULL, NULL, setCritical),  //hardware_Hw_Version_MAC2
  SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getCritical, NULL, setCritical),  //Коррекция времени
  SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_WRITE , SNMP_ASN1_TYPE_INTEGER, getCritical, NULL, setCritical)   //Интервал коррекции времени
};

static const struct snmp_node* const settings_critical[] = {
  &aCritical[0].node.node,
  &aCritical[1].node.node,
  &aCritical[2].node.node,
  &aCritical[3].node.node,
  &aCritical[4].node.node,
  &aCritical[5].node.node,
};

static const struct snmp_tree_node critical_mib2_root = SNMP_CREATE_TREE_NODE(10, settings_critical);


