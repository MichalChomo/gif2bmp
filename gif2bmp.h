//  Name: Michal Chomo
// Login: xchomo01
//  Date: 5.5.2017
//
//  Header file with constants, data structures and function definitions.

#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

#define LOGIN "xchomo01"

#define GIF_HEADER_SIZE 6
#define GIF_HEADER_89 "GIF89a"
#define GIF_HEADER_87 "GIF87a"
#define GIF_EXT_SEPARATOR 0xf921
#define GIF_IMG_SEPARATOR 0x2c
#define GIF_TERMINATOR 0x3b
#define GIF_GCT_FLAG 0x80
#define GIF_SORT_FLAG 0x08
#define GIF_IMG_LCT_FLAG 0x08
#define GIF_IMG_INTERLACE_FLAG 0x04
#define GIF_IMG_SORT_FLAG 0x02

#define BUFFER_PART 256

#define LZW_MAX_CODE_SIZE 12

typedef struct {
    int64_t bmpSize;
    int64_t gifSize;
}tGIF2BMP;

typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  packedField;
    uint8_t  bgColorIndex;
    uint8_t  pixelAspectRatio;
}tGifLsd;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}tColor;

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
    tGifImgDesc    desc;
    tColor        *localColorTable;
    uint8_t *data;
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
    uint8_t header[GIF_HEADER_SIZE];
    tGifLsd       lsd;
    tColor       *globalColorTable;
    tGifGce      *gceArr;
    tGifImg      *images;
    tGifInfo      info;
}tGif;

typedef struct {
    uint8_t  *colorIndexes;
    uint8_t   size;
}tDictRow;

typedef struct {
    tDictRow **rows;
    uint16_t   size;
    uint16_t   insertIndex;
}tDict;

/**
 * Convert image in .gif format to .bmp format.
 * @param gif2bmp Sizes of both files.
 * @param inputFile
 * @param outpuFile
 * @return 0 OK, 1 error
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

void loadFileToBuffer(uint8_t **buffer, FILE *fp, int64_t size);


/**
 * @param fileName Name of the log file.
 * @param uncodedSize GIF file size.
 * @param codedSize BMP file size.
 * @return 0 success, 1 fail.
 */
int createLogfile(char *fileName, tGIF2BMP *g2b);

/**
 * Get the size of the file in bytes.
 * @para fileName Name of the file.
 * @return Size of the file.
 */
int64_t getFileSize(const char *fileName);

void dictInit(tDict *dict, uint16_t size);

void dictInsert(tDict *dict, tDictRow *row);

void dictSearch(tDict *dict, uint16_t index, tDictRow **row);

void dictDestroy(tDict *dict);

void dictResize(tDict *dict, uint16_t size);

int parseGif(tGif *gif, uint8_t *buffer);

int checkHeader(uint8_t *buffer);

void getGifLsd(tGifLsd *lsd, uint8_t *buffer);

uint16_t getColorTableSize(uint8_t pf);

void getColorTable(tColor **ct, uint16_t size, uint8_t *buffer);

void printColorTable(tColor *ct, uint16_t size);

int isGce(uint8_t *buffer);

void getGce(tGifGce *gce, uint8_t *buffer);

int isImgDesc(uint8_t *buffer);

void getImgDesc(tGifImgDesc *id, uint8_t *buffer);

void getImgInfo(tGifImgInfo *info, uint8_t pf);

void decodeLzwData(tGifImg *img, uint8_t *buffer, uint8_t **out);

uint16_t getCode(uint8_t **buffer, uint8_t codeSize);

tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k);

void freeGif(tGif *gif);
#endif
