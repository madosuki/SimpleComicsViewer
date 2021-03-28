#include "database_utils.h"

created_string_s *create_pair(sqlite_pair_s *data)
{
  if(data == NULL) {
    puts("detect null in create_pair first");
    return NULL;
  }
  
  const char *space = " ";

  ssize_t whole_size = data->key_len + data->conjuction_len + data->value_len + 3;
  char *tmp = (char*)calloc(whole_size, 1);
  if(tmp == NULL) {
    puts("failed realloc in create_pair");
    return NULL;
  }

  memmove(tmp, data->key, data->key_len);
  memmove(tmp + data->key_len, space, 1);
  memmove(tmp + data->key_len + 1, data->conjuction, data->conjuction_len);
  memmove(tmp + data->key_len + 1 + data->conjuction_len, space, 1);
  memmove(tmp + data->key_len + 1 + data->conjuction_len + 1, data->value, data->value_len);
  tmp[whole_size] = '\0';

  created_string_s *result = malloc(sizeof(created_string_s));
  memset(result, 0, sizeof(created_string_s));
  result->data = tmp;
  result->size = whole_size - 1;

  return result;
}

created_string_s *create_where(sqlite_query_pair_with_conditon_s_list *list)
{
  if(list == NULL)
    return NULL;
  
  const char *prefix = "where ";
  const ssize_t prefix_len = 6;
  
  ssize_t result_len = prefix_len + 1;
  char *result = (char*)calloc(result_len, 1);
  if(result == NULL) {
    puts("failed allocation memory in result");
    return NULL;
  }

  memmove(result, prefix, prefix_len);

  ssize_t current_pos = prefix_len;
  for(ssize_t i = 0; i < list->size; ++i) {
      sqlite_query_pair_with_conjuction_s *data = list->pair_list[i];
      created_string_s *tmp = create_pair(data->data);
      if(tmp == NULL) {
        puts("failed allocation in create pair");
        free(result);
        result = NULL;
        return NULL;
      }

      result_len += tmp->size;

      int is_conjuction = 0;
      if(i != 0 && data->conjuction != NULL && data->conjuction_len > 0) {
        result_len += data->conjuction_len + 2;
        is_conjuction = 1;
      }
    
      char *check = realloc(result, result_len);
      if(check == NULL) {
        free(result);
        result = NULL;

        free(tmp);
        tmp = NULL;

        puts("failed realloc");
      
        return NULL;
      }
      result = check;
      
      if(is_conjuction) {
        memmove(result + current_pos, " ", 1);
        ++current_pos;
        
        memmove(result + current_pos, data->conjuction, data->conjuction_len);
        current_pos += data->conjuction_len;
        
        memmove(result + current_pos, " ", 1);
        ++current_pos;

      }
      memmove(result + current_pos, tmp->data, tmp->size);
      current_pos += tmp->size;

      free(tmp);
      tmp = NULL;
  }

  result[result_len - 1] = '\0';
  
  created_string_s *return_value = (created_string_s*)malloc(sizeof(created_string_s));
  memset(return_value, 0, sizeof(created_string_s));
  return_value->data = result;
  return_value->size = result_len - 1;

  return return_value;
}

char *create_select_string(const char *table_name, const char *column,  created_string_s **options, ssize_t options_length)
{
  const char *prefix = "select ";
  const ssize_t prefix_length = 7;
  
  const char *from = " from ";
  const ssize_t from_length = 6;
  
  const ssize_t column_length = strlen(column);
  const ssize_t table_name_length = strlen(table_name);
  
  ssize_t size = prefix_length + from_length + strlen(column) + strlen(table_name) + 2;
  char *select = (char*)calloc(size, 1);

  ssize_t pos = 0;
  memmove(select, prefix, prefix_length);
  pos += prefix_length;
  
  memmove(select + pos, column, column_length);
  pos += column_length;

  memmove(select + pos, from, from_length);
  pos += from_length;
  
  memmove(select + pos, table_name, table_name_length);
  pos += table_name_length;

  if(options != NULL && options_length > 0) {
    ssize_t options_string_length = 0;
    for(ssize_t i = 0; i < options_length; ++i) {
      if(options[i] != NULL && options[i]->data != NULL) {
        options_string_length += options[i]->size;
      }        
    }

    if(options_string_length > 0) {
      size += options_string_length;
      char *tmp = realloc(select, size);
      if(tmp == NULL) {
        free(select);
        select = NULL;
        puts("failed realloc in select_sqlite3");
        return NULL;
      }
      select = tmp;
      memmove(select + pos, " ", 1);
      ++pos;
    
      for(ssize_t i = 0; i < options_length; ++i) {
        memmove(select + pos, options[i]->data, options[i]->size);
        pos += options[i]->size;
      }
    }

  }

  select[size] = '\0';

  return select;
}

file_histoy_s *get_file_history(db_s *db)
{
  sqlite3 *ppDb;
  int condition = sqlite3_open(db->file_path, &ppDb);
  if(condition != SQLITE_OK) {
    puts("sqlite3 open error");
    return NULL;
  }

  const char *table_name = "'file-history'";
  created_string_s orderby = {};
  orderby.data = "order by date asc";
  orderby.size = 13;

  created_string_s **options_list = malloc(sizeof(created_string_s*));
  if(options_list == NULL) {
    puts("failed allocate options_list");
    
    return NULL;
  }
  options_list[0] = &orderby;

  char *select = create_select_string(table_name, "*", options_list, 1);
  free(options_list);
  options_list = NULL;

  if(select == NULL) {
    puts("failed create select");
    return NULL;
  }

  const ssize_t select_size = strlen(select);

  sqlite3_stmt *stmt;
  int err = sqlite3_prepare_v2(ppDb, select, select_size, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return NULL;
  }


  file_histoy_s *history = malloc(sizeof(file_histoy_s));
  if(history == NULL) {
    puts("failed allocate history");
    return NULL;
  }
  ssize_t history_size = 1;
  
  while(1) {
    err = sqlite3_step(stmt);

    if(err == SQLITE_BUSY) {
      continue;
    }

    if(err == SQLITE_DONE) {
      break;
    }


    file_histoy_s history_data = {};
    int id = sqlite3_column_int(stmt, 0);
    const unsigned char *file_name = sqlite3_column_text(stmt, 1);
    const ssize_t file_name_size = strlen((char*)file_name);
    
    history_data.file_path = (char*)file_name;
    history_data.file_path_size = file_name_size;

    history[history_size - 1] = history_data;
    ++history_size;


    if(err == SQLITE_ROW) {
      continue;
    }

    
    printf("something error\n");
    break;
  }


  sqlite3_finalize(stmt);

  free(select);
  select = NULL;
  
  condition = sqlite3_close(ppDb);
  if(condition != SQLITE_OK) {
    puts("sqlite3 close error");
    return NULL;
  }


  return history;
}

int inline create_table(db_s *db, const char *sql)
{
  sqlite3 *ppDb;
  int err = sqlite3_open(db->file_path, &ppDb);
  if(err == SQLITE_ERROR) {
    return -1;
  }

  sqlite3_stmt *stmt;
  err = sqlite3_prepare_v2(ppDb, sql, -1, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return -1;
  }

  while(1) {
    err = sqlite3_step(stmt);

    if(err == SQLITE_BUSY)
      continue;

    if(err == SQLITE_DONE)
      break;
    
  }

  sqlite3_finalize(stmt);

  return 1;
}

int create_file_history_table(db_s *db)
{

  const char *create_statement = "create table 'file-history' (id integer primary key autoincrement, path text not null, date datetime not null)";

  return create_table(db, create_statement);
}

int create_book_shelf_table(db_s *db)
{

  const char *create_statement = "create table 'book-shelf' (id integer primary key autoincrement, filepath text not null, thumbpath text not null, title text not null)";

  return create_table(db, create_statement);
}
