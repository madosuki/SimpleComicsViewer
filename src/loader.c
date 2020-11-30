#include "loader.h"

// const int eocd_size = 22;

// int finally_uncompress_size = 0;

ssize_t get_file_size(const char *filename)
{
  struct stat stat_tmp;
  if (stat(filename, &stat_tmp) == 0)
    return stat_tmp.st_size;

  return -1L;
}

ssize_t get_file_count(const char *filename)
{
  struct archive *arc = archive_read_new();
  archive_read_support_filter_all(arc);
  archive_read_support_format_all(arc);

  int condition = archive_read_open_filename(arc, filename, get_file_size(filename));
  if (condition != ARCHIVE_OK) {
    printf("failed archive_read_open_filename");
    return -1;
  }

  struct archive_entry *entry = NULL;

  ssize_t count = 0;
  while(archive_read_next_header(arc, &entry) != ARCHIVE_EOF) {
    ++count;
  }

  archive_read_close(arc);
  condition = archive_read_free(arc);

  return count;
}

int copy_data_on_memory(struct archive *archive_read, uncompress_data_t *data, ssize_t file_size)
{
  if(data == NULL)
    return -1;

  int condition;
  const void *buf;
  size_t size;
  la_int64_t offset;
  unsigned char *result = (unsigned char*)calloc(file_size + 1, 1);
  ssize_t current_size = 0;
  size_t previous_size = 0;

  do {
    condition = archive_read_data_block(archive_read, &buf, &size, &offset);

    if (condition == ARCHIVE_EOF)
      break;

    if (condition != ARCHIVE_OK)
      return condition;

    if (buf == NULL) {
      break;
    }

    previous_size = current_size;
    current_size += size;
    memcpy(result + previous_size, buf, size);

  } while (condition != ARCHIVE_EOF);

  int check = detect_image(result, current_size);
  if (check != UTILS_JPG && check != UTILS_PNG) {
    free(result);
    result = NULL;
    return NON_IMAGE_MESSAGE;
  }

  result[current_size] = '\0';
  data->data = result;
  data->file_size = current_size;

  return ARCHIVE_OK;
}

int load_from_compress_file(const char* filename, uncompress_data_set_t *result_list)
{

  struct archive *arc = archive_read_new();
  archive_read_support_filter_all(arc);
  archive_read_support_format_all(arc);

  /* int flags = ARCHIVE_EXTRACT_TIME | ARCHIVE_EXTRACT_PERM | ARCHIVE_EXTRACT_ACL | ARCHIVE_EXTRACT_FFLAGS; */
  /* struct archive *extract = archive_write_disk_new(); */
  /* archive_write_disk_set_options(extract, flags); */
  /* archive_write_disk_set_standard_lookup(extract); */

  // const char *filename = "test.zip";
  int condition = archive_read_open_filename(arc, filename, get_file_size(filename));
  if (condition != ARCHIVE_OK) {
    printf("failed archive_read_open_filename");
    return -1;
  }
  
  struct archive_entry *entry = NULL;
  ssize_t file_count = get_file_count(filename);
  uncompress_data_t **data_list = (uncompress_data_t **)calloc(file_count, sizeof(uncompress_data_t*));

  int is_failed = 0;
  int is_image = 0;
  ssize_t read_count = 0;
  while (1) {

    condition = archive_read_next_header(arc, &entry);
    if (condition == ARCHIVE_EOF)
      break;

    if (condition != ARCHIVE_OK)
      break;

    data_list[read_count] = (uncompress_data_t*)malloc(sizeof(uncompress_data_t));

    const char *pathname = archive_entry_pathname(entry);
    const ssize_t pathname_size = strlen(pathname);

    ssize_t file_size = archive_entry_size(entry);


    // printf("%s\n", pathname);

    /* condition = archive_write_header(extract, entry); */
    /* if(condition != ARCHIVE_OK) */
    /*   break; */

    if (archive_entry_size(entry) > 0) {
      // condition = copy_data(arc, extract);
      condition = copy_data_on_memory(arc, data_list[read_count], file_size);
      if (condition != NON_IMAGE_MESSAGE && condition != ARCHIVE_OK) {
        printf("failed copy_data\n");
        is_failed = 1;
        break;
      }

      if (condition != NON_IMAGE_MESSAGE) {
        data_list[read_count]->file_name = (char*)calloc(pathname_size + 1, 1);
        strncpy(data_list[read_count]->file_name, pathname, pathname_size);
        is_image = 1;
      }

      if (condition == NON_IMAGE_MESSAGE) {
        free(data_list[read_count]);
        data_list[read_count] = NULL;
      }
    }

    if(is_image) {
      ++read_count;
    }

    is_image = 0;

    // condition = archive_write_finish_entry(extract);
  }

  archive_read_close(arc);
  condition = archive_read_free(arc);

  /* archive_write_close(extract); */
  /* archive_write_free(extract); */
  if (condition != ARCHIVE_OK) {
    printf("failed archive_read_free\n");
    return -1;
  }


  result_list->uncompress_data_list = data_list;
  result_list->size = read_count;

  // printf("%zd, %zd\n", file_count, read_count);

  return 1;
}


int convert_bytes_to_int(unsigned char *src)
{
  int result = src[3] << 24 |
    src[2] << 16 |
    src[1] << 8 |
    src[0];

  return result;
}

void free_uncompress_data_set(uncompress_data_set_t *data)
{
  if(data != NULL) {

    if(data->uncompress_data_list != NULL) {

      for(int i = 0; i < data->size; i++) {
        if(data->uncompress_data_list[i] != NULL) {

          if(data->uncompress_data_list[i]->data != NULL) {
            free(data->uncompress_data_list[i]->data);
            data->uncompress_data_list[i]->data = NULL;
          }

          if(data->uncompress_data_list[i]->file_name != NULL) {
            free(data->uncompress_data_list[i]->file_name);
            data->uncompress_data_list[i]->file_name = NULL;
          }

          free(data->uncompress_data_list[i]);
          data->uncompress_data_list[i] = NULL;
        }

      }

      free(data->uncompress_data_list);
      data->uncompress_data_list = NULL;
    }

    free(data);
    data = NULL;
  }

}