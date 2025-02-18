#include "main.h"
#include "fw.h"
#include "flash.h"
#include "crc.h"
#include "stm32f4xx.h"
#include "db.h"

#ifdef __IAR_SYSTEMS_ICC__ 


const struct fw_version_s firmware_data @ (FW_BASE+FW_OFFSET+FW_VER_OFFSET) = {
  MAIN_VER_MAJ, MAIN_VER_MIN,
  0x01, 0x00,
  0x00000000,
};

extern void start_application(unsigned long app_link_location)  
{  
    asm(" ldr sp, [r0,#0]"); // load the stack pointer value from the program's reset vector  
    asm(" ldr pc, [r0,#4]"); // load the program counter value from the program's reset vector to cause operation to continue from there  
}

int check_application(uint32_t addr, uint32_t addr_size)
{
  unsigned long crc;
  crc = hw_crc32((unsigned char*)addr,addr_size-4);
  if(crc == *((unsigned long*)(addr+addr_size-4)))
    return 1;
  return 0;
}

void boot_code(uint32_t addr) { //Переход к выполнению кода с адреса
    __DSB();
    __ISB();
    SCB->VTOR = (uint32_t)addr; 
    __DSB();
    __ISB();
    start_application(addr);
}


#endif