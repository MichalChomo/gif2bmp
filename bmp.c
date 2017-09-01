#include "bmp.h"

void initBmp(tBmp *bmp, tGif *gif) {
    uint8_t *colorIndexesPtr = NULL;
    uint8_t *dataStart = NULL;
    uint32_t bmpDataSize = 0;
    // tColor is 3 bytes, so if width * 3 is not a multiple of 4, padding has
    // to be added so the bmp data is aligned to multiple of 4.
    uint8_t padding = (4 - ((gif->lsd).width * sizeof(tColor))) % 4;

    initBmpHeader(&(bmp->header));
    initBmpDibHeader(&(bmp->dib), (gif->lsd).width, -(gif->lsd).height);

    // Allocate memory, include padding.
    bmpDataSize = gif->colorIndexesSize * sizeof(tColor)
            + padding * (gif->lsd).height;
    (bmp->header).size += bmpDataSize;
    (bmp->dib).sizeImage = bmpDataSize;

    bmp->data = malloc(bmpDataSize);
    memset(bmp->data, 0, bmpDataSize);
    dataStart = bmp->data;
    colorIndexesPtr = gif->colorIndexes;
    for (uint16_t i = 0; i < (gif->lsd).height; ++i) {
        for (uint16_t j = 0; j < (gif->lsd).width; ++j) {
            *(bmp->data) = ((gif->globalColorTable)[*colorIndexesPtr]).blue;
            ++(bmp->data);
            *(bmp->data) = ((gif->globalColorTable)[*colorIndexesPtr]).green;
            ++(bmp->data);
            *(bmp->data) = ((gif->globalColorTable)[*colorIndexesPtr]).red;
            ++(bmp->data);
            ++colorIndexesPtr;
        }
        bmp->data += padding;
    }
    bmp->data = dataStart;
}

void initBmpHeader(tBmpHeader *hdr) {
    hdr->fileType = BMP_HEADER_TYPE;
    hdr->size = sizeof(tBmpHeader) + sizeof(tBmpDib);
    hdr->reserved1 = 0;
    hdr->reserved2 = 0;
    hdr->dataOffset = BMP_DATA_OFFSET;
}

void initBmpDibHeader(tBmpDib *dib, uint32_t width,
        uint32_t height) {
    dib->headerSize = sizeof(tBmpDib);
    dib->width = width;
    dib->height = height;
    dib->planes = BMP_PLANES;
    dib->bitCount = BMP_BITCOUNT;
    dib->compression = BMP_COMPRESS_METHOD;
    dib->hResolution = BMP_HRES;
    dib->vResolution = BMP_VRES;
    dib->colorsUsed = BMP_COLORS_USED;
    dib->significantColors = BMP_SIG_COLORS;
}

void freeBmp(tBmp *bmp) {
    free(bmp->data);
}

void writeBmpToFile(FILE *fp, tBmp *bmp) {
    uint16_t word = 0;
    uint32_t dWord = 0;

    word = (bmp->header).fileType;
    fwrite(&word, sizeof(uint16_t), 1, fp);
    dWord = (bmp->header).size;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    word = (bmp->header).reserved1;
    fwrite(&word, sizeof(uint16_t), 1, fp);
    word = (bmp->header).reserved2;
    fwrite(&word, sizeof(uint16_t), 1, fp);
    dWord = (bmp->header).dataOffset;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);

    dWord = (bmp->dib).headerSize;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).width;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).height;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    word = (bmp->dib).planes;
    fwrite(&word, sizeof(uint16_t), 1, fp);
    word = (bmp->dib).bitCount;
    fwrite(&word, sizeof(uint16_t), 1, fp);
    dWord = (bmp->dib).compression;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).sizeImage;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).hResolution;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).vResolution;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).colorsUsed;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);
    dWord = (bmp->dib).significantColors;
    fwrite(&dWord, sizeof(uint32_t), 1, fp);

    fwrite(bmp->data, sizeof(uint8_t), (bmp->dib).sizeImage, fp);
}
