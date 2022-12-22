#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdint.h>

#include <archive.h>
#include <archive_entry.h>

#include "utils.h"

#define COMPRESS_METHOD_DEFLATE 8
#define NON_IMAGE_MESSAGE 2


typedef struct {
  uint32_t signature;
  uint16_t version;
  uint16_t bitflag;
  uint16_t compress_method;
  uint16_t latest_modify_time;
  uint16_t latest_modify_date;
  uint32_t crc32;
  uint32_t compress_size;
  uint32_t uncompress_size;
  uint16_t file_name_length;
  uint16_t extra_field_length;
  uint8_t *file_name;
  uint8_t *extra_field;
} zip_local_header_t;

typedef struct {
  uint32_t signature;
  uint16_t version;
  uint16_t minimum_version;
  uint16_t bitflag;
  uint16_t compress_method;
  uint16_t latest_modify_time;
  uint16_t latest_modify_date;
  uint32_t crc32;
  uint32_t compress_size;
  uint32_t uncompress_size;
  uint16_t file_name_length;
  uint16_t extra_field_length;
  uint16_t file_comment_length;
  uint16_t file_start_disk_num;
  uint16_t internal_file_attribute;
  uint32_t external_file_attribute;
  uint32_t offset_relative_local_header;
  uint8_t *file_name;
  uint8_t *extra_field;
  uint8_t *file_comment;
} zip_central_header_t;

typedef struct {
  uint8_t *data;
  uint8_t *file_name;
  uint32_t file_size;
} uncompress_data_t;

typedef struct {
  uncompress_data_t **uncompress_data_list;
  ssize_t size;
} uncompress_data_set_t;

void free_uncompress_data_set(uncompress_data_set_t *data);

int load_from_compress_file(const char *file_name, uncompress_data_set_t *data_set);

int get_file_size(const char *filename, size_t *file_size_ptr);

int copy_data_on_memory(struct archive *archive_read, uncompress_data_t *data, size_t file_size);

#endif // LOADER_H
