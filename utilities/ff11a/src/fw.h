#ifndef __FW_H__
#define __FW_H__

#include <stdint.h>

struct fw_version_s {
  uint8_t  fvh;
  uint8_t  fvl;
  uint8_t  hvh;
  uint8_t  hvl;
  uint32_t date;
};

#ifdef BOOTLOADER

extern const struct fw_version_s firmware_data;
#define FW_BASE         0x08000000
#define FW_OFFSET       0x00020000
#define FW_VER_OFFSET   0x00000400

#endif


#ifdef LOADER

extern const struct fw_version_s firmware_data;
#define FW_BASE         0x08000000
#define FW_OFFSET       0x00000000
#define FW_VER_OFFSET   0x00000400

#endif


#ifdef FIRMWARE

extern const struct fw_version_s firmware_data;
#define FW_BASE         0x08000000
#define FW_OFFSET       0x00040000
#define FW_VER_OFFSET   0x00000400
#define FW_START        (FW_BASE + FW_OFFSET)

#define FW_BASE_LOADER         0x08000000
#define FW_OFFSET_LOADER       0x00000000
#define FW_VER_OFFSET_LOADER   0x00000400
#define FW_LOADER_START        (FW_BASE_LOADER + FW_OFFSET_LOADER)

#endif



void boot_code(uint32_t addr);
void boot_app(uint32_t addr, uint32_t addr_size);
int check_application(uint32_t addr, uint32_t addr_size);

#endif