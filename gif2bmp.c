//  Name: Michal Chomo
// Login: xchomo01
//  Date: 5.5.2017
//
//  Definitions of functions for GIF to BMP conversion.

#include "gif2bmp.h"

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {
    uint8_t *buffer = NULL;
    uint8_t *bufferStart = NULL;
    tGif gif;
    tBmp bmp;

    loadFileToBuffer(&buffer, inputFile, gif2bmp->gifSize);
    bufferStart = buffer;

    for (int i = 0; i < gif2bmp->gifSize - 1; ++i) {
        printf("%02x ", *buffer);
        ++buffer;
    }
        printf("%02x ", *buffer);
    buffer = bufferStart;

    if (0 != parseGif(&gif, buffer)) {
        fprintf(stderr, "Error parsing gif file\n");
        free(bufferStart);
        return 1;
    }

    initBmp(&bmp, &gif);
    writeBmpToFile(outputFile, &bmp);

    freeGif(&gif);
    freeBmp(&bmp);
    free(bufferStart);

    return 0;
}

void loadFileToBuffer(uint8_t **buffer, FILE *fp, int64_t size) {
    uint8_t *bufferStart = *buffer;

    if (fp != stdin) {
        *buffer = malloc(size);
        memset(*buffer, 0, size);
        bufferStart = *buffer;
        fread(*buffer, size, 1, fp);
    } else {
        int c = 0;
        int i = 0;
        *buffer = malloc(BUFFER_PART);
        memset(*buffer, 0, BUFFER_PART);
        bufferStart = *buffer;

        c = fgetc(stdin);
        while (c != EOF && c != GIF_TERMINATOR) {
            **buffer = (uint8_t) c;
            ++(*buffer);
            ++i;
            if (i % BUFFER_PART == 0) {
                buffer = realloc(*buffer, ((i / BUFFER_PART) + 1)
                        * BUFFER_PART);
                *buffer += i / BUFFER_PART;
            }
            fgetc(stdin);
        }
    }

    *buffer = bufferStart;
}

int createLogfile(char *fileName, tGIF2BMP *g2b) {
    FILE *lfp;

    lfp = fopen(fileName, "w");
    if (lfp == NULL) {
        fprintf(stderr, "Cannot open logfile %s.\n", fileName);
        return 1;
    }
    fprintf(lfp, "login = %s\nuncodedSize = %ld\ncodedSize = %ld\n", LOGIN,
            g2b->gifSize, g2b->bmpSize);
    fclose(lfp);

    return 0;
}

int64_t getFileSize(const char *fileName) {
    struct stat st;

    if (0 != stat(fileName, &st)) {
        fprintf(stderr, "Cannot get file %s size.\n", fileName);
        return -1;
    }

    return (int64_t)st.st_size;
}

void dictInit(tDict *dict, uint16_t size) {
    tDictRow row;
    uint16_t i = 0;

    row.size = 1;
    dict->rows = malloc(size * sizeof(tDictRow *));
    memset(dict->rows, 0, size * sizeof(tDictRow *));
    for (; i < size / 2; ++i) {
        row.colorIndexes = malloc(sizeof(uint8_t));
        *(row.colorIndexes) = i;
        ((dict->rows)[i]) = malloc(sizeof(tDictRow));
        *((dict->rows)[i]) = row;
    }
    for (; i < size; ++i) {
        ((dict->rows)[i]) = NULL;
    }

    dict->size = size;
    dict->insertIndex = (size / 2) + 2;
}

void dictInsert(tDict *dict, tDictRow *row) {
    if (dict == NULL || row == NULL) {
        return;
    }
    if (dict->insertIndex < dict->size) {
        ((dict->rows)[dict->insertIndex]) = row;
        ++(dict->insertIndex);
    }
}

void dictSearch(tDict *dict, uint16_t index, tDictRow **row) {
    if (dict == NULL || index > dict->size) {
        return;
    }
    *row = ((dict->rows)[index]);
}

void dictDestroy(tDict *dict) {
    if (dict == NULL) {
        return;
    }
    tDictRow *row;

    for (uint16_t i = 0; i < dict->size; ++i) {
        row = (dict->rows)[i];
        if (row != NULL) {
            if (row->colorIndexes != NULL) {
                free(row->colorIndexes);
            }
            free(row);
        }
    }
    free(dict->rows);
}

void dictResize(tDict *dict, uint16_t size) {
    dict->rows = realloc(dict->rows, size * sizeof(tDictRow *));
    for (uint16_t i = size / 2; i < size; ++i) {
        ((dict->rows)[i]) = NULL;
    }
    dict->size = size;
}

int parseGif(tGif *gif, uint8_t *buffer) {
    tGifLsd lsd;
    tColor *tablePtr = NULL;
    tGifGce gce;
    tGifGce *gceArrPtr = NULL;
    tGifImg img;
    tGifImg *imgPtr = NULL;
    uint32_t imgSize = 0;

    memset(gif, 0, sizeof(tGif));
    // Parse header.
    if (checkHeader(buffer) != 0) {
        fprintf(stderr, "Wrong input file format.\n");
        return 1;
    }
    memcpy(&(gif->header), buffer, GIF_HEADER_SIZE);
    buffer += GIF_HEADER_SIZE;

    // Parse logical screen descriptor.
    getGifLsd(&lsd, buffer);
    buffer += sizeof(tGifLsd) - 1;
    (gif->info).isGlobalTable = lsd.packedField & GIF_GCT_FLAG;
    (gif->info).isSort = lsd.packedField & GIF_SORT_FLAG;
    gif->lsd = lsd;

    // Parse global color table.
    if ((gif->info).isGlobalTable > 0) {
        (gif->info).globalTableSize = getColorTableSize(lsd.packedField);
        getColorTable(&(tablePtr), (gif->info).globalTableSize,
                buffer);
        gif->globalColorTable = tablePtr;
        buffer += (gif->info).globalTableSize * sizeof(tColor);
    }

    // Parse graphics control extensions.
    while (isGce(buffer)) {
        buffer += 2;
        ++((gif->info).gceCount);
        gif->gceArr = realloc(gif->gceArr, (gif->info).gceCount
                * sizeof(tGifGce));
        gceArrPtr = gif->gceArr;
        gceArrPtr += (gif->info).gceCount - 1;
        getGce(&gce, buffer);
        memcpy(gceArrPtr, &gce, sizeof(tGifGce));
        buffer += sizeof(tGifGce);
    }

    // Parse image descriptors and image data.
    while (isImgDesc(buffer)) {
        ++buffer;
        ++((gif->info).imgCount);
        gif->images = realloc(gif->images, (gif->info).imgCount
                * sizeof(tGifImg));
        imgPtr = gif->images;
        imgPtr += (gif->info).imgCount - 1;
        getImgDesc(&(img.desc), buffer);
        buffer += sizeof(tGifImgDesc) - 1;
        getImgInfo(&(img.info), img.desc.packedField);
        if (img.info.isLocalTable > 0) {
            img.info.localTableSize = getColorTableSize(img.desc.packedField);
            getColorTable(&(img.localColorTable), img.info.localTableSize,
                    buffer);
            buffer += img.info.localTableSize * sizeof(tColor) - 1;
        }
        imgSize = (img.desc).width * (img.desc).height;
        printf("debug size %d\n", imgSize);
        gif->colorIndexes = malloc(imgSize);
        memset(gif->colorIndexes, 0, imgSize);
        gif->colorIndexesSize += imgSize;

        decodeLzwData(&img, buffer, &(gif->colorIndexes));
        printf("debug buffer parse %02x\n", *buffer);
        memcpy(gif->images, &img, sizeof(tGifImg));
    }

    return 0;
}

int checkHeader(uint8_t *buffer) {
    if (0 != memcmp(buffer, GIF_HEADER_89, GIF_HEADER_SIZE)
         && 0 != memcmp(buffer, GIF_HEADER_87, GIF_HEADER_SIZE)) {
        return 1;
    }

    return 0;
}

void getGifLsd(tGifLsd *lsd, uint8_t *buffer) {
    memcpy(&(lsd->width), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(lsd->height), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(lsd->packedField), buffer, sizeof(uint8_t));
    ++buffer;
    memcpy(&(lsd->bgColorIndex), buffer, sizeof(uint8_t));
    ++buffer;
    memcpy(&(lsd->pixelAspectRatio), buffer, sizeof(uint8_t));
}

uint16_t getColorTableSize(uint8_t pf) {
    // Get last 3 bits from packed field.
    uint8_t n = pf & 7;
    uint16_t res = 1;
    // Return size, it equals 2^(n + 1).
    return res << (n + 1);
}

void getColorTable(tColor **ct, uint16_t size, uint8_t *buffer) {
    tColor c;
    tColor *ctStart = NULL;

    *ct = malloc(size * sizeof(tColor));
    ctStart = *ct;
    while(size-- > 0) {
        c.red = *buffer;
        ++buffer;
        c.green = *buffer;
        ++buffer;
        c.blue = *buffer;
        ++buffer;
        **ct = c;
        ++(*ct);
    }

    *ct = ctStart;
}

void printColorTable(tColor *ct, uint16_t size) {
    tColor c;

    printf("Global color table:\n");
    for (; size > 0; --size) {
        c = *ct;
        printf("%02x %02x %02x||", c.red, c.green, c.blue);
        ++ct;
    }
    printf("\n");
}

int isGce(uint8_t *buffer) {
    uint16_t separator = GIF_EXT_SEPARATOR;

    if (0 == memcmp(buffer, &separator, sizeof(uint16_t))) {
        return 1;
    } else {
        return 0;
    }
}

void getGce(tGifGce *gce, uint8_t *buffer) {
    gce->size = *buffer;
    ++buffer;
    gce->packedField = *buffer;
    ++buffer;
    memcpy(&(gce->delay), buffer, sizeof(uint16_t));
    buffer += 2;
    gce->transColorIndex = *buffer;
}

int isImgDesc(uint8_t *buffer) {
    uint8_t separator = GIF_IMG_SEPARATOR;

    if (0 == memcmp(buffer, &separator, sizeof(uint8_t))) {
        return 1;
    } else {
        return 0;
    }
}

void getImgDesc(tGifImgDesc *id, uint8_t *buffer) {
    memcpy(&(id->left), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(id->top), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(id->width), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(id->height), buffer, sizeof(uint16_t));
    buffer += 2;
    id->packedField = *buffer;
}

void getImgInfo(tGifImgInfo *info, uint8_t pf) {
    info->isLocalTable = pf & GIF_IMG_LCT_FLAG;
    info->isInterlace = pf & GIF_IMG_INTERLACE_FLAG;
    info->isSort = pf & GIF_IMG_SORT_FLAG;
    info->localTableSize = getColorTableSize(pf);
}

void decodeLzwData(tGifImg *img, uint8_t *buffer, uint8_t **out) {
    tDict dict;
    tDictRow *row = NULL;
    tDictRow *prevRow = NULL;
    uint8_t k = 0;
    uint16_t code = 0;
    uint16_t prevCode = 0;
    uint16_t blockSize = 0;
    uint16_t clearCode = 0;
    uint16_t endCode = 0;
    uint8_t codeSize = 0;
    uint8_t *bufferStart = NULL;
    uint8_t *outStart = *out;

    codeSize = *buffer + 1;
    ++buffer; // Code size.
    if (codeSize == 2) {
        codeSize += 2;
    }
    clearCode = 1 << (codeSize - 1);
    endCode = clearCode + 1;

    blockSize = *buffer;
    ++buffer; // Block size.

        printf("debug block size %02x\n", blockSize);
    dictInit(&dict, 1 << codeSize);
    // Loop over blocks.
    while(blockSize != 0) {
        bufferStart = buffer;
        while ((buffer - bufferStart) < blockSize) {
            code = getCode(&buffer, codeSize);
            if (code == clearCode) {
            printf("debug CLEAR &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& \n");
                // Reinitialize the dictionary.
                dictDestroy(&dict);
                dictInit(&dict, 1 << codeSize);
                code = getCode(&buffer, codeSize);
                    printf("debug first CODE %04x\n", code);
                dictSearch(&dict, code, &row);
                if (row != NULL) {
                    memcpy(*out, row->colorIndexes, row->size * sizeof(uint8_t));
                    *(out) += row->size;
                }
            } else if (code == endCode) {
                    printf("debug END CODE ****************************\n");
                code = 0;
                break;
            } else {
                // Lookup the code in the dictionary.
                    printf("debug CODE %04x\n", code);
                dictSearch(&dict, code, &row);
                if (row != NULL) {
                    memcpy(*out, row->colorIndexes, row->size * sizeof(uint8_t));
                    k = **out; // First color index from current row.
                    *(out) += row->size;
                    dictSearch(&dict, prevCode, &prevRow);
                    dictInsert(&dict, createRowToAdd(prevRow, k));
                } else {
                    dictSearch(&dict, prevCode, &prevRow);
                    k = *(prevRow->colorIndexes);
                    memcpy(*out, prevRow->colorIndexes, prevRow->size * sizeof(uint8_t));
                    *(out) += prevRow->size;
                    memcpy(*out, &k, sizeof(uint8_t));
                    *(out) += 1;
                    dictInsert(&dict, createRowToAdd(prevRow, k));
                }
                    printf("debug END BUFFER %02x %02x ---------------------------\n", *(buffer + 0),*(buffer + 1));
            }
            if (dict.insertIndex >= (1 << codeSize)
                    && codeSize < LZW_MAX_CODE_SIZE) {
                printf("debug RESIZE++++++++++++++++++++++++++++++++++++++\n");
                ++codeSize;
                dictResize(&dict, (1 << codeSize));
            }
            prevCode = code;
            printf("debug dict index %d\n", dict.insertIndex);
        }
        blockSize = *buffer;
                    printf("debug END BLOCKSIZE %04x ---------------------------\n", blockSize);
                    printf("debug END BUFFER %02x %02x ---------------------------\n", *(buffer + 0),*(buffer + 1));
        ++buffer;
    }
    dictDestroy(&dict);

        printf("\nOUT\n");
                for (int j = 0; j < (img->desc).width * (img->desc).height; ++j) {
                    printf("%02x ", *(outStart + j));
                }
        printf("\nOUT\n");
    *out = outStart;
}

uint16_t getCode(uint8_t **buffer, uint8_t codeSize) {
    static uint16_t word = 0;
    static uint8_t bitsRead = 0;
    // Mask is codeSize count of 1's.
    uint16_t mask = (1 << codeSize) - 1;
    uint16_t code = 0;

    if (bitsRead == 0) {
        memcpy(&word, *buffer, sizeof(uint16_t));
        *(buffer) += 2;
    }
    if ((16 - bitsRead) >= codeSize) {
        // All code bits are in the current word.
        code = word & mask;
        // Remove used code bits from the word.
        word >>= codeSize;
        bitsRead += codeSize;
        if (bitsRead == 16) {
            // All code bits from the word were used, set bitsRead to 0, so on
            // the next call, new word will be read.
            bitsRead = 0;
        }
    } else {
        // New word has to be read.
        // Use bits left in the current word.
        code = word;
        memcpy(&word, *buffer, sizeof(uint16_t));
        *(buffer) += 2;
        // Add needed code bits from the new word.
        code |= word << (16 - bitsRead);
        code &= mask;
        bitsRead = codeSize - (16 - bitsRead);
        // Remove used code bits from the new word.
        word >>= bitsRead;
    }
    
    return code;
}

tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k) {
    tDictRow *rowToAdd = NULL;

    rowToAdd = malloc(sizeof(tDictRow));
    rowToAdd->size = prevRow->size + 1;
    rowToAdd->colorIndexes = malloc(rowToAdd->size * sizeof(uint8_t));
    memset(rowToAdd->colorIndexes, 0, rowToAdd->size * sizeof(uint8_t));
    memcpy(rowToAdd->colorIndexes, prevRow->colorIndexes,
            prevRow->size * sizeof(uint8_t));
    rowToAdd->colorIndexes += prevRow->size;
    memcpy(rowToAdd->colorIndexes, &k, sizeof(uint8_t));
    rowToAdd->colorIndexes -= prevRow->size;

    return rowToAdd;
}

void freeGif(tGif *gif) {
    free(gif->globalColorTable);
    free(gif->gceArr);
    free(gif->images);
    free(gif->colorIndexes);
}

void initBmp(tBmp *bmp, tGif *gif) {
    uint8_t *data = NULL;
    uint8_t *dataStart = NULL;
    uint16_t bmpDataSize = 0;

    initBmpHeader(&(bmp->header));
    initBmpDibHeader(&(bmp->dib), (gif->lsd).width, (gif->lsd).height);

    bmpDataSize = gif->colorIndexesSize * sizeof(tColor);
    (bmp->header).size += bmpDataSize;
    (bmp->dib).sizeImage = bmpDataSize;

    data = malloc(bmpDataSize);
    memset(data, 0, bmpDataSize);
    dataStart = data;
    bmp->data = malloc(bmpDataSize);
    memset(bmp->data, 0, bmpDataSize);
    for (uint16_t i = 0; i < gif->colorIndexesSize; ++i) {
        *data = ((gif->globalColorTable)[i]).red;
        ++data;
        *data = ((gif->globalColorTable)[i]).green;
        ++data;
        *data = ((gif->globalColorTable)[i]).blue;
        ++data;
    }

    memcpy(bmp->data, dataStart, bmpDataSize);
    free(dataStart);
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
