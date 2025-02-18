#include "lwip/opt.h"
#include "lwip/def.h"
#include "fs.h"
#include <string.h>

#include "ff.h"

#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name);
int fs_read_custom(struct fs_file *file, char *buffer, int count);
void fs_close_custom(struct fs_file *file);
#endif /* LWIP_HTTPD_CUSTOM_FILES */

#define MAX_CUSTOM_OPEN_FILES 10
struct file_custom_open_s {
  FIL file;
  int is_opend;
};

struct file_custom_open_s open_custom_files[MAX_CUSTOM_OPEN_FILES]={
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
  {.is_opend = 0,},
};


#if LWIP_HTTPD_CUSTOM_FILES
int fs_open_custom(struct fs_file *file, const char *name)
{
  int i = 0;
  struct file_custom_open_s *opened = NULL;
  char file_name[0x50] = "RAM:";
  
  if(0 == strncmp(name,"/fs/ram/",8))
  {
    strcat(file_name,&name[7]);
    
    for(i=0;i<MAX_CUSTOM_OPEN_FILES;i++)
    {
      if(open_custom_files[i].is_opend == 0)
      {
        opened = &open_custom_files[i];
        file->custom_file = (void*)&open_custom_files[i];
      }
    }
    if(opened)
    {
      if(f_open(&(opened->file),file_name,FA_READ) == FR_OK)
      {
        opened->is_opend = 1;
        file->len = (opened->file).fsize;
        file->index = 0;//file->len;
        file->pextension = NULL;
        file->http_header_included = 0;
//#if HTTPD_PRECALCULATED_CHECKSUM
//      file->chksum_count = f->chksum_count;
//      file->chksum = f->chksum;
//#endif /* HTTPD_PRECALCULATED_CHECKSUM */
//#if LWIP_HTTPD_FILE_STATE
//      file->state = fs_state_init(file, name);
//#endif /* #if LWIP_HTTPD_FILE_STATE */
        //opened->file.data = (const char *)f->data;
        //file->len = 1000;
        return 1;
      }
    }
    else
    {
      return 0;
    }
  }
  return 0;
}
int fs_read_custom(struct fs_file *file, char *buffer, int count)
{
  UINT rb;
  
  if(f_read((FIL*)(file->custom_file), buffer, count, &rb) == FR_OK)
  {
    
    if(file->index == file->len) {
      return -1;
    }

    file->index += rb;

    return (rb==0)?(-1):(rb);
  }
  return -1;
}
void fs_close_custom(struct fs_file *file)
{
  struct file_custom_open_s* opend = (struct file_custom_open_s*)(file->custom_file);
  opend->is_opend = 0;
  f_close((FIL*)(file->custom_file));
}
#endif /* LWIP_HTTPD_CUSTOM_FILES */