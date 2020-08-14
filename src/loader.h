#ifndef LOADER_H
#define LOADER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <zlib.h>
#include <stdint.h>

#include "utils.h"

#define COMPRESS_METHOD_DEFLATE 8

const int eocd_siz;

int finally_uncompress_size;

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
  int size;
} uncompress_data_set_t;

void free_uncompress_data_set(uncompress_data_set_t *data);

int *get_central_directory_offset(FILE *fp, const int file_size);

void set_central_headers(zip_central_header_t **headers, int central_offset, int cd_num_on_disk, int file_size, FILE *fp);

void free_local_header(zip_local_header_t *header);

void free_local_headers(zip_local_header_t ***headers, int size);

void free_central_header(zip_central_header_t *header);

void free_central_headers(zip_central_header_t **headers, int size);

int set_local_header(FILE *fp, zip_local_header_t ***headers, int size);

int convert_bytes_to_int(unsigned char *src);

void close_zip_data();

int load_from_zip(const char *file_name, uncompress_data_set_t *data_set);

#endif // LOADER_H
