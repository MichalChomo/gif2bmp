#ifndef BMP_H
#define BMP_H 

#include "gif2bmp.h"
#include "gif.h"

#define BMP_HEADER_TYPE 0x4d42
#define BMP_DATA_OFFSET 0x28
#define BMP_BITCOUNT 0x18
#define BMP_PLANES 1
#define BMP_COMPRESS_METHOD 0
#define BMP_HRES 0
#define BMP_VRES 0
#define BMP_COLORS_USED 0
#define BMP_SIG_COLORS 0

typedef struct {
    uint16_t fileType;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
}tBmpHeader;

typedef struct {
    uint32_t headerSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitCount;
    uint32_t compression;
    uint32_t sizeImage;
    uint32_t hResolution;
    uint32_t vResolution;
    uint32_t colorsUsed;
    uint32_t significantColors;
}tBmpDib;

typedef struct {
    tBmpHeader header;
    tBmpDib    dib;
    uint8_t   *data;
}tBmp;

void initBmp(tBmp *bmp, tGif *gif);

void initBmpHeader(tBmpHeader *hdr);

void initBmpDibHeader(tBmpDib *dib, uint32_t width,
        uint32_t height);

void freeBmp(tBmp *bmp);

void writeBmpToFile(FILE *fp, tBmp *bmp);

#endif
