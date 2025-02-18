#include "my_mibs.h"
#include "snmp_def.h"

#include "lwip/snmp.h"
#include "lwip/apps/snmp.h"
#include "lwip/apps/snmp_core.h"
#include "lwip/apps/snmp_mib2.h"
#include "lwip/apps/snmp_scalar.h"
#include "lwip/apps/snmp_table.h"
#include "settings.h"
#include <string.h>
#include "info.h"

#include "log.h"

#define SYNC_NODE_NAME(node_name) node_name


#ifdef USE_SWITCH //Использовать свитч kmz8895
static s16_t
statistics_port_get_value(const int iPort, const struct snmp_scalar_array_node_def *node, void *value)
{
  u64_t *uint_ptr = (u64_t*)value;
  switch (node->oid) {
  case 1: 
    *uint_ptr = stat[iPort].RxLoPriorityByte;
    break;
  case 2: 
    *uint_ptr = stat[iPort].RxHiPriorityByte;
    break;
  case 3: 
    *uint_ptr = stat[iPort].RxUndersizePkt;
    break;
  case 4: 
    *uint_ptr = stat[iPort].RxFragments;
    break;
  case 5: 
    *uint_ptr = stat[iPort].RxOversize;
    break;
  case 6: 
    *uint_ptr = stat[iPort].RxJabbers;
    break;
  case 7: 
    *uint_ptr = stat[iPort].RxSymbolError;
    break;
  case 8: 
    *uint_ptr = stat[iPort].RxCRCerror;
    break;
  case 9: 
    *uint_ptr = stat[iPort].RxAlignmentError;
    break;
  case 10: 
    *uint_ptr = stat[iPort].RxControl8808Pkts;
    break;
  case 11: 
    *uint_ptr = stat[iPort].RxPausePkts;
    break;
  case 12: 
    *uint_ptr = stat[iPort].RxBroadcast;
    break;
  case 13: 
    *uint_ptr = stat[iPort].RxMulticast;
    break;
  case 14: 
    *uint_ptr = stat[iPort].RxUnicast;
    break;
  case 15: 
    *uint_ptr = stat[iPort].Rx64Octets;
    break;
  case 16: 
    *uint_ptr = stat[iPort].Rx65to127Octets;
    break;
  case 17: 
    *uint_ptr = stat[iPort].Rx128to255Octets;
    break;
  case 18: 
    *uint_ptr = stat[iPort].Rx256to511Octets;
    break;
  case 19: 
    *uint_ptr = stat[iPort].Rx512to1023Octets;
    break;
  case 20: 
    *uint_ptr = stat[iPort].Rx1024to1522Octets;
    break;
  case 21: 
    *uint_ptr = stat[iPort].TxLoPriorityByte;
    break;
  case 22: 
    *uint_ptr = stat[iPort].TxHiPriorityByte;
    break;
  case 23: 
    *uint_ptr = stat[iPort].TxLateCollision;
    break;
  case 24: 
    *uint_ptr = stat[iPort].TxPausePkts;
    break;
  case 25: 
    *uint_ptr = stat[iPort].TxBroadcastPkts;
    break;
  case 26: 
    *uint_ptr = stat[iPort].TxMulticastPkts;
    break;
  case 27: 
    *uint_ptr = stat[iPort].TxUnicastPkts;
    break;
  case 28: 
    *uint_ptr = stat[iPort].TxDeferred;
    break;
  case 29: 
    *uint_ptr = stat[iPort].TxTotalCollision;
    break;
  case 30: 
    *uint_ptr = stat[iPort].TxExcessiveCollision; 
    break;
  case 31: 
    *uint_ptr = stat[iPort].TxSingleCollision; 
    break;
  case 32: 
    *uint_ptr = stat[iPort].TxMultipleCollision; 
    break;
  case 33: 
    *uint_ptr = stat[iPort].TxDropPackets; 
    break;
  case 34: 
    *uint_ptr = stat[iPort].RxDropPackets; 
    break;
  case 35: 
    *uint_ptr = GetPortTraffic(1, iPort); 
    break;
  case 36: 
    *uint_ptr = GetPortTraffic(0, iPort); 
    break;
  default:    
    return 0;
  }

  return sizeof(u64_t);
}

static s16_t
statistics_port1_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(0, node, value);
}

static s16_t
statistics_port2_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(1, node, value);
}

static s16_t
statistics_port3_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(2, node, value);
}

static s16_t
statistics_port4_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(3, node, value);
}

static s16_t
statistics_port5_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(4, node, value);
}

static s16_t
statistics_port6_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(5, node, value);
}

static s16_t
statistics_port7_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(6, node, value);
}

static s16_t
statistics_port8_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(7, node, value);
}

static s16_t
statistics_port9_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(8, node, value);
}

static s16_t
statistics_port10_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  return statistics_port_get_value(9, node, value);
}

#endif

#include "temp_ctrl.h"
#include <cmath> // для round
#ifdef USE_TEMP_CTRL
  extern float GetAverageTemp();
#endif

extern uint32_t CurrTimeInSecs();
static s16_t
info_get_value(const struct snmp_scalar_array_node_def *node, void *value)
{
  const char * var = NULL;
  s16_t var_len = 0, sresult;
  u16_t result;
  float cTemp = 0;
  char cMAC[20], mac[6];
  
  switch (node->oid) {
  case 1:    
    var     = rsettings->alias;
    break;
  case 2: 
    var     = GetInfo_fw();
    break;
  case 3: 
    var     = GetInfo_sw();
    break;
  case 4:    
    *(s32_t*)value = GetInfo_sID();
    return sizeof(s32_t);
    break;
  case 5: 
    *(u32_t*)value = CurrTimeInSecs();
//    MIB2_COPY_SYSUPTIME_TO((u32_t*)value);
    return sizeof(u32_t);
    break;  
  case 6: //Количество оставшихся блоков для записи sID, FW
    sresult = current_pos();
    *(s32_t*)value = 15 - sresult;
    return sizeof(s32_t);
    break;
  case 7: //MAC-адрес устройства
    memcpy(mac, &rsettings->mac, 6);
    sprintf( cMAC, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
    var = cMAC;
    break;
  case 8: //Порт связи с клиентом
    result = rsettings->HttpPort;
    *(s32_t*)value = result;
    return sizeof(s32_t);
  case 9: //Температура кристалла микроконтроллера
#ifdef USE_TEMP_CTRL
    cTemp = GetAverageTemp();
#endif
    result = round(cTemp);
    *(s32_t*)value = result;
    return sizeof(s32_t);
  default:
    LWIP_DEBUGF(SNMP_MIB_DEBUG,("system_get_value(): unknown id: %"S32_F"\n", node->oid));
    return 0;
  }
  
  LWIP_ASSERT("", (value != NULL));
  if (var_len == 0) {
    result = (s16_t)strlen((const char*)var);
  } else {
    result = var_len;
  }
  MEMCPY(value, var, result);
  return result;
}

#include "extio.h"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  extern uint8_t read_switch_status(int port, uint8_t reg);
#endif

#if (MKPSH10 != 0)
/* PORT5 SETTINGS */
static s16_t get_port5_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port5_auto(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].autonegotiation;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port5_auto(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_autonegotiation_t cVal = (val == 1)?SWITCH_AUTO_ENABLE:SWITCH_AUTO_DISABLE;
    if (cVal != rsettings->sw.ports[4].autonegotiation) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth5, snSpeedAuto, cVal);
      rsettings->sw.ports[4].autonegotiation = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port5_speed(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].speed;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port5_speed(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_speed_t cVal = (val == 1)?SWITCH_SPEED_100:SWITCH_SPEED_10;
    if (cVal != rsettings->sw.ports[4].speed) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth5, snSpeed, cVal);
      rsettings->sw.ports[4].speed = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port5_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port5_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[4].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth5, snDuplex, cVal);
      rsettings->sw.ports[4].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port5_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port5_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[4].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth5, snFlowCtrl, cVal);
      rsettings->sw.ports[4].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port5_mdi(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[4].mdi;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port5_mdi(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_mdi_t cVal = (val == 2)?SWITCH_MDIX:(val == 1)?SWITCH_MDI:SWITCH_AUTO;
    if (cVal != rsettings->sw.ports[4].mdi) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth5, snMdiMdx, cVal);
      rsettings->sw.ports[4].mdi = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port5_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(4,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}

/* PORT6 SETTINGS */
static s16_t get_port6_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port6_auto(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].autonegotiation;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port6_auto(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_autonegotiation_t cVal = (val == 1)?SWITCH_AUTO_ENABLE:SWITCH_AUTO_DISABLE;
    if (cVal != rsettings->sw.ports[5].autonegotiation) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth6, snSpeedAuto, cVal);
      rsettings->sw.ports[5].autonegotiation = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port6_speed(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].speed;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port6_speed(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_speed_t cVal = (val == 1)?SWITCH_SPEED_100:SWITCH_SPEED_10;
    if (cVal != rsettings->sw.ports[5].speed) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth6, snSpeed, cVal);
      rsettings->sw.ports[5].speed = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port6_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port6_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[5].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth6, snDuplex, cVal);
      rsettings->sw.ports[5].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port6_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port6_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[5].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth6, snFlowCtrl, cVal);
      rsettings->sw.ports[5].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port6_mdi(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[5].mdi;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port6_mdi(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_mdi_t cVal = (val == 2)?SWITCH_MDIX:(val == 1)?SWITCH_MDI:SWITCH_AUTO;
    if (cVal != rsettings->sw.ports[5].mdi) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth6, snMdiMdx, cVal);
      rsettings->sw.ports[5].mdi = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port6_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(5,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}

/* PORT7 SETTINGS */
static s16_t get_port7_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port7_auto(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].autonegotiation;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port7_auto(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_autonegotiation_t cVal = (val == 1)?SWITCH_AUTO_ENABLE:SWITCH_AUTO_DISABLE;
    if (cVal != rsettings->sw.ports[6].autonegotiation) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth7, snSpeedAuto, cVal);
      rsettings->sw.ports[6].autonegotiation = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port7_speed(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].speed;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port7_speed(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_speed_t cVal = (val == 1)?SWITCH_SPEED_100:SWITCH_SPEED_10;
    if (cVal != rsettings->sw.ports[6].speed) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth7, snSpeed, cVal);
      rsettings->sw.ports[6].speed = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port7_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port7_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[6].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth7, snDuplex, cVal);
      rsettings->sw.ports[6].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port7_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port7_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[6].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth7, snFlowCtrl, cVal);
      rsettings->sw.ports[6].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port7_mdi(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[6].mdi;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port7_mdi(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_mdi_t cVal = (val == 2)?SWITCH_MDIX:(val == 1)?SWITCH_MDI:SWITCH_AUTO;
    if (cVal != rsettings->sw.ports[6].mdi) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnEth7, snMdiMdx, cVal);
      rsettings->sw.ports[6].mdi = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port7_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(6,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}
#endif //MKPSH10

/* PORT1 SETIINGS */
static s16_t get_port1_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[0].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port1_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[0].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port1_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[0].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth1, snDuplex, cVal);
      rsettings->sw.ports[0].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port1_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[0].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port1_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[0].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth1, snFlowCtrl, cVal);
      rsettings->sw.ports[0].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
static s16_t get_port1_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(0,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}
#endif

/* PORT2 SETTINGS */
static s16_t get_port2_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[1].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port2_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[1].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port2_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[1].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth2, snDuplex, cVal);
      rsettings->sw.ports[1].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port2_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[1].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port2_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[1].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth2, snFlowCtrl, cVal);
      rsettings->sw.ports[1].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
static s16_t get_port2_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(1,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}
#endif

/* PORT3 SETTINGS */
static s16_t get_port3_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[2].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port3_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[2].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port3_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[2].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth3, snDuplex, cVal);
      rsettings->sw.ports[2].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port3_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[2].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port3_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[2].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth3, snFlowCtrl, cVal);
      rsettings->sw.ports[2].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
static s16_t get_port3_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(2,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}
#endif

/* PORT4 SETTINGS */
static s16_t get_port4_status(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[3].port_enabled;
    return sizeof(*uint_ptr);
}

static s16_t get_port4_duplex(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[3].full_duplex;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port4_duplex(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_duplex_t cVal = (val == 1)?SWITCH_FULL_DUPLEX:SWITCH_HALF_DUPLEX;
    if (cVal != rsettings->sw.ports[3].full_duplex) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth4, snDuplex, cVal);
      rsettings->sw.ports[3].full_duplex = cVal;
    }
    return SNMP_ERR_NOERROR;
}

static s16_t get_port4_flow(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = rsettings->sw.ports[3].flow_control;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_port4_flow(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    switch_flow_control_t cVal = (val == 1)?SWITCH_ENABLE_FLOW_CONTROL:SWITCH_DISABLE_FLOW_CONTROL;
    if (cVal != rsettings->sw.ports[3].flow_control) {
      SaveSettingEvent(etSetting, ecParamChangePort, pnF_Eth4, snFlowCtrl, cVal);
      rsettings->sw.ports[3].flow_control = cVal;
    }
    return SNMP_ERR_NOERROR;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
static s16_t get_port4_connect(struct snmp_node_instance* instance, void* value) {
    uint8_t status_sw = read_switch_status(3,1);//0xnE
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = ((status_sw >> 5) & 1);
    return sizeof(*uint_ptr);
}
#endif

/* SERVICE REBOOT */
static s16_t get_reboot(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = 0;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_reboot(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    if(val == 1)
    SaveEvent(0, etCount, ecCount); //Спец. событие: Переход на загрузчик
    return SNMP_ERR_NOERROR;
}

static s16_t get_savereboot(struct snmp_node_instance* instance, void* value) {
    u32_t *uint_ptr = (u32_t*) value;
    *uint_ptr = 0;
    return sizeof(*uint_ptr);
}

static snmp_err_t set_savereboot(struct snmp_node_instance* instance, u16_t len, void *value) {
    u32_t val = *((u32_t*)value);
    if(val == 1)
    settings_save();
    return SNMP_ERR_NOERROR;
}

#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_scalar_array_node_def statistics_port_nodes[] = {
  { 1, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 2, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 3, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 4, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 5, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},
  { 6, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 7, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 8, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  { 9, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {10, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {11, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {12, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {13, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {14, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {15, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {16, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY}, 
  {17, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {18, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {19, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {20, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {21, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {22, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},
  {23, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {24, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {25, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {26, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY}, 
  {27, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {28, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {29, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY}, 
  {30, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY}, 
  {31, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {32, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},  
  {33, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},
  {34, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},
  {35, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY},
  {36, SNMP_ASN1_TYPE_COUNTER64, SNMP_NODE_INSTANCE_READ_ONLY}
};
#endif

static const struct snmp_scalar_array_node_def my_info[] = {
  { 1, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY},              //devName
  { 2, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY},              //fwVer
  { 3, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY},              //swVer
  { 4, SNMP_ASN1_TYPE_UNSIGNED32, SNMP_NODE_INSTANCE_READ_ONLY},                //sID
  { 5, SNMP_ASN1_TYPE_UNSIGNED32, SNMP_NODE_INSTANCE_READ_ONLY},                 //upTime
  { 6, SNMP_ASN1_TYPE_UNSIGNED32, SNMP_NODE_INSTANCE_READ_ONLY},                //restHwCells
  { 7, SNMP_ASN1_TYPE_OCTET_STRING, SNMP_NODE_INSTANCE_READ_ONLY},              //devMACaddr
  { 8, SNMP_ASN1_TYPE_UNSIGNED32, SNMP_NODE_INSTANCE_READ_ONLY},                //portSrc
  { 9, SNMP_ASN1_TYPE_UNSIGNED32, SNMP_NODE_INSTANCE_READ_ONLY}                 //sysTemperature
};

//static const struct snmp_scalar_array_node_def settings_port_eth_nodes[] = {
//  { 1, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pStatus
//  { 2, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pAutoNeg        
//  { 3, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pSpeed        
//  { 4, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pDuplex
//  { 5, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pFlowCtrl
//  { 6, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE}                  //pAutoMDIX
//};
//
//static const struct snmp_scalar_array_node_def settings_port_feth_nodes[] = {
//  { 1, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pStatus         
//  { 2, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},                 //pDuplex
//  { 3, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE}                  //pFlowCtrl  
//};
//
//static const struct snmp_scalar_array_node_def my_service[] = {
//  { 1, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE},              //sRestart
//  { 2, SNMP_ASN1_TYPE_INTEGER, SNMP_NODE_INSTANCE_READ_WRITE}               //sSaveRestart  
//};

const struct snmp_scalar_array_node info_mib2_root = SNMP_SCALAR_CREATE_ARRAY_NODE(1, my_info, info_get_value, NULL, NULL);

#ifdef USE_SWITCH //Использовать свитч kmz8895
const struct snmp_scalar_array_node statistics_port1 = SNMP_SCALAR_CREATE_ARRAY_NODE(1, statistics_port_nodes, statistics_port1_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port2 = SNMP_SCALAR_CREATE_ARRAY_NODE(2, statistics_port_nodes, statistics_port2_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port3 = SNMP_SCALAR_CREATE_ARRAY_NODE(3, statistics_port_nodes, statistics_port3_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port4 = SNMP_SCALAR_CREATE_ARRAY_NODE(4, statistics_port_nodes, statistics_port4_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port5 = SNMP_SCALAR_CREATE_ARRAY_NODE(5, statistics_port_nodes, statistics_port5_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port6 = SNMP_SCALAR_CREATE_ARRAY_NODE(6, statistics_port_nodes, statistics_port6_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port7 = SNMP_SCALAR_CREATE_ARRAY_NODE(7, statistics_port_nodes, statistics_port7_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port8 = SNMP_SCALAR_CREATE_ARRAY_NODE(8, statistics_port_nodes, statistics_port8_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port9 = SNMP_SCALAR_CREATE_ARRAY_NODE(9, statistics_port_nodes, statistics_port9_get_value, NULL, NULL);
const struct snmp_scalar_array_node statistics_port10 = SNMP_SCALAR_CREATE_ARRAY_NODE(10, statistics_port_nodes, statistics_port10_get_value, NULL, NULL);
#endif

#if (MKPSH10 != 0)
static const struct snmp_scalar_node settings_port5_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_status, NULL, NULL);
static const struct snmp_scalar_node settings_port5_auto = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_auto, NULL, set_port5_auto);
static const struct snmp_scalar_node settings_port5_speed = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_speed, NULL, set_port5_speed);
static const struct snmp_scalar_node settings_port5_duplex = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_duplex, NULL, set_port5_duplex);
static const struct snmp_scalar_node settings_port5_flow = SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_flow, NULL, set_port5_flow);
static const struct snmp_scalar_node settings_port5_mdi = SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_mdi, NULL, set_port5_mdi);
static const struct snmp_scalar_node settings_port5_connect = SNMP_SCALAR_CREATE_NODE(7, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port5_connect, NULL, NULL);

static const struct snmp_scalar_node settings_port6_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_status, NULL, NULL);
static const struct snmp_scalar_node settings_port6_auto = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_auto, NULL, set_port6_auto);
static const struct snmp_scalar_node settings_port6_speed = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_speed, NULL, set_port6_speed);
static const struct snmp_scalar_node settings_port6_duplex = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_duplex, NULL, set_port6_duplex);
static const struct snmp_scalar_node settings_port6_flow = SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_flow, NULL, set_port6_flow);
static const struct snmp_scalar_node settings_port6_mdi = SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_mdi, NULL, set_port6_mdi);
static const struct snmp_scalar_node settings_port6_connect = SNMP_SCALAR_CREATE_NODE(7, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port6_connect, NULL, NULL);

static const struct snmp_scalar_node settings_port7_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_status, NULL, NULL);
static const struct snmp_scalar_node settings_port7_auto = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_auto, NULL, set_port7_auto);
static const struct snmp_scalar_node settings_port7_speed = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_speed, NULL, set_port7_speed);
static const struct snmp_scalar_node settings_port7_duplex = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_duplex, NULL, set_port7_duplex);
static const struct snmp_scalar_node settings_port7_flow = SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_flow, NULL, set_port7_flow);
static const struct snmp_scalar_node settings_port7_mdi = SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_mdi, NULL, set_port7_mdi);
static const struct snmp_scalar_node settings_port7_connect = SNMP_SCALAR_CREATE_NODE(7, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port7_connect, NULL, NULL);
#endif

static const struct snmp_scalar_node settings_port1_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port1_status, NULL, NULL);
static const struct snmp_scalar_node settings_port1_duplex = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port1_duplex, NULL, set_port1_duplex);
static const struct snmp_scalar_node settings_port1_flow = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port1_flow, NULL, set_port1_flow);
#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_scalar_node settings_port1_connect = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port1_connect, NULL, NULL);
#endif

static const struct snmp_scalar_node settings_port2_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port2_status, NULL, NULL);
static const struct snmp_scalar_node settings_port2_duplex = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port2_duplex, NULL, set_port2_duplex);
static const struct snmp_scalar_node settings_port2_flow = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port2_flow, NULL, set_port2_flow);
#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_scalar_node settings_port2_connect = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port2_connect, NULL, NULL);
#endif

static const struct snmp_scalar_node settings_port3_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port3_status, NULL, NULL);
static const struct snmp_scalar_node settings_port3_duplex = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port3_duplex, NULL, set_port3_duplex);
static const struct snmp_scalar_node settings_port3_flow = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port3_flow, NULL, set_port3_flow);
#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_scalar_node settings_port3_connect = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port3_connect, NULL, NULL);
#endif

static const struct snmp_scalar_node settings_port4_status = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port4_status, NULL, NULL);
static const struct snmp_scalar_node settings_port4_duplex = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port4_duplex, NULL, set_port4_duplex);
static const struct snmp_scalar_node settings_port4_flow = SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_port4_flow, NULL, set_port4_flow);
#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_scalar_node settings_port4_connect = SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY, SNMP_ASN1_TYPE_UNSIGNED32, get_port4_connect, NULL, NULL);
#endif

static const struct snmp_scalar_node service_reboot = SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_reboot, NULL, set_reboot);
static const struct snmp_scalar_node service_savereboot = SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32, get_savereboot, NULL, set_savereboot);

#if (MKPSH10 != 0)
static const struct snmp_node* const settings_port5_node[] = {
  &settings_port5_status.node.node,
  &settings_port5_auto.node.node,
  &settings_port5_speed.node.node,
  &settings_port5_duplex.node.node,
  &settings_port5_flow.node.node,
  &settings_port5_mdi.node.node,
  &settings_port5_connect.node.node
};

static const struct snmp_node* const settings_port6_node[] = {
  &settings_port6_status.node.node,
  &settings_port6_auto.node.node,
  &settings_port6_speed.node.node,
  &settings_port6_duplex.node.node,
  &settings_port6_flow.node.node,
  &settings_port6_mdi.node.node,
  &settings_port6_connect.node.node
};

static const struct snmp_node* const settings_port7_node[] = {
  &settings_port7_status.node.node,
  &settings_port7_auto.node.node,
  &settings_port7_speed.node.node,
  &settings_port7_duplex.node.node,
  &settings_port7_flow.node.node,
  &settings_port7_mdi.node.node,
  &settings_port7_connect.node.node
};
#endif

static const struct snmp_node* const settings_port1_node[] = {
  &settings_port1_status.node.node,
  &settings_port1_duplex.node.node,
  &settings_port1_flow.node.node,
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &settings_port1_connect.node.node
#endif
};

static const struct snmp_node* const settings_port2_node[] = {
  &settings_port2_status.node.node,
  &settings_port2_duplex.node.node,
  &settings_port2_flow.node.node,
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &settings_port2_connect.node.node
#endif
};

static const struct snmp_node* const settings_port3_node[] = {
  &settings_port3_status.node.node,
  &settings_port3_duplex.node.node,
  &settings_port3_flow.node.node,
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &settings_port3_connect.node.node
#endif
};

static const struct snmp_node* const settings_port4_node[] = {
  &settings_port4_status.node.node,
  &settings_port4_duplex.node.node,
  &settings_port4_flow.node.node,
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &settings_port4_connect.node.node
#endif
};

#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_node* const my_statistics_nodes[] = {
  &statistics_port1.node.node,
  &statistics_port2.node.node,
  &statistics_port3.node.node,
  &statistics_port4.node.node,
  &statistics_port5.node.node,
  &statistics_port6.node.node,
  &statistics_port7.node.node,
  &statistics_port8.node.node,
  &statistics_port9.node.node,
  &statistics_port10.node.node
};
#endif

static const struct snmp_tree_node settings_port1 = SNMP_CREATE_TREE_NODE(1, settings_port1_node);
static const struct snmp_tree_node settings_port2 = SNMP_CREATE_TREE_NODE(2, settings_port2_node);
static const struct snmp_tree_node settings_port3 = SNMP_CREATE_TREE_NODE(3, settings_port3_node);
static const struct snmp_tree_node settings_port4 = SNMP_CREATE_TREE_NODE(4, settings_port4_node);
#if (MKPSH10 != 0)
static const struct snmp_tree_node settings_port5 = SNMP_CREATE_TREE_NODE(5, settings_port5_node);
static const struct snmp_tree_node settings_port6 = SNMP_CREATE_TREE_NODE(6, settings_port6_node);
static const struct snmp_tree_node settings_port7 = SNMP_CREATE_TREE_NODE(7, settings_port7_node);
#endif

static const struct snmp_node* const my_settings_nodes[] = {
  &settings_port1.node,
  &settings_port2.node,
  &settings_port3.node,
  &settings_port4.node,
#if (MKPSH10 != 0)
  &settings_port5.node,
  &settings_port6.node,
  &settings_port7.node
#endif
};

static const struct snmp_node* const my_service_nodes[] = {
  &service_reboot.node.node,
  &service_savereboot.node.node    
};

#ifdef USE_SWITCH //Использовать свитч kmz8895
static const struct snmp_tree_node statistics_mib2_root = SNMP_CREATE_TREE_NODE(2, my_statistics_nodes);
#endif
static const struct snmp_tree_node settings_mib2_root = SNMP_CREATE_TREE_NODE(3, my_settings_nodes);
static const struct snmp_tree_node service_mib2_root = SNMP_CREATE_TREE_NODE(4, my_service_nodes);

#ifdef USE_STP
 #include "stp_mib2.c"
#endif
#include "power_mib2.c"
#include "rs485_mib2.c"
#ifdef USE_SWITCH //Использовать свитч kmz8895
  #include "dmac_mib2.c"
#endif

static const struct snmp_node* const my_mib2_nodes[] = {
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &statistics_mib2_root.node,
#endif
  &info_mib2_root.node.node,
  &settings_mib2_root.node,
  &service_mib2_root.node,
#ifdef USE_STP
  &rstp_mib2_root.node,
#endif
  &power_mib2_root.node,
  &rs485_mib2_root.node,
#ifdef USE_SWITCH //Использовать свитч kmz8895
  &Dmac_mib2_root.node,
#endif
  &critical_mib2_root.node
};

// ID устройства в SNMP
// !!! Изменения в этих определениях должны быть согласованы
#if (MKPSH10 != 0)
  #define SNMP_DEVICE_ID 1
#endif
#if (IMC_FTX_MC != 0)
  #define SNMP_DEVICE_ID 2
#endif
#if (UTD_M != 0)
  #define SNMP_DEVICE_ID 51
#endif
#if (IIP != 0)
  #define SNMP_DEVICE_ID 52
#endif

static const struct snmp_tree_node my_mib2_root = SNMP_CREATE_TREE_NODE(1, my_mib2_nodes);
static const u32_t  my_mib2_base_oid_arr[] =
  { 1, 3, 6, 1, 4, 1, SNMP_ENTERPRISE_TRANSMASH_ID, SNMP_DEVICE_ID };
const struct snmp_mib my_mib2 = SNMP_MIB_CREATE(my_mib2_base_oid_arr, &my_mib2_root.node);