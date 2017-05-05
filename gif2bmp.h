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

#define DICT_LEAF_COUNT 256

#define BUFFER_PART 256

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
    uint8_t isLocalTable;
    uint8_t isInterlace;
    uint8_t isSort;
    uint16_t localTableSize;
}tGifImgInfo;

typedef struct {
    tGifImgDesc   desc;
    tColor        *localColorTable;
    unsigned char *data;
    tGifImgInfo   info;
}tGifImg;

typedef struct {
    uint8_t  isGlobalTable;
    uint8_t  isSort;
    uint16_t globalTableSize;
    uint8_t  gceCount;
    uint8_t  imgCount;
}tGifInfo;

typedef struct {
    unsigned char header[GIF_HEADER_SIZE];
    tGifLsd       lsd;
    tColor        *globalColorTable;
    tGifGce       *gceArr;
    tGifImg       *images;
    tGifInfo      info;
}tGif;

typedef struct tDictNode {
    struct tDictNode *children[DICT_LEAF_COUNT];
    bool   isLeaf;
}tDictNode;

/**
 * Convert image in .gif format to .bmp format.
 * @param gif2bmp Sizes of both files.
 * @param inputFile
 * @param outpuFile
 * @return 0 OK, 1 error
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

void loadFileToBuffer(unsigned char **buffer, FILE *fp, int64_t size);


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

tDictNode *createNode();

void insert(tDictNode *root, unsigned char *key, int8_t keyLength);

void destroy(tDictNode *root);

int parseGif(tGif *gif, unsigned char *buffer);

int checkHeader(unsigned char *buffer);

void getGifLsd(tGifLsd *lsd, unsigned char *buffer);

uint16_t getTableSize(uint8_t pf);

void getGlobalTable(tColor **gct, uint16_t size, unsigned char *buffer);

void printGct(tColor *gct, uint16_t size);

int isGce(unsigned char *buffer);

void getGce(tGifGce *gce, unsigned char *buffer);

int isImgDesc(unsigned char *buffer);

void getImgDesc(tGifImgDesc *id, unsigned char *buffer);

void getImgInfo(tGifImgInfo *info, uint8_t pf);

void freeGif(tGif *gif);
#endif
