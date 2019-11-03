#include "mybmp.h"

char* mybmp_create_header(bmp_config *pconfig, int *size)
{
    MY_BITMAPFILEHEADER *pbmpheader;
    MY_BITMAPINFOHEADER *pbmpinfo;
    int color_table_len = 0;

    if(pconfig->bitcount <= 8)      //if have color table
        color_table_len = (1 << pconfig->bitcount) * sizeof(MY_RGBQUAD);
    
    int file_size = sizeof(MY_BITMAPFILEHEADER) + sizeof(MY_BITMAPINFOHEADER) + color_table_len + (pconfig->length)*(pconfig->bitcount/8);
    int header_size = sizeof(MY_BITMAPFILEHEADER) + sizeof(MY_BITMAPINFOHEADER) + color_table_len;
    *size = header_size;
    char *p = malloc( header_size );

    pbmpheader = (MY_BITMAPFILEHEADER*)p;
    pbmpheader->bfType = 0x4d42;
    pbmpheader->bfReserved1 = 0;
    pbmpheader->bfReserved2 = 0;
    pbmpheader->bfSize = file_size;
    pbmpheader->bfOffBits = header_size;

    pbmpinfo = (MY_BITMAPINFOHEADER*)(pbmpheader+1);
    pbmpinfo->biSize = 40;
    pbmpinfo->biPlanes = 1;
    pbmpinfo->biClrImportant = 0;
    pbmpinfo->biBitCount = pconfig->bitcount;
    pbmpinfo->biWidth = pconfig->biwidth;
    pbmpinfo->biHeight = pconfig->length / pconfig->biwidth;
    pbmpinfo->biCompression = pconfig->compress;
    pbmpinfo->biSizeImage = pconfig->length*pconfig->bitcount/8;
    pbmpinfo->biXPelsPerMeter = pconfig->xpels;
    pbmpinfo->biYPelsPerMeter = pconfig->ypels;
    pbmpinfo->biClrUsed = (1 << pconfig->bitcount) & 0xffffffff;

    return p;
}

int mybmp_create_24bits_bmp(char *name ,void *pic, int len, int width)
{
    bmp_config config;
    int size;
    char *p;
    int ret;

    config.bitcount = 24;
    config.biwidth = width;
    config.color_table = NULL;
    config.compress = NONE;
    
    config.xpels = 0x120b;
    config.ypels = 0x120b;
    config.length = len/sizeof(mybmp_24bits_pixel);

    p = mybmp_create_header(&config, &size);
    FILE *fp = fopen(name, "wb");
    if(fp != NULL)
    {
        fwrite(p,size,1,fp);
        fwrite(pic,len,1,fp);
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    
    free(p);
    return ret;
}


int mybmp_create_16bits_565_bmp(char *name ,void *pic, int len, int width)
{
    bmp_config config;
    int size;
    char *p;
    int ret;

    config.bitcount = 16;
    config.biwidth = width;
    config.color_table = NULL;
    config.compress = NONE;
    
    config.xpels = 0x120b;
    config.ypels = 0x120b;
    config.length = len/sizeof(my_bmp_16bits_565_pixel);

    p = mybmp_create_header(&config, &size);
    FILE *fp = fopen(name, "wb");
    if(fp != NULL)
    {
        fwrite(p,size,1,fp);
        fwrite(pic,len,1,fp);
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    
    free(p);
    return ret;
}