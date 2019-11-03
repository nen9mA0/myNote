#ifndef _MYBMP_H
#define _MYBMP_H
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef enum _my_bmp_compress
{
    NONE,
    RLE8,
    RLE4,
    MASK
} my_bmp_compress;


#pragma pack(push)
#pragma pack(1)

typedef struct _MY_RGBQUAD
{
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} MY_RGBQUAD;

typedef struct _mybmp_16bits_555_pixel
{
    union
    {
        struct
        {
            uint8_t r:5;
            uint8_t g:5;
            uint8_t b:5;
        } data;
        uint16_t pad;
    } pixel;
} my_bmp_16bits_555_pixel;

typedef struct _mybmp_16bits_565_pixel
{
    union
    {
        struct
        {
            uint8_t r:5;
            uint8_t g:6;
            uint8_t b:5;
        } data;
        uint16_t pad;
    } pixel;
} my_bmp_16bits_565_pixel;

typedef struct _mybmp_24bits_pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} mybmp_24bits_pixel;

typedef struct _mybmp_32bits_pixel
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t alpha;
} mybmp_32bits_pixel;

#pragma pack(pop)


#pragma pack(push)
#pragma pack(2)
typedef struct _bmp_config
{
    uint32_t length;
    uint32_t biwidth;
    uint16_t bitcount;
    my_bmp_compress compress;
    uint32_t xpels;
    uint32_t ypels;
    MY_RGBQUAD *color_table;
} bmp_config;

typedef struct _MY_BITMAPFILEHEADER
{
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
} MY_BITMAPFILEHEADER;

typedef struct _MY_BITMAPINFOHEADER
{
    uint32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} MY_BITMAPINFOHEADER;
#pragma pack(pop)


char* mybmp_create_header(bmp_config *pconfig, int *size);
int mybmp_create_24bits_bmp(char *name ,void *pic, int len, int width);
int mybmp_create_16bits_565_bmp(char *name ,void *pic, int len, int width);

#endif