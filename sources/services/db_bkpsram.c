/**
  ******************************************************************************
  * @file    db.c
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    03.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  * 
  ******************************************************************************
  */ 

#include "db.h"
#include "stdarg.h"
#include "bkpsram.h"
#include "stm32f4xx.h"
#include "string.h"
#include <ctype.h>

#define BKPSRAM_SIZE 0x1000

static void dbbkpsram_lock()
{
  bkpsram_lock();
}

static void dbbkpsram_unlock()
{
  bkpsram_unlock();
}

typedef enum parse_state_e {
  PARSE_STATE_INIT,
  PARSE_STATE_VARIABLE,
  PARSE_STATE_VALUE,
  PARSE_STATE_COMPLETE,
} parse_state_t;


db_err_t dbbkpsram_addrec( char * _addr, uint16_t _max_size, char *key, char *value )
{
  char *addr = _addr, *eaddr=addr+_max_size;
  int size,i;
  size = (strlen(key)+strlen(value)+3);
  while(*addr!='\0' && addr != eaddr)
  {
    addr++;
  }
  //if(addr==_addr)
  //  return DB_ERROR;
  
  if(size >= (int)(eaddr-addr))
  {
    return DB_NO_SPACE;
  }
  
  for(i=0;i<strlen(key);i++)
  {
    *addr++ = key[i];
  }
  *addr++ = '=';
  
  for(i=0;i<strlen(value);i++)
  {
    *addr++ = value[i];
  }
  *addr++ = ';';
  
  *addr = '\0';
  return DB_OK;
}

#define parse_key_get_str() buffer
#define parse_key_reset() {buffer_index=0;}
#define parse_key_push(ch) {if(buffer_index<sizeof(buffer))\
                        buffer[buffer_index++]=ch;}
#define parse_value_get_str() value
#define parse_value_reset() {value_index=0;}
#define parse_value_push(ch) {if(value_index<sizeof(value))\
                        value[value_index++]=ch;}
static uint8_t buffer_index = 0;
static uint8_t value_index = 0;
static char buffer[0xFF] = {0,};
static char value[0xFF] = {0,};

static db_err_t dbbkpsram_next_key( char * _addr, uint16_t _max_size, char *key, char *value, int next)
{
  db_err_t retval=DB_OK;
  char ch=0;
  int index=0,err=0;
  static char *addr=0; 
  parse_state_t state = PARSE_STATE_INIT;
  if(addr==0)
  {
    addr=_addr;
  }
  if(next==0)
  {
    addr=_addr;
  }
  
  if( (*addr=='\0') || (addr>=(_addr+_max_size)) )
  {
    return DB_END;
  }
  
  if( (*addr=='\0') && (addr==_addr) )
  {
    return DB_EMPTY;
  }
  
  while(state != PARSE_STATE_COMPLETE)
  {
    ch = *addr;
    if(state == PARSE_STATE_COMPLETE)
    {
      retval=DB_OK;
      break;
    }
    if('\0' == ch)
    {
      state = PARSE_STATE_COMPLETE;
      retval=DB_OK;
      break;
    }
    else 
    {
      addr++;
    }
    switch(state)
    {
      case PARSE_STATE_INIT:
        {
          if(isalnum(ch) || '_' == ch)
          {
            key[index++]=ch;
            state = PARSE_STATE_VARIABLE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VARIABLE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            key[index++]=ch;
          }
          else
          if('=' == ch)
          {
            key[index++]='\0';
            index=0;
            state = PARSE_STATE_VALUE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VALUE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            value[index++]=ch;
          }
          else
          if(';' == ch)
          {
            value[index++]='\0';
            state = PARSE_STATE_COMPLETE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_COMPLETE:
        {
        }
        break;
    }
    if(err)
    {
      retval=DB_ERROR;
      return DB_OK;
    }
  }
  return retval;
}

char *dbbkpsram_findkey( char * _addr, uint16_t _max_size, char *key )
{
//  struct list_str_value_s *element = 0;
  parse_state_t state = PARSE_STATE_INIT;
  char *addr = _addr, *eaddr=addr+_max_size;
  char ch = 0;
  int err = 0;
  int find = 0;
  int find_cnt = 0;
  
  memset(buffer,0,sizeof(buffer));
  memset(value,0,sizeof(value));
  
  parse_key_reset();
  /* Парсер настроек */
  while(state != PARSE_STATE_COMPLETE && addr != eaddr)
  {
    ch = *addr;
    if('\0' == ch)
    {
      state = PARSE_STATE_COMPLETE;
    }
    else 
    {
      addr++;
    }
    switch(state)
    {
      case PARSE_STATE_INIT:
        {
//          element = 0;
          if(isalnum(ch) || '_' == ch)
          {
            parse_key_reset();
            parse_key_push(ch);
            state = PARSE_STATE_VARIABLE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VARIABLE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            parse_key_push(ch);
          }
          else
          if('=' == ch)
          {
            parse_key_push(0);
            if(strlen(key)==strlen(buffer))
            {
              if(strcmp(key,buffer)==0)
              {
                find=1;
                find_cnt++;
              }
            }
            //element = get_element(parse_key_get_str());
            parse_key_reset();
            state = PARSE_STATE_VALUE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VALUE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            if(find)
            {
              parse_value_push(ch);
            }
          }
          else
          if(';' == ch)
          {
            if(find)
            {
              parse_value_push(0);
              find=0;
            }
            parse_value_reset();
            state = PARSE_STATE_INIT;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
    }
    if(err)
    {
      parse_value_reset();
      parse_key_reset();
      memset(buffer,0,sizeof(buffer));
      memset(value,0,sizeof(value));
      return 0;
    }
  }
  if(find_cnt)
  {
    return value;
  }
  return 0;
}

static db_err_t dbbkpsram_clean( char * _addr, uint16_t _max_size )
{
  char find=0;
  char *addr = _addr, *eaddr=addr+_max_size;
  while(addr != eaddr)
  {
    if(find)
    {
      *addr++='\0';
    }
    else
    {
      if(*addr++=='\0')
      {
        find = 1;
      }
    }
  }
  return DB_OK;
}

static db_err_t dbbkpsram_delkey( char * _addr, uint16_t _max_size, char *key )
{
  
//  struct list_str_value_s *element = 0;
  parse_state_t state = PARSE_STATE_INIT;
  char *addr = _addr, *eaddr=addr+_max_size;
  char ch = 0;
  int err = 0;
  int find = 0;
  int find_cnt = 0;
  
  memset(buffer,0,sizeof(buffer));
  memset(value,0,sizeof(value));
  
  parse_key_reset();
  /* Парсер настроек */
  while(state != PARSE_STATE_COMPLETE && addr != eaddr)
  {
    ch = *addr;
    if('\0' == ch)
    {
      state = PARSE_STATE_COMPLETE;
    }
    else 
    {
      addr++;
    }
    switch(state)
    {
      case PARSE_STATE_INIT:
        {
//          element = 0;
          if(isalnum(ch) || '_' == ch)
          {
            parse_key_reset();
            parse_key_push(ch);
            state = PARSE_STATE_VARIABLE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VARIABLE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            parse_key_push(ch);
          }
          else
          if('=' == ch)
          {
            parse_key_push(0);
            if(strlen(key)==strlen(buffer))
            {
              if(strcmp(key,buffer)==0)
              {
                /* Erase record*/
                char *begin=addr-strlen(buffer)-1;
                char *start=begin;
                int len = 0;
                char *end=begin;
                find=1;
                find_cnt++;
                while(*addr != ';')
                {
                  addr++;
                };
                end=addr+1;
                len=end-start;
                addr=begin;
                while(*end!='\0')
                {
                  *begin++=*end++;
                }
                while((begin-start)<len)
                {
                  *begin++='\0';
                }
                *begin='\0';
                addr=start;
                state = PARSE_STATE_INIT;
                parse_key_reset();
                break;
              }
            }
            //element = get_element(parse_key_get_str());
            parse_key_reset();
            state = PARSE_STATE_VALUE;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
      case PARSE_STATE_VALUE:
        {
          if(isalnum(ch) || '_' == ch)
          {
            if(find)
            {
              parse_value_push(ch);
            }
          }
          else
          if(';' == ch)
          {
            if(find)
            {
              parse_value_push(0);
            }
            parse_value_reset();
            state = PARSE_STATE_INIT;
          }
          else
          if(isspace(ch))
          {
            break;
          }
          else
          {
            err++;
          }
        }
        break;
    }
    if(err)
    {
      parse_value_reset();
      parse_key_reset();
      memset(buffer,0,sizeof(buffer));
      memset(value,0,sizeof(value));
      return DB_ERROR;
    }
  }
  if(find)
  {
    return DB_OK;
  }
  return DB_NOT_FOUND;
}


static db_err_t dbbkpsram_next( char *key, char *value, int next)
{
  return dbbkpsram_next_key((char*)BKPSRAM_BASE,BKPSRAM_SIZE,key,value,next);
}

static db_err_t dbbkpsram_del( char *key )
{
  dbbkpsram_unlock();
  dbbkpsram_delkey((char*)BKPSRAM_BASE,BKPSRAM_SIZE, key);
  dbbkpsram_clean((char*)BKPSRAM_BASE,BKPSRAM_SIZE);
  dbbkpsram_lock();
  return DB_OK;
}

static db_err_t dbbkpsram_set( char *key, char *value )
{
  dbbkpsram_unlock();
  dbbkpsram_delkey((char*)BKPSRAM_BASE,BKPSRAM_SIZE, key);
  dbbkpsram_addrec((char*)BKPSRAM_BASE,BKPSRAM_SIZE, key, value);
  dbbkpsram_clean((char*)BKPSRAM_BASE,BKPSRAM_SIZE);
  dbbkpsram_lock();
  return DB_OK;
}

static db_err_t dbbkpsram_get( char *key, char *value )
{
  char *retval = 0;
  if((retval = dbbkpsram_findkey((char*)BKPSRAM_BASE,BKPSRAM_SIZE, key )))
  {
    strcpy(value,retval);
  }
  if(retval)
    return DB_OK;
  return DB_ERROR;
}

static int dbbkpsram_recs( )
{
  int retval = 0;
  db_err_t ret = DB_OK;
  if((ret = dbbkpsram_next(buffer,value,0)) == DB_OK)
  {  
    while(ret == DB_OK)
    {
      retval++;
      ret = dbbkpsram_next(buffer,value,1);
    }
  }
  return retval;
}

db_adapter_t dbbkpsram = {
  .next = dbbkpsram_next,
  .set  = dbbkpsram_set,
  .get  = dbbkpsram_get,
  .del  = dbbkpsram_del,
  .recs = dbbkpsram_recs,
};
