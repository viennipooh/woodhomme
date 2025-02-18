/**
  ******************************************************************************
  * @file    db.h
  * @author  Dmitry Vakhrushev ( vdv.18@mail.ru )
  * @version V1.0
  * @date    03.05.2016
  * @brief   --
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */ 
  
#ifndef _DB_H__
#define _DB_H__

typedef enum db_err_e {
  DB_OK         = 0,
  DB_NOT_FOUND  = -1,
  DB_ERROR      = -2,
  DB_NO_SPACE   = -3,
  DB_EMPTY      = -4,
  DB_END        = -5,
} db_err_t;

typedef db_err_t (*db_init_t)(void);
typedef db_err_t (*db_deinit_t)(void);
typedef db_err_t (*db_next_t)(char *key, char *value, int next);
typedef db_err_t (*db_set_t)(char *key, char *value);
typedef db_err_t (*db_get_t)(char *key, char *value);
typedef db_err_t (*db_del_t)(char *key);
typedef int      (*db_recs_t)( void );

typedef struct db_adapter_s {
  db_init_t     init;
  db_deinit_t   deinit;
  db_next_t next;
  db_set_t set;
  db_get_t get;
  db_del_t del;
  db_recs_t recs;
} db_adapter_t;


void db_init( void );
void db_deinit( void );

extern db_adapter_t dbbkpsram;
extern db_adapter_t dbsram;

#endif /* _DB_H__ */


