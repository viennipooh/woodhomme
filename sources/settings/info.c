#include "info.h"
#include "otp.h"
#include "string.h"
#include "mbcrc.h"


const struct info_s default_info = {
  .timestamp = 0x00000000,
  .hw_version = {
    .major = 1,
    .minor = 2
  },
  .serial_number = 0x0002,
  .serial_str = "NONE",
  .mac = {0x00,0x11,0x22,0x33,0x44,0x55},
  .crc = 0x00
};

struct info_s const * p_default_info = &default_info;

static int check_ff(unsigned char *data,int len){
  for(int i=0;i<len;i++)
  {
    if(data[i]!=0xFF)
      return -1;
  }
  return 0;
}

int current_pos() {
  int index = -1;
  int i;
  for (i = 0; i < 16; i++) {
    if (otp_blocklocked(i) == otp_result_ok) {
      index = i;
    } else {
      break;
    }
  }
  return index;
}

enum eFulfil TestFulfill(char * iSerialStr) {
  if (strncmp(iSerialStr, "FMQ", 3) == 0)
    return fOld;
  if (strncmp(iSerialStr, "FQX", 3) == 0)
    return fCurrent;
  if (strncmp(iSerialStr, "NONE", 4) == 0)
    return fNone;
  return fError;
}

int info_current(struct info_s * _info)
{
  uint16_t crc16=0;
  int index = -1;
  int i = 0;
  struct info_s info;
  struct info_new_s * info_new;
  unsigned char *p = (unsigned char*)&info;
  // Count blocked pages, records
  memcpy(_info,&default_info,sizeof(default_info));
  index =  current_pos();
  if((index>=0) && (index<16))
  {
    for(i=0;i<32;i++)
    {
      p[i] = otp_read(index,i);
    }
  }
  else
  {
    return -1;
  }
  crc16 = mbcrc(p,30);
  if(crc16 == info.crc)
  {
    enum eFulfil cFulfil = TestFulfill(info.serial_str);
    switch (cFulfil) {
    case fNone:
    case fOld:
    case fCurrent:
      _info->hw_version = info.hw_version;
      _info->serial_number = (info.serial_number & 0xffff);
      _info->timestamp = info.timestamp;
      memcpy(_info->mac, info.mac, 6);
      memcpy(_info->serial_str, info.serial_str, INFO_MAX_SERIAL_STRING);
      break;
    case fError:
      info_new = (struct info_new_s *)&info;
      cFulfil = TestFulfill(info_new->serial_str);
      switch (cFulfil) {
      case fOld:
      case fCurrent:
        _info->hw_version = info_new->hw_version;
        _info->serial_number = (info_new->serial_number & 0xffff);
        _info->timestamp = info_new->timestamp;
        memcpy(_info->mac, info_new->mac, 6);
        memcpy(_info->serial_str, info_new->serial_str, INFO_MAX_SERIAL_STRING_OLD);
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
  }
  else if(check_ff(p,32) != 0)
  {
    memcpy(_info,p,32);
  }
  else
  {
    memcpy(_info,&default_info,sizeof(default_info));
    return -1;
  }
  return 0;
}

int info_count()
{
  int cnt = 0;
  // Count blocked pages, records
  for(int i=0;i<16;i++)
  {
    if(otp_blocklocked(i) == otp_result_ok)
    {
      cnt += 1;
    }
    else break;
  }
  return cnt;
}

#include "log.h"
otp_result_t info_write(struct info_s * _info)
{
//  // !!! Для проверок - отключение фактической записи
//  return otp_result_ok;
//  // !!!
  uint16_t crc16;
  int index = current_pos();
  int i = 0;
  otp_result_t cResult;
  _info->timestamp = GetCurrTime();
  unsigned char *p = (unsigned char*)_info;
  crc16 = mbcrc((unsigned char *)_info, 30);
  _info->crc = crc16;
  if((index >= -1) && (index < 15)) {
    enum eFulfil cFulfil = TestFulfill(_info->serial_str);
    switch (cFulfil) {
    case fNone:
    case fError:
      memset(_info->serial_str, 0, INFO_MAX_SERIAL_STRING);
      strcpy(_info->serial_str, "FQX");
      break;
    default:
      break;
    }
    ++index;
    for(i = 0; i < 32; i++) {
      cResult = otp_write(index, i, p[i]);
      if (cResult == otp_result_error) {
        if (check_ff((unsigned char *)(OTP_START_ADDR + index * OTP_BYTES_IN_BLOCK), 32) != 0) {
          otp_blocklock(index); //Что-то записалось - заблокируем
        }
        return cResult;
      }
    }
    if (cResult == otp_result_ok)
      otp_blocklock(index); //Записалось верно - заблокируем
    return cResult;
  } else {
    return otp_result_error;
  }
}

#include "main.h"
#include "info.h"
const uint8_t gPrototype[6] = {0x26, 0x00, 0x41, 0x00, 0x05, 0x51};
bool IsFirmMAC(uint8_t * ipMAC) {
  if ((ipMAC[0] == MAC_ADDR_MICR0) &&
      (ipMAC[1] == MAC_ADDR_MICR1) &&
      (ipMAC[2] == MAC_ADDR_MICR2)) {
    return true;
#ifdef USE_PROTOTYPE_MKPSH //Учет опытного образца МКПШ
  } else { //Учесть MAC опытного МКПШ
    if (memcmp(ipMAC, gPrototype, 6) == 0)
      return true;
#endif
  }
  return false;
}

extern bool IsDefaultMac(char * iMac);
otp_result_t WriteHWSetting(uint32_t iHWSetting, int iHWIndex) {
  struct info_s info;

  info_current(&info);
  if (IsDefaultMac((char *)info.mac)) {
    info.mac[0] = MAC_ADDR0;
    info.mac[1] = MAC_ADDR1;
    info.mac[2] = MAC_ADDR2;
    info.mac[3] = MAC_ADDR3;
    info.mac[4] = MAC_ADDR4;
    info.mac[5] = MAC_ADDR5;
  }
  otp_result_t cResult = otp_result_error;
  switch (iHWIndex) {
  case 2:
    info.serial_number = iHWSetting;
    cResult = info_write(&info);
    break;
  case 3:
    info.hw_version.major = (iHWSetting >> 8) & 0xff;
    info.hw_version.minor = (iHWSetting     ) & 0xff;
    cResult = info_write(&info);
    break;
  case 4:
    info.hw_version.major = (iHWSetting >> 8) & 0xff;
    info.hw_version.minor = (iHWSetting     ) & 0xff;
    info.mac[0] = MAC_ADDR_MICR0;
    info.mac[1] = MAC_ADDR_MICR1;
    info.mac[2] = MAC_ADDR_MICR2;
    info.mac[3] = MAC_ADDR_MICR3;
    info.mac[4] = (iHWSetting >> 24) & 0xff;
    info.mac[5] = (iHWSetting >> 16) & 0xff;
    cResult = info_write(&info);
    break;
  default:
    break;
  }
  return cResult;
}

char info_fw_1[6] = "NONE";
char info_sw_1[6] = "0.0";

const char * GetInfo_fw() {
  struct info_s info_id;

  if(info_current(&info_id) == 0) {
    if ((info_id.hw_version.major != 0) || (info_id.hw_version.minor != 0)) {
      sprintf(info_fw_1, "%d.%d", info_id.hw_version.major, info_id.hw_version.minor);
    }
  }

  return info_fw_1;
}

uint32_t GetInfo_sID() {
  struct info_s info_id;

  uint32_t sID = 0;
  if(info_current(&info_id) == 0) {
    sID = info_id.serial_number;
  }
  return sID;
}

const char * GetInfo_sw() {
  sprintf(info_sw_1, "%d.%d", MAIN_VER_MAJ, MAIN_VER_MIN);
  return info_sw_1;
}

