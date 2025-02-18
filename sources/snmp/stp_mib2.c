//******************************************************************************
// Имя файла    :       stp_mib2.c
// Заголовок    :       
// Автор        :       Сосновских А.А.
// Дата         :       29.11.2018
//
//------------------------------------------------------------------------------
/**
База данных MIB2 параметров RSTP по IEEE 802.1d

**/

#include "stp_mib2.h"

static s16_t getSettingsBridge(struct snmp_node_instance * instance, void* value) {
#ifdef SNMP_ENABLE_GET_VALUES
  u32_t * uint_ptr = (u32_t*) value;
  u8_t * u8_ptr = (u8_t*) value;
  struct sRstpBridgeDesc * cpRstpBridgeDesc = gaRstpBridgeDesc;
  switch (instance->node->oid) {
  case 1:
    *uint_ptr = cpRstpBridgeDesc[0].Priority;
    break;
  case 2:
    memcpy(u8_ptr, rsettings->aRstpBridgeDesc[0].MAC, ETH_HWADDR_LEN);
    return ETH_HWADDR_LEN;
  case 3:
    *uint_ptr = rsettings->aRstpBridgeDesc[0].FirmId;
    break;
  case 4:
    *uint_ptr = rsettings->aRstpBridgeDesc[0].MaxAge;
    break;
  case 5:
    *uint_ptr = rsettings->aRstpBridgeDesc[0].HelloTime;
    break;
  case 6:
    *uint_ptr = rsettings->aRstpBridgeDesc[0].ForwardDelay;
    break;
    
  case 7:
    *uint_ptr = gaRstpBridge[0].BridgeRole;
    break;
  case 8:
    *uint_ptr = gaRstpBridge[0].RootPortIdx;
    break;
  case 9:
    memcpy(u8_ptr, gaRstpBridge[0].RootPriority.RootBridgeID.MAC.addr, ETH_HWADDR_LEN);
    return ETH_HWADDR_LEN;
  default:
    return 0;
  }
  return sizeof(*uint_ptr);
#else
  return 0;
#endif
}

int memucmp(uint8_t * iSrc, uint8_t * iDst, int iSize);
void CopyRstpCfg(struct sRstpBridgeDesc * ipRstpBridgeDescSrc, struct sRstpBridgeDesc * ipRstpBridgeDescDst);

static snmp_err_t setSettingsBridge(struct snmp_node_instance* instance, u16_t len, void *value) {
#ifdef SNMP_ENABLE_SETTINGS
  u32_t val = *((u32_t*)value);
  u8_t * u8_ptr = (u8_t*) value;
  bool cChanged = false;
  struct sRstpBridgeDesc * cpRstpBridgeDesc = gaRstpBridgeDesc;
  switch (instance->node->oid) {
  case 1:
    if (val <= 15) {
      if (val != cpRstpBridgeDesc[0].Priority) {
        cpRstpBridgeDesc[0].Priority = val;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
  case 2:
    if (memucmp(cpRstpBridgeDesc[0].MAC, u8_ptr, ETH_HWADDR_LEN) != 0) {
      memcpy(cpRstpBridgeDesc[0].MAC, u8_ptr, ETH_HWADDR_LEN);
      cChanged = true;
    }
    break;
 #ifdef SNMP_ENABLE_SETTINGS_FULL      
  case 3:
    if (val <= 4095) {
      if (val != cpRstpBridgeDesc[0].FirmId) {
        cpRstpBridgeDesc[0].FirmId = val;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
  case 4:
    if ((val >= 6) && (val <= 40)) {
      if (val != cpRstpBridgeDesc[0].MaxAge) {
        cpRstpBridgeDesc[0].MaxAge = val;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
  case 5:
    if ((val >= 1) && (val <= 2)) {
      if (val != cpRstpBridgeDesc[0].HelloTime) {
        cpRstpBridgeDesc[0].HelloTime = val;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
  case 6:
    if ((val >= 4) && (val <= 30)) {
      if (val != cpRstpBridgeDesc[0].ForwardDelay) {
        cpRstpBridgeDesc[0].ForwardDelay = val;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
 #else
  case 3:
  case 4:
  case 5:
  case 6:
    return SNMP_ERR_NOACCESS;
 #endif
  case 7:
  case 8:
  case 9:
    return SNMP_ERR_NOTWRITABLE;
  case 10:
    //Запись во Flash
    CopyRstpCfg(gaRstpBridgeDesc, rsettings->aRstpBridgeDesc);
    settings_save();
//    SetRstpBlock();
    return SNMP_ERR_NOERROR;
  default:
    return SNMP_ERR_NOSUCHINSTANCE;
  }
  if (cChanged) {
    //
    return SNMP_ERR_NOERROR;
  }
  return SNMP_ERR_NOERROR;
#else
  return SNMP_ERR_NOACCESS;
#endif
}


static s16_t getSettingsPort(struct snmp_node_instance * instance, void* value, int portNum) {
#ifdef SNMP_ENABLE_GET_VALUES
  u32_t * uint_ptr = (u32_t*) value;
  u8_t * u8_ptr = (u8_t*) value;
  struct sBPort * cpPort = &gaRstpBridge[0].aPort[portNum - 1];
  struct sBPortDesc * cpPortDesc = &gaRstpBridgeDesc[0].aBPortDesc[portNum - 1];
  switch (instance->node->oid) {
  case 1:
    *uint_ptr = cpPortDesc->portEnabled;
    break;
  case 2:
    *uint_ptr = cpPortDesc->adminEdge;
    break;
  case 3:
    *uint_ptr = cpPortDesc->autoEdge;
    break;
  case 4:
    *uint_ptr = cpPortDesc->ForcePriority;
    break;
  case 5:
    *uint_ptr = cpPortDesc->MigrateTime;
    break;
  case 6:
    *uint_ptr = cpPortDesc->PortPathCost;
    break;
  //ReadOnly
  case 7:
    *uint_ptr = cpPort->prtState;
    break;
  case 8:
    *uint_ptr = cpPort->pstState;
    break;
  case 9:
    *uint_ptr = cpPort->prState;
    break;
  case 10:
    *uint_ptr = (cpPort->msgPriority.DesignatedBridgeID.Priority >> 4) & 0x0f;
    break;
  case 11:
    memcpy(u8_ptr, cpPort->msgPriority.DesignatedBridgeID.MAC.addr, ETH_HWADDR_LEN);
    return ETH_HWADDR_LEN;
  case 12:
    *uint_ptr = cpPort->pstState;
    break;
  case 13:
    *uint_ptr = cpPort->msgPriority.RootPathCost;
    break;
  case 14:
    *uint_ptr = cpPort->allSynced;
    break;
  default:
    break;
  }
  return sizeof(*uint_ptr);
#else
  return 0;
#endif
}

static s16_t getSettingsPort1(struct snmp_node_instance * instance, void* value) {
  return getSettingsPort(instance, value, 1);
}

static s16_t getSettingsPort2(struct snmp_node_instance * instance, void* value) {
  return getSettingsPort(instance, value, 2);
}

static s16_t getSettingsPort3(struct snmp_node_instance * instance, void* value) {
  return getSettingsPort(instance, value, 3);
}

static s16_t getSettingsPort4(struct snmp_node_instance * instance, void* value) {
  return getSettingsPort(instance, value, 4);
}

static snmp_err_t setSettingsPort(struct snmp_node_instance* instance, u16_t len, void *value, int portNum) {
#ifdef SNMP_ENABLE_SETTINGS
  u32_t * uint_ptr = (u32_t*) value;
  uint32_t cValue = *uint_ptr;
  bool cChanged = false;
  struct sBPortDesc * cpPortDesc = &gaRstpBridgeDesc[0].aBPortDesc[portNum - 1];
  switch (instance->node->oid) {
  case 1:
    cValue &= 0x01;
    if ((bool)cValue != cpPortDesc->portEnabled) {
      cpPortDesc->portEnabled = cValue;
      cChanged = true;
    }
    break;
  case 2:
    cValue &= 0x01;
    if ((bool)cValue != cpPortDesc->adminEdge) {
      cpPortDesc->adminEdge = cValue;
      cChanged = true;
    }
    break;
  case 3:
    cValue &= 0x01;
    if ((bool)cValue != cpPortDesc->autoEdge) {
      cpPortDesc->autoEdge = cValue;
      cChanged = true;
    }
    break;
 #ifdef SNMP_ENABLE_SETTINGS_FULL      
  case 4:
    if (cValue <= 15) {
      if (cValue != cpPortDesc->ForcePriority) {
        cpPortDesc->ForcePriority = cValue;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
  case 6:
    if ((cValue >= 20000) && (cValue <= 2000000)) {
      if (cValue != cpPortDesc->PortPathCost) {
        cpPortDesc->PortPathCost = cValue;
        cChanged = true;
      }
    } else {
      return SNMP_ERR_WRONGVALUE;
    }
    break;
 #else
  case 4:
  case 6:
    return SNMP_ERR_NOACCESS;
 #endif
  case 5:
  case 7:
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
    return SNMP_ERR_NOTWRITABLE;
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

static snmp_err_t setSettingsPort1(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setSettingsPort(instance, len, value, 1);
}

static snmp_err_t setSettingsPort2(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setSettingsPort(instance, len, value, 2);
}

static snmp_err_t setSettingsPort3(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setSettingsPort(instance, len, value, 3);
}

static snmp_err_t setSettingsPort4(struct snmp_node_instance* instance, u16_t len, void *value) {
  return setSettingsPort(instance, len, value, 4);
}

//Мост
static const struct snmp_scalar_node aSettingsBridge[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, setSettingsBridge),
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_OCTET_STRING, getSettingsBridge, NULL, setSettingsBridge),
  SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, setSettingsBridge),
  SNMP_SCALAR_CREATE_NODE(4, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, setSettingsBridge),
  SNMP_SCALAR_CREATE_NODE(5, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, setSettingsBridge),
  SNMP_SCALAR_CREATE_NODE(6, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, setSettingsBridge),
  
  SNMP_SCALAR_CREATE_NODE(7, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(8, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsBridge, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(9, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getSettingsBridge, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(10, SNMP_NODE_INSTANCE_WRITE_ONLY, SNMP_ASN1_TYPE_UNSIGNED32 , NULL, NULL, setSettingsBridge)
};

static const struct snmp_node* const settings_bridge_node[] = {
  &aSettingsBridge[0].node.node,
  &aSettingsBridge[1].node.node,
  &aSettingsBridge[2].node.node,
  &aSettingsBridge[3].node.node,
  &aSettingsBridge[4].node.node,
  &aSettingsBridge[5].node.node,
  
  &aSettingsBridge[6].node.node,
  &aSettingsBridge[7].node.node,
  &aSettingsBridge[8].node.node,
  &aSettingsBridge[9].node.node
};

//Порты 1 .. 4
static const struct snmp_scalar_node aSettingsPort1[] = {
  SNMP_SCALAR_CREATE_NODE( 1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  SNMP_SCALAR_CREATE_NODE( 2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  SNMP_SCALAR_CREATE_NODE( 3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  SNMP_SCALAR_CREATE_NODE( 4, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  SNMP_SCALAR_CREATE_NODE( 5, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  SNMP_SCALAR_CREATE_NODE( 6, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, setSettingsPort1),
  
  SNMP_SCALAR_CREATE_NODE( 7, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 8, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 9, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(10, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(11, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(12, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(13, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(14, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort1, NULL, NULL)
};

static const struct snmp_node* const settings_rstp_port1_node[] = {
  &aSettingsPort1[ 0].node.node,
  &aSettingsPort1[ 1].node.node,
  &aSettingsPort1[ 2].node.node,
  &aSettingsPort1[ 3].node.node,
  &aSettingsPort1[ 4].node.node,
  &aSettingsPort1[ 5].node.node,
  
  &aSettingsPort1[ 6].node.node,
  &aSettingsPort1[ 7].node.node,
  &aSettingsPort1[ 8].node.node,
  &aSettingsPort1[ 9].node.node,
  &aSettingsPort1[10].node.node,
  &aSettingsPort1[11].node.node,
  &aSettingsPort1[12].node.node,
  &aSettingsPort1[13].node.node
};

static const struct snmp_scalar_node aSettingsPort2[] = {
  SNMP_SCALAR_CREATE_NODE( 1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  SNMP_SCALAR_CREATE_NODE( 2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  SNMP_SCALAR_CREATE_NODE( 3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  SNMP_SCALAR_CREATE_NODE( 4, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  SNMP_SCALAR_CREATE_NODE( 5, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  SNMP_SCALAR_CREATE_NODE( 6, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, setSettingsPort2),
  
  SNMP_SCALAR_CREATE_NODE( 7, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 8, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 9, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(10, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(11, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(12, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(13, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(14, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort2, NULL, NULL)
};

static const struct snmp_node* const settings_rstp_port2_node[] = {
  &aSettingsPort2[ 0].node.node,
  &aSettingsPort2[ 1].node.node,
  &aSettingsPort2[ 2].node.node,
  &aSettingsPort2[ 3].node.node,
  &aSettingsPort2[ 4].node.node,
  &aSettingsPort2[ 5].node.node,
  
  &aSettingsPort2[ 6].node.node,
  &aSettingsPort2[ 7].node.node,
  &aSettingsPort2[ 8].node.node,
  &aSettingsPort2[ 9].node.node,
  &aSettingsPort2[10].node.node,
  &aSettingsPort2[11].node.node,
  &aSettingsPort2[12].node.node,
  &aSettingsPort2[13].node.node
};

static const struct snmp_scalar_node aSettingsPort3[] = {
  SNMP_SCALAR_CREATE_NODE( 1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  SNMP_SCALAR_CREATE_NODE( 2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  SNMP_SCALAR_CREATE_NODE( 3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  SNMP_SCALAR_CREATE_NODE( 4, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  SNMP_SCALAR_CREATE_NODE( 5, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  SNMP_SCALAR_CREATE_NODE( 6, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, setSettingsPort3),
  
  SNMP_SCALAR_CREATE_NODE( 7, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 8, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 9, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(10, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(11, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(12, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(13, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(14, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort3, NULL, NULL)
};

static const struct snmp_node* const settings_rstp_port3_node[] = {
  &aSettingsPort3[ 0].node.node,
  &aSettingsPort3[ 1].node.node,
  &aSettingsPort3[ 2].node.node,
  &aSettingsPort3[ 3].node.node,
  &aSettingsPort3[ 4].node.node,
  &aSettingsPort3[ 5].node.node,
  
  &aSettingsPort3[ 6].node.node,
  &aSettingsPort3[ 7].node.node,
  &aSettingsPort3[ 8].node.node,
  &aSettingsPort3[ 9].node.node,
  &aSettingsPort3[10].node.node,
  &aSettingsPort3[11].node.node,
  &aSettingsPort3[12].node.node,
  &aSettingsPort3[13].node.node
};

static const struct snmp_scalar_node aSettingsPort4[] = {
  SNMP_SCALAR_CREATE_NODE( 1, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  SNMP_SCALAR_CREATE_NODE( 2, SNMP_NODE_INSTANCE_READ_WRITE, SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  SNMP_SCALAR_CREATE_NODE( 3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  
  SNMP_SCALAR_CREATE_NODE( 4, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  SNMP_SCALAR_CREATE_NODE( 5, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  SNMP_SCALAR_CREATE_NODE( 6, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, setSettingsPort4),
  
  SNMP_SCALAR_CREATE_NODE( 7, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 8, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE( 9, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(10, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(11, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(12, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(13, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL),
  SNMP_SCALAR_CREATE_NODE(14, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_UNSIGNED32  , getSettingsPort4, NULL, NULL)
};

static const struct snmp_node* const settings_rstp_port4_node[] = {
  &aSettingsPort4[ 0].node.node,
  &aSettingsPort4[ 1].node.node,
  &aSettingsPort4[ 2].node.node,
  &aSettingsPort4[ 3].node.node,
  &aSettingsPort4[ 4].node.node,
  &aSettingsPort4[ 5].node.node,
  
  &aSettingsPort4[ 6].node.node,
  &aSettingsPort4[ 7].node.node,
  &aSettingsPort4[ 8].node.node,
  &aSettingsPort4[ 9].node.node,
  &aSettingsPort4[10].node.node,
  &aSettingsPort4[11].node.node,
  &aSettingsPort4[12].node.node,
  &aSettingsPort4[13].node.node
};


static const struct snmp_tree_node settings_rstp_port1 = SNMP_CREATE_TREE_NODE(1, settings_rstp_port1_node);
static const struct snmp_tree_node settings_rstp_port2 = SNMP_CREATE_TREE_NODE(2, settings_rstp_port2_node);
static const struct snmp_tree_node settings_rstp_port3 = SNMP_CREATE_TREE_NODE(3, settings_rstp_port3_node);
static const struct snmp_tree_node settings_rstp_port4 = SNMP_CREATE_TREE_NODE(4, settings_rstp_port4_node);

static const struct snmp_node* const settings_ports_node[] = {
  &settings_rstp_port1.node,
  &settings_rstp_port2.node,
  &settings_rstp_port3.node,
  &settings_rstp_port4.node,
};

static const struct snmp_tree_node rstp_bridge = SNMP_CREATE_TREE_NODE(1, settings_bridge_node);
static const struct snmp_tree_node rstp_ports = SNMP_CREATE_TREE_NODE(2, settings_ports_node);


static const struct snmp_node* const rstp_nodes[] = {
  &rstp_bridge.node,
  &rstp_ports.node
};

static const struct snmp_tree_node rstp_mib2_root = SNMP_CREATE_TREE_NODE(5, rstp_nodes);


