//******************************************************************************
// ��� �����    :       dmac_mib2.c
// ���������    :       
// �����        :       ���������� �.�.
// ����         :       16.08.2019
//
//------------------------------------------------------------------------------
/**
���� ������ MIB2 ��� �������� ������� DMAC ���� / IMC-FTX

**/

#include "dmac_mib2.h"

#if (MKPSH10 != 0)

#include "sdram.h"
#define DMAC_TABLE_SIZE (1024)
SDmacItem * gpDmacTable = (SDmacItem *)CODE_ADDRESS_UPL;
SDmacItem * lpDmacTable = (SDmacItem *)CODE_ADDRESS_DMAC;
#else

#include "extio.h"
#define DMAC_TABLE_SIZE (512)
SDmacItem  gDmacTable[DMAC_TABLE_SIZE];
SDmacItem * gpDmacTable = gDmacTable;
SDmacItem  lDmacTable[DMAC_TABLE_SIZE];
SDmacItem * lpDmacTable = lDmacTable;
#endif
int gDmacTableSize = 0;
uint32_t gDmacRefreshTime = 0;
int lDmacTableSize = 0;
int gReadDmacItemStart = 0, gDmacReadItemNum = 0;

#include "string.h"
extern ksz8895fmq_read_dmac_t dmac_table[][1000];
extern int dmac_table_size[];
extern int SelectDevPort(uint8_t iSwitch, uint8_t iPort);

#ifdef TWO_SWITCH_ITEMS
int8_t gSwitchNum = 2;
#else
int8_t gSwitchNum = 1;
#endif
//������������ ���������� �������, ����������� �� ���� ���� ������
#define DMAC_ITEM_NUM_MAX  (30)

//"����������" ������ � DMAC ��� �������� ������ (�� 30 ������� max)
//#define DMAC_READ_TEST

int ReadDmacTableSize() {
  int cDmacTableSize = 0;
  ksz8895fmq_read_dmac_t * cp_ksz8895fmq_read_dmac;
  for (int n = 0; n < gSwitchNum; ++n) {
    for (int i = 0; i < dmac_table_size[n]; ++i) {
      cp_ksz8895fmq_read_dmac = &dmac_table[n][i];
      if (cp_ksz8895fmq_read_dmac->port != 4) {
#ifdef TWO_SWITCH_ITEMS
        if ((n == 0) && (cp_ksz8895fmq_read_dmac->port == 0)) {
          continue; //
        }
#endif
        //������������� ���� ������ � ���� ����
        int cPort = SelectDevPort(n, cp_ksz8895fmq_read_dmac->port);
        if (cPort >= 0) {
          ++cDmacTableSize;
        }
      }
    }
  }
  return cDmacTableSize;
}

extern void settings_dmac_table_update();
extern uint8_t cEmptyMAC[ETH_HWADDR_LEN];

void ReadDmacTableTo(SDmacItem * ipDmacItem, int * ipDmacTableSize) {
  //��������� ������� DMAC �� ������� SDRAM
  SDmacItem * cpDmacItemCurr;
  int cDmacTableSize = 0;
  cDmacTableSize = 0;
  ksz8895fmq_read_dmac_t * cp_ksz8895fmq_read_dmac;
  for (int n = 0; n < gSwitchNum; ++n) {
    for (int i = 0; i < dmac_table_size[n]; ++i) {
      cp_ksz8895fmq_read_dmac = &dmac_table[n][i];
      cpDmacItemCurr = &ipDmacItem[cDmacTableSize];
      if (cp_ksz8895fmq_read_dmac->port != 4) {
#ifdef TWO_SWITCH_ITEMS
        if ((n == 0) && (cp_ksz8895fmq_read_dmac->port == 0)) {
          continue; //
        }
#endif
        //������������� ���� ������ � ���� ����
        int cPort = SelectDevPort(n, cp_ksz8895fmq_read_dmac->port);
        if (cPort >= 0) {
          if (memcmp(cp_ksz8895fmq_read_dmac->mac, cEmptyMAC, ETH_HWADDR_LEN) != 0) {
            //������ MAC ����������
            memcpy(cpDmacItemCurr->MAC, cp_ksz8895fmq_read_dmac->mac, ETH_HWADDR_LEN);
            cpDmacItemCurr->Port = cPort + 1;
            cpDmacItemCurr->VlanId = 0;
            ++cDmacTableSize;
          }
        }
      }
    }
  }
#ifdef DMAC_READ_TEST
#define DMAC_TEST_ITEM_NUM  DMAC_ITEM_NUM_MAX
  //"����������" ������ � DMAC ��� �������� ������ (�� 30 ������� max)
  for (int i = 0; i < DMAC_TEST_ITEM_NUM; ++i) {
    cpDmacItemCurr = &ipDmacItem[cDmacTableSize + i];
    memcpy((void *)cpDmacItemCurr, (void *)&ipDmacItem[0], sizeof(SDmacItem));
    ++cDmacTableSize;
  }
#endif
  *ipDmacTableSize = cDmacTableSize;
}

extern u32_t sys_now(void);
void ReadDmacTable() {
  //��������� ������� DMAC �� ������� SDRAM
  settings_dmac_table_update();
  if (gDmacTableSize == 0) {
    if (ReadDmacTableSize() == 0) {
      return;
    }
  } else {
    if (ReadDmacTableSize() < gDmacTableSize) { //
      return;
    }
  }
  ReadDmacTableTo(gpDmacTable, &gDmacTableSize);
  gDmacRefreshTime = sys_now();
}

//static s16_t getDmac(const struct snmp_scalar_array_node_def *node, void *value) {
static s16_t getDmac(struct snmp_node_instance * instance, void* value) {
  u32_t * uint_ptr = (u32_t*) value;
  const char * var = NULL;
  s16_t var_len = 0;
  switch (instance->node->oid) {
  case 1: //���������� ������� � �������� DMAC
    //��������� ������� � gDmacTable
    //��������� ���������� ������� � ��������
    ReadDmacTable();
    *uint_ptr = gDmacTableSize;
    break;
  case 3: //OctetString = 
    //��������� ��������� ������
    if ((gReadDmacItemStart < 0) || (gReadDmacItemStart >= gDmacTableSize)) {
      return 0;
    }
    //��������� ���������� ������� ��� ��������
    if ((gReadDmacItemStart + gDmacReadItemNum) > gDmacTableSize) {
      gDmacReadItemNum = gDmacTableSize - gReadDmacItemStart;
    }
    if (gDmacReadItemNum > 0) {
      var = (char *)&gpDmacTable[gReadDmacItemStart];
      var_len = gDmacReadItemNum * sizeof(SDmacItem);
      memcpy(value, var, var_len);
    }
    return var_len;
  default:
    return 0;
  }
  return sizeof(*uint_ptr);
}

//��� ������ ��������� ������� � ���������� ��� ������
//static snmp_err_t setDmac(const struct snmp_scalar_array_node_def* node, u16_t len, void *value) {
static snmp_err_t setDmac(struct snmp_node_instance* instance, u16_t len, void *value) {
  u32_t * uint_ptr = (u32_t*) value;
  uint32_t cValue = *uint_ptr;
  uint16_t cStart = cValue & 0xffff;
  uint16_t cSize = (cValue >> 16) & 0xff;
  snmp_err_t cError = SNMP_ERR_NOERROR;
  switch (instance->node->oid) {
  case 2: //
    if ((cStart >= gDmacTableSize)) {
      cError = SNMP_ERR_WRONGLENGTH;
    } else {
      gReadDmacItemStart = cStart;
      if (cSize > DMAC_ITEM_NUM_MAX)
        cSize = DMAC_ITEM_NUM_MAX;
      gDmacReadItemNum = cSize;
      cError = SNMP_ERR_NOERROR;
    }
    break;
  default:
    cError = SNMP_ERR_NOSUCHINSTANCE;
    break;
  }
  return cError;
}

//��������� DMAC
static const struct snmp_scalar_node aDmacTable[] = {
  SNMP_SCALAR_CREATE_NODE(1, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_INTEGER, getDmac, NULL, NULL),     //������ ������� DMAC
  SNMP_SCALAR_CREATE_NODE(2, SNMP_NODE_INSTANCE_WRITE_ONLY , SNMP_ASN1_TYPE_INTEGER, NULL, NULL, setDmac),    //��������� ������� � ���������� ��� ������
  SNMP_SCALAR_CREATE_NODE(3, SNMP_NODE_INSTANCE_READ_ONLY , SNMP_ASN1_TYPE_OCTET_STRING, getDmac, NULL, NULL) //����������� ����
};


static const struct snmp_node* const settingsDmacTable[] = {
  &aDmacTable[0].node.node,
  &aDmacTable[1].node.node,
  &aDmacTable[2].node.node,
};

static const struct snmp_tree_node Dmac_mib2_root = SNMP_CREATE_TREE_NODE(8, settingsDmacTable);

