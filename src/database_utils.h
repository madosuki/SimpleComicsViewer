#ifndef DATABASE_UTILS
#define DATABASE_UTILS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>
#include <time.h>

typedef enum SQLITE_TYPE_ENUM{
  INTEGER,
  TEXT
} SQLITE_TYPE_ENUM;

typedef struct {
  char *key;
  ssize_t key_len;
  
  char *conjuction;
  ssize_t conjuction_len;
  
  char *value;
  ssize_t value_len;
  int is_dynamic_value;
} sqlite_pair_s;

typedef struct {
  sqlite_pair_s *data;
  
  char *conjuction;
  ssize_t conjuction_len;
} sqlite_query_pair_with_conjuction_s;

typedef struct {
  sqlite_query_pair_with_conjuction_s **pair_list;
  ssize_t size;
} sqlite_query_pair_with_conditon_s_list;

typedef struct {
  ssize_t size;
  char *data;
} created_string_s;

typedef struct {
  void *value;
  SQLITE_TYPE_ENUM type;
} sqlite_value_s;

typedef struct {
  sqlite_value_s *value_list;
  char **column_name_list;
  int list_size;
} sqlite_value_s_list;

typedef struct {
  created_string_s **file_path_name_list;
  ssize_t size;
} file_history_s;

typedef struct {
  uint8_t *thumbnail;
  ssize_t thumbnail_x;
  ssize_t thumbnail_y;
  ssize_t thumbnail_array_size;
} thumbnail_s;

typedef struct {
  char *file_path;
  ssize_t file_path_size;
  thumbnail_s thumbnail;
} book_info_s;

typedef struct {
  book_info_s *list;
  ssize_t size;
} book_shelf_s;

typedef struct {
  char *file_path;
  ssize_t file_path_length;
} db_s;


int get_file_history(db_s *db, file_history_s *history);
int create_file_history_table(db_s *db);
int create_book_shelf_table(db_s *db);

int insert_or_udpate_file_history(db_s *db, const char* file_path_name, const ssize_t file_path_name_size, const long unixtime);

#endif
