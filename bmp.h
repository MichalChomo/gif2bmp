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

/**
 * BMP file header.
 */
typedef struct {
    uint16_t fileType;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t dataOffset;
}tBmpHeader;

/**
 * Device independent bitmap.
 */
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

/**
 * BMP image struct, contains header, DIB and image data.
 */
typedef struct {
    tBmpHeader header;
    tBmpDib    dib;
    uint8_t   *data;
}tBmp;

/**
 * Initialize BMP image and fill its data.
 * @param[in, out] bmp Pointer to BMP struct.
 * @param[in]      gif Pointer to GIF struct.
 */
void initBmp(tBmp *bmp, tGif *gif);

/**
 * Fill BMP header.
 * @param[out] hdr Pointer to BMP header.
 */
void initBmpHeader(tBmpHeader *hdr);

/**
 * Fill BMP device independent bitmap.
 * @param[out] dib    Pointer to DIB struct.
 * @param[in]  width  Image width.
 * @param[in]  height Image height.
 */
void initBmpDibHeader(tBmpDib *dib, uint32_t width,
        uint32_t height);

/**
 * Free all memory allocated for BMP struct.
 * @param[in] bmp Pointer to BMP struct.
 */
void freeBmp(tBmp *bmp);

/**
 * Write the BMP image to file.
 * @param[out] fp  Pointer to file.
 * @param[in]  bmp Pointer to BMP struct.
 */
void writeBmpToFile(FILE *fp, tBmp *bmp);

/**
 * Fill the BMP image data with the GIF image data.
 * @param[out] bmpData      Pointer to pointer to BMP data.
 * @param[in]  colorTable   Pointer to the color table.
 * @param[in]  colorIndexes Pointer to pointer to the color indexes.
 * @param[in]  height       Height of the image.
 * @param[in]  width        Width of the image.
 * @param[in]  padding      BMP data has to be aligned to 4 bytes, so if
 *                          width * 3 is not a multiple of 4, it has to be
 *                          padded.
 */
void fillBmpData(uint8_t **bmpData, tColor *colorTable, uint8_t **colorIndexes, uint16_t height, uint16_t width, uint8_t padding);

/**
 * Fill the BMP image data with the GIF image data that are interlaced.
 * @param[out] bmpData      Pointer to pointer to BMP data.
 * @param[in]  colorTable   Pointer to the color table.
 * @param[in]  colorIndexes Pointer to pointer to the color indexes.
 * @param[in]  height       Height of the image.
 * @param[in]  width        Width of the image.
 * @param[in]  padding      BMP data has to be aligned to 4 bytes, so if
 *                          width * 3 is not a multiple of 4, it has to be
 *                          padded.
 */
void fillBmpDataInterlace(uint8_t **bmpData, tColor *colorTable, uint8_t **colorIndexes, uint16_t height, uint16_t width, uint8_t padding);

#endif
