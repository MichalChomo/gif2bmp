#ifndef GIF_H
#define GIF_H

#include "gif2bmp.h"
#include "dictionary.h"

#define GIF_HEADER_SIZE 6
#define GIF_HEADER_89 "GIF89a"
#define GIF_HEADER_87 "GIF87a"
#define GIF_GC_EXT_SEPARATOR 0xf921
#define GIF_APP_EXT_SEPARATOR 0xff21
#define GIF_IMG_SEPARATOR 0x2c
#define GIF_TERMINATOR 0x3b
#define GIF_GCT_FLAG 0x80
#define GIF_SORT_FLAG 0x08
#define GIF_IMG_LCT_FLAG 0x80
#define GIF_IMG_INTERLACE_FLAG 0x40
#define GIF_IMG_SORT_FLAG 0x20

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  packedField;
    uint8_t  bgColorIndex;
    uint8_t  pixelAspectRatio;
}tGifLsd;

typedef struct {
    uint8_t  size;
    uint8_t  packedField;
    uint16_t delay;
    uint8_t  transColorIndex;
}tGifGce;

typedef struct {
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
    uint8_t  packedField;
}tGifImgDesc;

typedef struct {
    uint8_t  isLocalTable;
    uint8_t  isInterlace;
    uint8_t  isSort;
    uint16_t localTableSize;
}tGifImgInfo;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}tColor;

typedef struct {
    tGifImgDesc    desc;
    tColor        *localColorTable;
    tGifImgInfo    info;
}tGifImg;

typedef struct {
    uint8_t  isGlobalTable;
    uint8_t  isSort;
    uint16_t globalTableSize;
    uint8_t  gceCount;
    uint8_t  imgCount;
}tGifInfo;

typedef struct {
    uint8_t       header[GIF_HEADER_SIZE];
    tGifLsd       lsd;
    tColor       *globalColorTable;
    tGifGce      *gceArr;
    tGifImg      *images;
    tGifInfo      info;
    uint8_t      *colorIndexes;
    uint32_t      colorIndexesSize;
}tGif;

int parseGif(tGif *gif, uint8_t *buffer);

int checkHeader(uint8_t *buffer);

void getGifLsd(tGifLsd *lsd, uint8_t *buffer);

uint16_t getColorTableSize(uint8_t pf);

void getColorTable(tColor **ct, uint16_t size, uint8_t *buffer);

void printColorTable(tColor *ct, uint16_t size);

int isGce(uint8_t *buffer);

void getGce(tGifGce *gce, uint8_t *buffer);

void ignoreAppExt(uint8_t **buffer);

int isImgDesc(uint8_t *buffer);

void getImgDesc(tGifImgDesc *id, uint8_t *buffer);

void getImgInfo(tGifImgInfo *info, uint8_t pf);

void decodeLzwData(tGifImg *img, uint8_t *buffer, uint8_t **out);

uint16_t getCode(uint8_t **buffer, uint8_t codeSize, bool endOfBlock);

tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k);

void freeGif(tGif *gif);

void copyRow(tDictRow *dest, tDictRow *src);

#endif
