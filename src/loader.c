#include "loader.h"

const int eocd_size = 22;

int finally_uncompress_size = 0;

int *get_zip_central_directory_offset(FILE *fp, const int file_size)
{
  fseek(fp, file_size - 22, SEEK_SET);
  uint32_t eocd_sig;
  int read_count = fread(&eocd_sig, 1, 4, fp);

  uint16_t disk_num;
  read_count = fread(&disk_num, 1, 2, fp);

  uint16_t central_directory_start_positon;
  read_count = fread(&central_directory_start_positon, 1, 2, fp);

  uint16_t cd_num_on_disk;
  read_count = fread(&cd_num_on_disk, 1, 2, fp);

  uint16_t total_cd_record_num;
  read_count = fread(&total_cd_record_num, 1, 2, fp);

  uint32_t cd_size;
  read_count = fread(&cd_size, 1, 4, fp);

  uint32_t offset;
  read_count = fread(&offset, 1, 4, fp);

  int *tuple = (int*)calloc(2, sizeof(int));
  tuple[0] = cd_num_on_disk;
  tuple[1] = offset;

  return tuple;
}

void set_zip_central_headers(zip_central_header_t **headers, int central_offset, int cd_num_on_disk, int file_size, FILE *fp)
{
  int pos = 0;
  int current = central_offset + pos;
  fseek(fp, central_offset, SEEK_SET);
  uint32_t total_uncompress_file_size = 0;
  for(int i = 0; i < cd_num_on_disk; i++) {

    zip_central_header_t *tmp = (zip_central_header_t*)calloc(sizeof(zip_central_header_t), sizeof(zip_central_header_t));


    uint32_t c_sig;
    int read_count = fread(&c_sig, 1, 4, fp);
    tmp->signature = c_sig;

    uint16_t version;
    read_count = fread(&version, 1, 2, fp);
    tmp->version = version;

    uint16_t minimum_version;
    read_count = fread(&minimum_version, 1, 2, fp);
    tmp->minimum_version = minimum_version;

    uint16_t bitflag;
    read_count = fread(&bitflag, 1, 2, fp);
    tmp->bitflag = bitflag;

    uint16_t compress_method;
    read_count = fread(&compress_method, 1, 2, fp);
    tmp->compress_method = compress_method;

    uint16_t latest_modify_time;
    read_count = fread(&latest_modify_time, 1, 2, fp);
    tmp->latest_modify_time = latest_modify_time;

    uint16_t latest_modify_date;
    read_count = fread(&latest_modify_date, 1, 2, fp);
    tmp->latest_modify_date = latest_modify_date;

    uint32_t crc32;
    read_count = fread(&crc32, 1, 4, fp);
    tmp->crc32 = crc32;

    uint32_t compress_size;
    read_count = fread(&compress_size, 1, 4, fp);
    tmp->compress_size = compress_size;

    uint32_t uncompress_size;
    read_count = fread(&uncompress_size, 1, 4, fp);
    tmp->uncompress_size = uncompress_size;

    total_uncompress_file_size += uncompress_size;

    uint16_t file_name_length;
    read_count = fread(&file_name_length, 1, 2, fp);
    tmp->file_name_length = file_name_length;

    uint16_t extra_field_length;
    read_count = fread(&extra_field_length, 1, 2, fp);
    tmp->extra_field_length = extra_field_length;

    uint16_t comment_length;
    read_count = fread(&comment_length, 1, 2, fp);
    pos += (uint32_t)comment_length;
    tmp->file_comment_length = comment_length;

    uint16_t start_disk_num;
    read_count = fread(&start_disk_num, 1, 2, fp);
    tmp->file_start_disk_num = start_disk_num;

    uint16_t internal_file_attribute;
    read_count = fread(&internal_file_attribute, 1, 2, fp);
    tmp->internal_file_attribute = internal_file_attribute;

    uint32_t external_file_attribute;
    read_count = fread(&external_file_attribute, 1, 4, fp);
    tmp->external_file_attribute = external_file_attribute;

    uint32_t offset_relative_local_header;
    read_count = fread(&offset_relative_local_header, 1, 4, fp);
    tmp->offset_relative_local_header = offset_relative_local_header;

    if(file_name_length > 0) {
      uint8_t *file_name = (uint8_t*)calloc(file_name_length, 1);
      read_count = fread(file_name, 1, file_name_length, fp);

      tmp->file_name = (uint8_t*)calloc(file_name_length, 1);
      memcpy(tmp->file_name, file_name, file_name_length);

      free(file_name);
      file_name = NULL;
    }

    if(extra_field_length > 0) {
      uint8_t *extra_field = (uint8_t*)calloc(extra_field_length, 1);
      read_count = fread(extra_field, 1, extra_field_length, fp);

      tmp->extra_field = (uint8_t*)calloc(extra_field_length, 1);
      memcpy(tmp->extra_field, extra_field, extra_field_length);

      free(extra_field);
      extra_field = NULL;
    }

    if(comment_length > 0) {
      uint8_t *file_comment = (uint8_t*)calloc(comment_length, 1);
      read_count = fread(file_comment, 1, comment_length, fp);

      tmp->file_comment = (uint8_t*)calloc(comment_length, 1);
      memcpy(tmp->file_comment, file_comment, comment_length);

      free(file_comment);
      file_comment = NULL;
    }

    headers[i] = tmp;

  }

  finally_uncompress_size = total_uncompress_file_size;
}

void free_zip_local_header(zip_local_header_t *header)
{
  if(header->file_name != NULL) {
    free(header->file_name);
    header->file_name = NULL;
  }

  if(header->extra_field != NULL) {
    free(header->extra_field);
    header->extra_field = NULL;
  }
}

void free_zip_local_headers(zip_local_header_t ***headers, int size)
{
  if(*headers != NULL) {
    for(int i = 0; i < size; ++i) {
      if(*headers[i] != NULL) {
        free_zip_local_header(*headers[i]);

        free(*headers[i]);

        *headers[i] = NULL;
      }
    }

    free(*headers);
  }
}

void free_zip_central_header(zip_central_header_t *header)
{
  if(header != NULL) {
    if(header->file_name != NULL) {
      free(header->file_name);
      header->file_name = NULL;
    }

    if(header->extra_field != NULL) {
      free(header->extra_field);
      header->extra_field = NULL;
    }

    if(header->file_comment != NULL) {
      free(header->file_comment);
      header->file_comment = NULL;
    }

    free(header);
    header = NULL;
  }
}

void free_zip_central_headers(zip_central_header_t **headers, int size)
{
  if(headers != NULL) {
    for(int i = 0; i < size; i++) {
      if(headers[i] != NULL) {
        free_zip_central_header(headers[i]);
      }
    }

    free(headers);
    headers = NULL;
  }
}

int set_zip_local_header(FILE *fp, zip_local_header_t ***headers, int size)
{

  // set signature
  uint8_t signature[4];
  int read_count = fread(signature, 1, 4, fp);
  if(*(uint32_t*)&signature != 67324752)
  {
    printf("not local header\n");
    return 0;
  }

  zip_local_header_t *tmp = (zip_local_header_t*)calloc(sizeof(zip_local_header_t), sizeof(zip_local_header_t));

  tmp->signature = *(uint32_t*)&signature;

  // set version
  uint8_t version[2];
  read_count = fread(version, 1, 2, fp);
  tmp->version = *(uint16_t*)&version;


  // set bitflag
  uint8_t bitflag[2];
  read_count = fread(bitflag, 1, 2, fp);
  tmp->bitflag = *(uint16_t*)&bitflag;

  // set compress method
  uint8_t compress_method[2];
  read_count = fread(compress_method, 1, 2, fp);
  tmp->compress_method = *(uint16_t*)&compress_method;

  // set latest_modify_time
  uint8_t latest_modify_time[2];
  read_count = fread(latest_modify_time, 1, 2, fp);
  tmp->latest_modify_time = *(uint16_t*)&latest_modify_time;

  // set latest_modify_date
  uint8_t latest_modify_date[2];
  read_count = fread(latest_modify_date, 1, 2, fp);
  tmp->latest_modify_date = *(uint16_t*)&latest_modify_date;

  // set crc32
  uint8_t crc32[4];
  read_count = fread(crc32, 1, 4, fp);
  tmp->crc32 = *(uint32_t*)&crc32;

  // set compress size
  uint8_t compress_size[4];
  read_count = fread(compress_size, 1, 4, fp);
  tmp->compress_size = *(uint32_t*)&compress_size;

  // set uncompress_size
  uint8_t uncompress_size[4];
  read_count = fread(uncompress_size, 1, 4, fp);
  tmp->uncompress_size = *(uint32_t*)&uncompress_size;

  // set file name length
  uint8_t file_name_length[2];
  read_count = fread(file_name_length, 1, 2, fp);
  tmp->file_name_length = *(uint16_t*)&file_name_length;

  // set extra field length
  uint8_t extra_field_length[2];
  read_count = fread(extra_field_length, 1, 2, fp);
  tmp->extra_field_length = *(uint16_t*)&extra_field_length;

  // set file name
  tmp->file_name = (uint8_t*)calloc(tmp->file_name_length, 1);
  read_count = fread(tmp->file_name, 1, tmp->file_name_length, fp);

  // set extrat field
  if(extra_field_length > 0) {
    tmp->extra_field = (uint8_t*)calloc(tmp->extra_field_length, 1);
    read_count = fread(tmp->extra_field, 1, tmp->extra_field_length, fp);
  }
  else {
    tmp->extra_field = NULL;
  }

  zip_local_header_t **check = (zip_local_header_t**)realloc(*headers, size + 1);
  if(check != NULL) {
    *headers = check;
    *headers[size] = tmp;

    return size + 1;
  }

  free(check);

  return 0;
}

int convert_bytes_to_int(unsigned char *src)
{
  int result = src[3] << 24 |
    src[2] << 16 |
    src[1] << 8 |
    src[0];

  return result;
}

int load_from_zip(const char *file_name, uncompress_data_set_t *data_set)
{
  if(data_set == NULL) {
    printf("data_set is NULL\n");
    return 0;
  }

  FILE *fp;

  int fd = open(file_name, O_RDONLY);
  if(fd < 0) {
    printf("File Destructor Open Error\n");
    return 0;
  }

  fp = fdopen(fd, "rb");
  if (fp == NULL) {
    printf("fdopen Error\n");
    close(fd);
    return 0;
  }

  struct stat stbuf;
  fstat(fd, &stbuf);

  long file_size = stbuf.st_size;
  if(file_size < 100) {
    fclose(fp);
    close(fd);
    return 0;
  }

  int *eocd_data = get_zip_central_directory_offset(fp, file_size);
  int offset = eocd_data[1];
  int cd_num_on_disk = eocd_data[0];
  free(eocd_data);

  zip_central_header_t **headers = (zip_central_header_t**)calloc(cd_num_on_disk, sizeof(zip_central_header_t*));

  set_zip_central_headers(headers, offset, cd_num_on_disk, file_size, fp);

  data_set->uncompress_data_list = (uncompress_data_t**)malloc(sizeof(uncompress_data_t*) * cd_num_on_disk);
  memset(data_set->uncompress_data_list, 0, cd_num_on_disk);

  if(data_set->uncompress_data_list == NULL) {
    fclose(fp);
    close(fd);

    free_zip_central_headers(headers, cd_num_on_disk);

    free_uncompress_data_set(data_set);

    printf("allocate error from data_set->uncompress_data_list\n");
    return 0;
  }

  int i = 0;
  int position = 0;
  while(i < cd_num_on_disk) {
    uint32_t local_header_offset = headers[i]->offset_relative_local_header;

    // set file name length offset
    fseek(fp, local_header_offset + 26, SEEK_SET);

    uint16_t file_name_length;
    int read_count = fread(&file_name_length, 1, 2, fp);
    uint16_t extra_field_length;
    read_count = fread(&extra_field_length, 1, 2, fp);

    uint8_t *tmp_file_name = (uint8_t*)calloc(file_name_length, 1);
    if(tmp_file_name == NULL) {
      free(tmp_file_name);
      tmp_file_name = NULL;

      close(fd);
      fclose(fp);

      free_zip_central_headers(headers, cd_num_on_disk);

      return 0;
    }
    read_count = fread(tmp_file_name, 1, file_name_length, fp);

    fseek(fp, extra_field_length, SEEK_CUR);

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;

    uint8_t *body = (uint8_t*)calloc(headers[i]->compress_size, 1);
    if(body == NULL) {

      free(tmp_file_name);
      tmp_file_name = NULL;


      free(body);
      body = NULL;
      printf("allocate error from body\n");

      free_zip_central_headers(headers, cd_num_on_disk);

      free_uncompress_data_set(data_set);

      fclose(fp);

      close(fd);

      return 0;
    }

    read_count = fread(body, 1, headers[i]->compress_size, fp);
    if(ferror(fp)) {
      printf("file error\n");

      free(tmp_file_name);
      tmp_file_name = NULL;


      free(body);
      body = NULL;

      free_zip_central_headers(headers, cd_num_on_disk);

      free_uncompress_data_set(data_set);

      fclose(fp);

      close(fd);

      return 0;
    }


    uint8_t *out = (uint8_t*)calloc(headers[i]->uncompress_size, 1);
    if(out == NULL) {
      free(tmp_file_name);
      tmp_file_name = NULL;

      free(body);
      body = NULL;

      free(out);
      out = NULL;

      free_zip_central_headers(headers, cd_num_on_disk);

      free_uncompress_data_set(data_set);

      fclose(fp);

      close(fd);

      printf("allocate error from out\n");
      return 0;
    }

    if(headers[i]->compress_method == COMPRESS_METHOD_DEFLATE) {

      strm.avail_in = headers[i]->compress_size;
      strm.next_in = body;
      strm.avail_out = headers[i]->uncompress_size;
      strm.next_out = out;

      int ret = inflateInit2(&strm, -MAX_WBITS);

      ret = inflate(&strm, Z_NO_FLUSH);
      switch(ret) {
        case Z_NEED_DICT:
          printf("Z_NEED_DICT\n");
          inflateEnd(&strm);

          free(tmp_file_name);
          tmp_file_name = NULL;

          free(body);
          body = NULL;

          if(out != NULL) {
            free(out);
            out = NULL;
          }

          free_zip_central_headers(headers, cd_num_on_disk);

          free_uncompress_data_set(data_set);

          fclose(fp);

          close(fd);

          return 0;

        case Z_DATA_ERROR:
          printf("%s\n", body);

          inflateEnd(&strm);

          if(out != NULL) {
            free(out);
            out = NULL;
          }

          free(tmp_file_name);
          tmp_file_name = NULL;

          free(body);
          body = NULL;

          free_uncompress_data_set(data_set);

          free_zip_central_headers(headers, cd_num_on_disk);


          fclose(fp);

          close(fd);

          return 0;

        case Z_MEM_ERROR:
          printf("Z_MEM_ERROR\n");
          inflateEnd(&strm);

          if(out != NULL) {
            free(out);
            out = NULL;
          }

          free(tmp_file_name);
          tmp_file_name = NULL;

          free(body);
          body = NULL;

          free_zip_central_headers(headers, cd_num_on_disk);

          free_uncompress_data_set(data_set);


          fclose(fp);

          close(fd);

          return 0;
      }
    } else {
      memcpy(out, body, headers[i]->uncompress_size);

      free(body);
      body = NULL;
    }

    int check = detect_image(out, headers[i]->uncompress_size);

    if(check == UTILS_PNG || check == UTILS_JPG) {
      data_set->uncompress_data_list[position] = (uncompress_data_t*)calloc(sizeof(uncompress_data_t), sizeof(uncompress_data_t));
      if(data_set->uncompress_data_list[position] != NULL) {

        data_set->uncompress_data_list[position]->file_name = (uint8_t*)calloc(file_name_length, 1);
        memcpy(data_set->uncompress_data_list[position]->file_name, tmp_file_name, file_name_length);

        data_set->uncompress_data_list[position]->data = (uint8_t*)calloc(headers[i]->uncompress_size, 1);
        memcpy(data_set->uncompress_data_list[position]->data, out, headers[i]->uncompress_size);

        data_set->uncompress_data_list[position]->file_size = headers[i]->uncompress_size;

        position++;

        data_set->size = position;
      }
    }


    if(out != NULL) {
      free(out);
      out = NULL;
    }

    inflateEnd(&strm);

    free(tmp_file_name);
    tmp_file_name = NULL;

    free(body);
    body = NULL;

    i++;
  }


  free_zip_central_headers(headers, cd_num_on_disk);

  fclose(fp);

  close(fd);

  return 1; 
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
