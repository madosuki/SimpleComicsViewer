#include "utils.h"

const short png_sig_size = 8;
const unsigned char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

const short jpg_sig_size = 2;
const unsigned char jpg_sig[2] = {0xFF, 0xD8};

/* const short zlib_sig_size = 2; */
/* const unsigned char zlib_no_compression_or_low_sig[2] = {120, 1}; */
/* const unsigned char zlib_default_compression_sig[2] = {120, 156}; */
/* const unsigned char zlib_best_compression_sig[2] = {120, 218}; */

const uint8_t compress_headers_flag = UTILS_ZIP;

int get_hash(uint8_t *bytes, const ssize_t bytes_size, uint8_t *result)
{
  
  if(bytes == NULL || bytes_size < 0 || result == NULL) {
    return 0;
  }

  /*
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, bytes, bytes_size);
  SHA256_Final(result, &ctx);
  */

  OpenSSL_add_all_digests();

  EVP_MD_CTX *md_ctx = EVP_MD_CTX_create();
  const EVP_MD *md = EVP_sha256();

  EVP_DigestInit_ex(md_ctx, md, NULL);
  EVP_DigestUpdate(md_ctx, bytes, bytes_size);

  uint32_t result_size = 0;
  EVP_DigestFinal_ex(md_ctx, result, &result_size);
  EVP_MD_CTX_destroy(md_ctx);
  EVP_cleanup();
  
  return 1;
}

double *calc_aspect_raito(int width, int height)
{
  double for_width_aspect = (double)width / height;
  double for_height_aspect = (double)height / width;

  double *tuple = (double*)calloc(2, sizeof(double));

  tuple[0] = for_width_aspect;
  tuple[1] = for_height_aspect;

  return tuple;
}

int detect_image(uint8_t *buf, int size)
{
  if(size < jpg_sig_size) {
    return 0;
  }

  int count = 0;
  if(buf[0] == png_sig[0]) {
    count++;

    for(int i = 1; i < png_sig_size; ++i) {
      if(buf[i] == png_sig[i]) {
        count++;
      }
    }

    if(count == png_sig_size) {
      return UTILS_PNG;
    }

  } else if(buf[0] == jpg_sig[0]) {
    count++;

    for(int i = 1; i < jpg_sig_size; ++i) {
      if(buf[i] == jpg_sig[i]) {
        count++;
      }
    }

    if(count == jpg_sig_size) {
      return UTILS_JPG;
    }
  }

  return 0;
}

int detect_image_from_file(const char *file_name)
{
  FILE *fp;

  fp = fopen(file_name, "rb");
  if(fp == NULL) {
    printf("fopen error in detect_image_from_file\n");
    return 0;
  }

  struct stat stbuf;
  if(fstat(fileno(fp), &stbuf) != 0) {
    printf("fstat error in detect_image_from_file=n");
    fclose(fp);
    return 0;
  }

  if(!S_ISREG(stbuf.st_mode)) {
    fclose(fp);
    return 0;
  }

  if(stbuf.st_size < 4) {
    printf("this file size is too small in detect_image_from_file\n");
    fclose(fp);
    return 0;
  }
  
  uint16_t sig;
  fseek(fp, 0L, SEEK_SET);
  int read_count = fread(&sig, 1, 2, fp);

  if(sig == *(uint16_t*)&jpg_sig) {
    printf("detect jpg: %s, sig: %x\n", file_name, sig);
    
    fclose(fp);
    
    return 1;

  } else {

    uint64_t tmp_sig;
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&tmp_sig, 1, 8, fp);
    if(tmp_sig == *(uint64_t*)&png_sig) {
      printf("detect png: %s, sig: %x\n", file_name, sig);

      fclose(fp);

      return 1;
    }
  }

  fclose(fp);

  return 0;
}

int detect_compress_file(const char *file_name)
{
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
  if(file_size < 9) {
    fclose(fp);
    close(fd);
    return 0;
  }

  /* uint64_t sig; */
  /* fseek(fp, 0L, SEEK_SET); */
  /* int read_count = fread(&sig, 1, 4, fp); */

  uint8_t sig[8];
  fseek(fp, 0L, SEEK_SET);
  int read_count = fread(sig, 1, 4, fp);


  const uint8_t zip_sig[4] = {0x50, 0x4B, 0x03, 0x04};
  const uint8_t rar_sig_1[7] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x00};
  const uint8_t rar_sig_2[8] = {0x52, 0x61, 0x72, 0x21, 0x1A, 0x07, 0x01, 0x00};
  const uint8_t seven_z_sig[4] = {0x37, 0x7A, 0xBC, 0xAF};
  const uint8_t gz_sig[2] = {0x1F, 0x8B};
  const uint8_t xz_sig[6] = {0xFD, 0x37, 0x7A, 0x58, 0x5A, 0x00};
  
  int is_compress = 0;

  if(*(uint32_t*)sig == *(uint32_t*)zip_sig) {
    is_compress = 1;
  }

  if(!is_compress) {
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&sig, 1, 6, fp);
    int count = 0;
    for(int i = 0; i < 6; ++i) {
      if(sig[i] == xz_sig[i])
        ++count;
      else
        break;
    }

    if(count == 6)
      is_compress = 1;
  }


  if(!is_compress) {
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&sig, 1, 7, fp);
    int count = 0;
    for(int i = 0; i < 7; ++i) {
      if(sig[i] == rar_sig_1[i])
        ++count;
      else
        break;
    }

    if(count == 7)
      is_compress = 1;
  }

  if(!is_compress) {
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&sig, 1, 8, fp);
    if(*(uint64_t*)sig == *(uint64_t*)rar_sig_2) {
      is_compress = 1;
    }
  }

  if(!is_compress) {
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&sig, 1, 4, fp);
    if(*(uint32_t*)sig == *(uint32_t*)seven_z_sig) {
      is_compress = 1;
    }
  }

  
  if(!is_compress) {
    fseek(fp, 0L, SEEK_SET);
    read_count = fread(&sig, 1, 2, fp);
    int count = 0;
    for(int i = 0; i < 2; ++i) {
      if(sig[i] == gz_sig[i])
        ++count;
      else
        break;
    }

    if(count == 2)
      is_compress = 1;
  }


  fclose(fp);
  close(fd);

  
  if(is_compress) {
    return 1;
  }

  return 0;
}

char *get_directory_path_from_filename(const char *file_name)
{
  int size = strlen(file_name) + 1;


  int count = 0;
  for(int i = size; i >= 0; i--) {
    if(file_name[i] == '/') {
      break;
    }

    count++;
  }

  char *tmp_name = (char*)calloc((size - count) + 1, 1);
  if(tmp_name != NULL) {
    strncpy(tmp_name, file_name, size - count);
  }

  return tmp_name;
}
