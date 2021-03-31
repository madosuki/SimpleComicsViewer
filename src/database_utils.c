#include "database_utils.h"

char *create_data()
{
  return NULL;
}

int get_file_history(db_s *db, file_history_s *history)
{
  sqlite3 *ppDb;
  int condition = sqlite3_open(db->file_path, &ppDb);
  if(condition != SQLITE_OK) {
    puts("sqlite3 open error");
    return -1;
  }

  const char *select = "select * from 'file-history' order by unixtime asc";
  const ssize_t select_size = strlen(select);

  sqlite3_stmt *stmt;
  int err = sqlite3_prepare_v2(ppDb, select, select_size, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return -1;
  }

  const ssize_t max_list_size = 20;
  ssize_t list_size = 0;
  created_string_s *list = (created_string_s*)malloc(sizeof(created_string_s)  * max_list_size);
  if(list == NULL) {
    puts("failed allocate list");
    return -1;
  }

  
  while(1) {
    err = sqlite3_step(stmt);

    if(err == SQLITE_BUSY) {
      continue;
    }

    if(err == SQLITE_DONE) {
      break;
    }


    created_string_s history_data = {};
    const unsigned char *file_name = sqlite3_column_text(stmt, 1);
    const ssize_t file_name_size = strlen((char*)file_name);
    
    /* history_data.data = (char*)file_name; */
    history_data.data = calloc(file_name_size + 1, 1);
    memmove(history_data.data, file_name, file_name_size);
    history_data.data[file_name_size] = '\0';
    
    history_data.size = file_name_size;

    list[list_size] = history_data;
    ++list_size;

    if(max_list_size >= list_size) {
      history_data.size = list_size;
      break;
    }


    if(err == SQLITE_ROW) {
      continue;
    }

    
    printf("something error\n");
    break;
  }


  sqlite3_finalize(stmt);

  condition = sqlite3_close(ppDb);
  if(condition != SQLITE_OK) {
    puts("sqlite3 close error");
    return -1;
  }

  history->file_path_name_list = list;
  history->size = list_size;

  return 1;
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

  sqlite3_close(ppDb);

  return 1;
}

int create_file_history_table(db_s *db)
{

  const char *create_statement = "create table 'file-history' (id integer primary key autoincrement, path text not null, unixtime integer not null)";

  return create_table(db, create_statement);
}

int create_book_shelf_table(db_s *db)
{

  const char *create_statement = "create table 'book-shelf' (id integer primary key autoincrement, filepath text not null, thumbpath text not null, title text not null)";

  return create_table(db, create_statement);
}

int insert_file_history(db_s *db, const char *file_path, const ssize_t file_path_size, const long unixtime)
{
  sqlite3 *ppDb;
  int err = sqlite3_open(db->file_path, &ppDb);
  if(err == SQLITE_ERROR) {
    return -1;
  }
  const char *sql = "insert into 'file-history' (name, unixtime) (?, ?)";
  const ssize_t swl_size = 50;

  sqlite3_stmt *stmt;
  err = sqlite3_prepare_v2(ppDb, sql, -1, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return -1;
  }

  err = sqlite3_bind_text(stmt, 1, file_path, file_path_size, SQLITE_TRANSIENT);
  if(err == SQLITE_ERROR) {
    sqlite3_finalize(stmt);
    sqlite3_close(ppDb);

    return -1;
  }

  err = sqlite3_bind_int64(stmt, 2, unixtime);
  if(err == SQLITE_ERROR) {
    sqlite3_finalize(stmt);
    sqlite3_close(ppDb);

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

  sqlite3_close(ppDb);

  return 1;
}

int update_file_history(db_s *db, const char *file_name, const ssize_t file_name_size, const long unixtime)
{
  sqlite3 *ppDb;
  int err = sqlite3_open(db->file_path, &ppDb);
  if(err == SQLITE_ERROR) {
    return -1;
  }
  const char *sql = "update 'file-history' set unixtime = ? when name = ?";
  const ssize_t swl_size = 50;

  sqlite3_stmt *stmt;
  err = sqlite3_prepare_v2(ppDb, sql, -1, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return -1;
  }

  err = sqlite3_bind_int64(stmt, 1, unixtime);
  if(err == SQLITE_ERROR) {
    sqlite3_finalize(stmt);
    sqlite3_close(ppDb);

    return -1;
  }

  err = sqlite3_bind_text(stmt, 2, file_name, file_name_size, SQLITE_TRANSIENT);
  if(err == SQLITE_ERROR) {
    sqlite3_finalize(stmt);
    sqlite3_close(ppDb);

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

  sqlite3_close(ppDb);

  return 1;
}

int check_exists_row_in_file_history(db_s *db, const char *file_path_name, const ssize_t file_path_name_size)
{

  sqlite3 *ppDb;
  int err = sqlite3_open(db->file_path, &ppDb);
  if(err == SQLITE_ERROR) {
    return -1;
  }
  const char *sql = "select count(name) from 'file-history' when name = ?";
  const ssize_t swl_size = 50;

  sqlite3_stmt *stmt;
  err = sqlite3_prepare_v2(ppDb, sql, -1, &stmt, NULL);
  if(err == SQLITE_ERROR) {
    printf("failed sqlite prepare v2\n");

    return -1;
  }

  err = sqlite3_bind_text(stmt, 1, file_path_name, file_path_name_size, SQLITE_TRANSIENT);
  if(err == SQLITE_ERROR) {
    sqlite3_finalize(stmt);
    sqlite3_close(ppDb);

    return -1;
  }


  int count = 0;
  while(1) {
    err = sqlite3_step(stmt);

    if(err == SQLITE_BUSY)
      continue;

    count = sqlite3_column_int(stmt, 0);
    
    if(err == SQLITE_DONE)
      break;
  }

  sqlite3_finalize(stmt);

  sqlite3_close(ppDb);

  if(!count) {
    return -1;
  }
  

  return 1;
}

int insert_or_udpate_file_history(db_s *db, const char* file_path_name, const ssize_t file_path_name_size, const long unixtime)
{
  if(file_path_name == NULL || file_path_name_size < 1 || unixtime < 1)
    return -1;

  int is_exists = check_exists_row_in_file_history(db, file_path_name, file_path_name_size);
  if(!is_exists) {
    return insert_file_history(db, file_path_name, file_path_name_size, unixtime);
  }

  return update_file_history(db, file_path_name, file_path_name_size, unixtime);
}
