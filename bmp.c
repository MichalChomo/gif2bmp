#include "bmp.h"

void initBmp(tBmp *bmp, tGif *gif) {
    uint8_t *dataStart = NULL;
    uint8_t *colorIndexesPtr = NULL;
    tColor *colorTable = NULL;
    uint32_t bmpDataSize = 0;
    // tColor is 3 bytes, so if width * 3 is not a multiple of 4, padding has
    // to be added so the bmp data is aligned to multiple of 4.
    uint8_t padding = (4 - ((gif->lsd).width * sizeof(tColor))) % 4;

    initBmpHeader(&(bmp->header));
    initBmpDibHeader(&(bmp->dib), (gif->lsd).width, -(gif->lsd).height);

    // Compute the data size, include padding.
    bmpDataSize = gif->colorIndexesSize * sizeof(tColor)
            + padding * (gif->lsd).height;
    (bmp->header).size += bmpDataSize;
    (bmp->dib).sizeImage = bmpDataSize;

    bmp->data = malloc(bmpDataSize);
    memset(bmp->data, 0, bmpDataSize);
    dataStart = bmp->data;
    colorIndexesPtr = gif->colorIndexes;
    for (uint8_t i = 0; i < (gif->info).imgCount; ++i) {
        if ((gif->images)[i].info.isLocalTable > 0) {
            colorTable = (gif->images)[i].localColorTable;
        } else if ((gif->info).isGlobalTable > 0) {
            colorTable = gif->globalColorTable;
        } else {
            fprintf(stderr, "No color table.\n");
        }
        if ((gif->images)[i].info.isInterlace == 0) {
            fillBmpData(&(bmp->data), colorTable, &colorIndexesPtr,
                    (gif->images)[i].desc.height, (gif->images)[i].desc.width,
                    padding);
        } else {
            fillBmpDataInterlace(&(bmp->data), colorTable, &colorIndexesPtr,
                    (gif->images)[i].desc.height, (gif->images)[i].desc.width,
                    padding);
        }
    }
    bmp->data = dataStart;
}

void fillBmpData(uint8_t **bmpData, tColor *colorTable, uint8_t **colorIndexes,
        uint16_t height, uint16_t width, uint8_t padding) {
    uint16_t index = 0;
    for (uint16_t i = 0; i < height; ++i) {
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[(i * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[(i * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[(i * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
    }
}

void fillBmpDataInterlace(uint8_t **bmpData, tColor *colorTable, uint8_t **colorIndexes, uint16_t height, uint16_t width, uint8_t padding) {
    uint16_t index = 0;
    // First pass - 0th lines from each strip.
    for (uint16_t i = 0; i < height; i += 8) {
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[(i * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[(i * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[(i * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
    }
    // Second pass - 4th lines from each strip.
    for (uint16_t i = 4; i < height; i += 8) {
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[(i * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[(i * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[(i * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
    }
    // Third pass - 2nd and 6th lines from each strip.
    for (uint16_t i = 2; i < height; i += 8) {
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[(i * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[(i * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[(i * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
        if ((i + 4) >= height) {
            break;
        }
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
    }
    // Fourth pass - 1st, 3rd, 5th and 7th lines from each strip.
    for (uint16_t i = 1; i < height; i += 8) {
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[(i * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[(i * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[(i * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
        if ((i + 2) >= height) {
            break;
        }
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[((i + 2) * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[((i + 2) * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[((i + 2) * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
        if ((i + 4) >= height) {
            break;
        }
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[((i + 4) * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
        if ((i + 6) >= height) {
            break;
        }
        for (uint16_t j = 0; j < width; ++j) {
            index = j * 3;
            (*bmpData)[((i + 6) * (width * 3 + padding)) + index] =
                (colorTable[**colorIndexes]).blue;
            (*bmpData)[((i + 6) * (width * 3 + padding)) + index + 1] =
                (colorTable[**colorIndexes]).green;
            (*bmpData)[((i + 6) * (width * 3 + padding)) + index + 2] =
                (colorTable[**colorIndexes]).red;
            ++(*colorIndexes);
        }
    }
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
