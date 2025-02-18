#ifndef __LD_API_H__
#define __LD_API_H__


#define LD_API_TABLE_ADDR       0x08000800
#define LD_APITABLE             ((unsigned long *)LD_API_TABLE_ADDR)
#define LD_VERSION              (LD_APITABLE[0])
#define LD_FLASHTABLE           ((unsigned long *)(LD_APITABLE[1]))
#define LD_APPTABLE             ((unsigned long *)(LD_APITABLE[2]))

#define LD_FlashLock            ((void (*)( void ))LD_FLASHTABLE[0])
#define LD_FlashUnlock          ((void (*)( void ))LD_FLASHTABLE[1])
#define LD_FlashSectorErase     ((void (*)( unsigned char sector ))LD_FLASHTABLE[2])
#define LD_FlashProgram                                                 \
                      ((void (*)( unsigned long *addr,                  \
                                  unsigned long *buffer,                \
                                  unsigned long length))LD_FLASHTABLE[3])

#define LD_AppErase             ((void (*)( void ))LD_APPTABLE[0])
#define LD_AppUpdErase          ((void (*)( void ))LD_APPTABLE[1])
#define LD_CheckApp                                                     \
                      ((void (*)( unsigned long addr,                   \
                                  unsigned long addr_size ))LD_APPTABLE[2])
#define LD_BootApp                                                    \
                      ((void (*)( unsigned long addr,                   \
                                  unsigned long addr_size ))LD_APPTABLE[3])
#define LD_BootLoader           ((void (*)( void ))LD_APPTABLE[4])
#define LD_BootLoaderDelay      ((void (*)( void ))LD_APPTABLE[5])
#define LD_UpdateApp            ((void (*)( void ))LD_APPTABLE[6])
#define LD_RebootWOUpdate       ((void (*)( void ))LD_APPTABLE[7])
#define LD_Reboot               ((void (*)( void ))LD_APPTABLE[8])
#endif