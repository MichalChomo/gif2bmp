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

/**
 * Logical screen descriptor.
 */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t  packedField;
    uint8_t  bgColorIndex;
    uint8_t  pixelAspectRatio;
}tGifLsd;

/**
 * Graphics control extension.
 */
typedef struct {
    uint8_t  size;
    uint8_t  packedField;
    uint16_t delay;
    uint8_t  transColorIndex;
}tGifGce;

/**
 * Image descriptor.
 */
typedef struct {
    uint16_t left;
    uint16_t top;
    uint16_t width;
    uint16_t height;
    uint8_t  packedField;
}tGifImgDesc;

/**
 * Info from packed field byte of image descriptor.
 */
typedef struct {
    uint8_t  isLocalTable;
    uint8_t  isInterlace;
    uint8_t  isSort;
    uint16_t localTableSize;
}tGifImgInfo;

/**
 * Color of the pixel.
 */
typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
}tColor;

/**
 * Image descriptor, local color table and info.
 */
typedef struct {
    tGifImgDesc    desc;
    tColor        *localColorTable;
    tGifImgInfo    info;
}tGifImg;

/**
 * Info from packed field byte of logical screen descriptor and count of graphics
 * control extensions and images.
 */
typedef struct {
    uint8_t  isGlobalTable;
    uint8_t  isSort;
    uint16_t globalTableSize;
    uint8_t  gceCount;
    uint8_t  imgCount;
}tGifInfo;

/**
 * GIF image struct, contains all blocks and image data.
 */
typedef struct {
    uint8_t       header[GIF_HEADER_SIZE];
    tGifLsd       lsd; /**< Logical screen descriptor. */
    tColor       *globalColorTable;
    tGifGce      *gceArr; /**< Array of graphics control extensions. */
    tGifImg      *images; /**< Array of images. */
    tGifInfo      info;
    uint8_t      *colorIndexes; /**< Array of decoded color indexes. */
    uint32_t      colorIndexesSize;
}tGif;

/**
 * Parse data from buffer and store it to GIF struct.
 * @param[out] gif    Pointer to the GIF struct.
 * @param[in]  buffer Pointer to data.
 * @return 0 ok, 1 error.
 */
int parseGif(tGif *gif, uint8_t *buffer);

/**
 * Check if header matches with GIF 87 or 89 version.
 * @param[in] buffer Pointer to data.
 * @return 0 ok, 1 header doesn't match.
 */
int checkHeader(uint8_t *buffer);

/**
 * Parse logical screen descriptor data.
 * @param[out] lsd    Pointer to logical screen descriptor struct.
 * @param[in]  buffer Pointer to data.
 */
void getGifLsd(tGifLsd *lsd, uint8_t *buffer);

/**
 * Get size of the color table from packed field byte.
 * @param[in] pf Packed field byte.
 * @return Size of the color table.
 */
uint16_t getColorTableSize(uint8_t pf);

/**
 * Parse color table.
 * @param[out] ct     Pointer to the color table.
 * @param[in]  size   Color table size.
 * @param[in]  buffer Pointer to data.
 */
void getColorTable(tColor **ct, uint16_t size, uint8_t *buffer);

/**
 * Check if there is graphics control extension in data.
 * @param[in] buffer Pointer to data.
 * @return 1 there is GCE, 0 no GCE.
 */
int isGce(uint8_t *buffer);

/**
 * Parse graphics control extension.
 * @param[out] gce    Pointer to graphics control extension struct.
 * @param[in]  buffer Pointer to data.
 */
void getGce(tGifGce *gce, uint8_t *buffer);

/**
 * Skip application extension.
 * @param[in] buffer Pointer to data.
 */
void ignoreAppExt(uint8_t **buffer);

/**
 * Check if there is image descriptor in data.
 * @param[in] buffer Pointer to data.
 * @return 1 there is image descriptor, 0 no image descriptor.
 */
int isImgDesc(uint8_t *buffer);

/**
 * Parse image descriptor.
 * @param[out] gce    Pointer to image descriptor struct.
 * @param[in]  buffer Pointer to data.
 */
void getImgDesc(tGifImgDesc *id, uint8_t *buffer);

/**
 * Get information from packed field byte of image descriptor.
 * @param[out] info Pointer to image info struct.
 * @param[in]  pf   Packed field byte.
 */
void getImgInfo(tGifImgInfo *info, uint8_t pf);

/**
 * Decode LZW image data.
 * @param[in]  img    Pointer to the image struct.
 * @param[in]  buffer Pointer to data to decode.
 * @param[out] out    Pointer to pointer to decoded data.
 */
void decodeLzwData(tGifImg *img, uint8_t *buffer, uint8_t **out);

/**
 * Get the code from the data according to code size.
 * @param[in] buffer     Pointer to pointer to data.
 * @param[in] codeSize   Current code size.
 * @param[in] endOfBlock Indicator that buffer is going to reach end of block.
 * @return The code.
 */
uint16_t getCode(uint8_t **buffer, uint8_t codeSize, bool endOfBlock);

/**
 * Free all memory allocated for GIF struct.
 * @param[in] gif Pointer to GIF struct.
 */
void freeGif(tGif *gif);

#endif
