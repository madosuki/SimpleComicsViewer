#include "utils.h"

const short png_sig_size = 8;
const unsigned char png_sig[8] = {137, 80, 78, 71, 13, 10, 26, 10};

const short jpg_sig_size = 2;
const unsigned char jpg_sig[2] = {255, 216};

const short zlib_sig_size = 2; 
const unsigned char zlib_no_compression_or_low_sig[2] = {120, 1};
const unsigned char zlib_default_compression_sig[2] = {120, 156};
const unsigned char zlib_best_compression_sig[2] = {120, 218};

const uint8_t compress_headers_flag = UTILS_ZIP;

double mygcd(double x, double y)
{
    if(x < 0 || y == 0) {
        return 1;
    }

    if(x == y) {
        return 1;
    }

    double left = fmin(x, y);
    double right = fmod(fmax(x, y), fmin(x, y));
    while(1)
    {
        double tmp = fmod(left, right);
        if(tmp <= 0.0)
        {
            return right;
        }

        left = right;
        right = tmp;

    }
}

double *calc_aspect_raito(int width, int height, double gcd)
{
    double width_aspect = width / gcd;
    double height_aspect = height / gcd;
    printf("%f, %f, %f\n", width_aspect, height_aspect, gcd);

    double *tuple = (double*)calloc(2, sizeof(double));

    tuple[0] = width_aspect;
    tuple[1] = height_aspect;

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
    if(file_size < 4) {
        fclose(fp);
        close(fd);
        return 0;
    }

    uint16_t sig;
    fseek(fp, 0L, SEEK_SET);
    fread(&sig, 1, 2, fp);
    printf("%s : sig = %u\njpg sig: %u, png sig: %u\n", file_name, sig, *(uint32_t*)&jpg_sig, *(uint32_t*)&png_sig);

    if(sig == *(uint16_t*)&jpg_sig) {

        printf("detect jpeg : %s\n", file_name);

        fclose(fp);

        close(fd);

        return 1;

    } else {

        uint64_t tmp_sig;
        fseek(fp, 0L, SEEK_SET);
        fread(&tmp_sig, 1, 8, fp);
        if(tmp_sig == *(uint64_t*)&png_sig) {

            fclose(fp);

            close(fd);

            return 1;
        }
    }

    fclose(fp);

    close(fd);

    return 0;
}

uint8_t detect_compress_file(const char *file_name)
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
    if(file_size < 4) {
        fclose(fp);
        close(fd);
        return 0;
    }

    uint32_t sig;
    fread(&sig, 1, 4, fp);

    if(sig == 67324752)
    {
        fclose(fp);
        close(fd);

        return UTILS_ZIP;
    }

    fclose(fp);
    close(fd);

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
