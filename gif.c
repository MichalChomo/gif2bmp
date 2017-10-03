#include "gif.h"

int parseGif(tGif *gif, uint8_t *buffer) {
    tGifLsd lsd;
    tColor *tablePtr = NULL;
    tGifGce gce;
    tGifGce *gceArrPtr = NULL;
    tGifImg img;
    tGifImg *imgPtr = NULL;
    uint32_t imgSize = 0;
    uint8_t *colorIndexesStart = NULL;

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

    // Ignore application extension.
    ignoreAppExt(&buffer);

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
            buffer += img.info.localTableSize * sizeof(tColor);
        }
        imgSize = (img.desc).width * (img.desc).height;
        gif->colorIndexes = malloc(imgSize);
        if (colorIndexesStart == NULL) {
            colorIndexesStart = gif->colorIndexes;
        }
        memset(gif->colorIndexes, 0, imgSize);
        gif->colorIndexesSize += imgSize;

        decodeLzwData(&img, buffer, &(gif->colorIndexes));
        memcpy(gif->images, &img, sizeof(tGifImg));
    }
    gif->colorIndexes = colorIndexesStart;

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
    uint16_t separator = GIF_GC_EXT_SEPARATOR;

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

void ignoreAppExt(uint8_t **buffer) {
    uint16_t separator = GIF_APP_EXT_SEPARATOR;
    uint8_t blockSize = 0;
    uint8_t appDataSize = 0;

    if (0 != memcmp(*buffer, &separator, sizeof(uint8_t))) {
        return;
    }

    *buffer += 2;
    blockSize = **buffer;
    *buffer += blockSize + 1;
    while (0 != (appDataSize = **buffer)) {
        *buffer += appDataSize + 1;
    }
    ++(*buffer);
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
    tDictRow prevRow;
    uint8_t k = 0;
    uint16_t code = 0;
    uint16_t blockSize = 0;
    uint16_t clearCode = 0;
    uint16_t endCode = 0;
    uint8_t origCodeSize = 0;
    uint8_t codeSize = 0;
    uint8_t *bufferStart = NULL;
    uint8_t *outStart = *out;
    bool bufferOverBlock = false;

    prevRow.colorIndexes = NULL;

    codeSize = *buffer;
    ++buffer; // Code size.
    ++codeSize; // Increment because of clear code and EOI code.
    origCodeSize = codeSize;

    clearCode = 1 << (codeSize - 1);
    endCode = clearCode + 1;

    blockSize = *buffer;
    ++buffer; // Block size.

    dictInit(&dict, 1 << codeSize);
    bufferStart = buffer;
    // Loop over blocks.
    while (blockSize != 0) {
        bufferStart = buffer;
        if (bufferOverBlock) {
            // Skip over the first byte after block size byte, because it has 
            // already been read.
            ++buffer;
            bufferOverBlock = false;
        }
        while ((buffer - bufferStart) < blockSize) {
            if ((buffer - bufferStart) > (blockSize - 2)) {
                code = getCode(&buffer, codeSize, true);
            } else {
                code = getCode(&buffer, codeSize, false);
            }
            if (code == clearCode) {
                dict.insertIndex = endCode + 1;
                codeSize = origCodeSize;
                dictResize(&dict, (1 << codeSize));
                code = getCode(&buffer, codeSize, false);
                dictSearch(&dict, code, &row);
                if (row != NULL) {
                    memcpy(*out, row->colorIndexes,
                            row->size * sizeof(uint8_t));
                    *out += row->size;
                    copyRow(&prevRow, row);
                }
            } else if (code == endCode) {
                free(prevRow.colorIndexes);
                dictDestroy(&dict); 
                return;
            } else {
                // Lookup the code in the dictionary.
                dictSearch(&dict, code, &row);
                if (row != NULL) {
                    memcpy(*out, row->colorIndexes,
                            row->size * sizeof(uint8_t));
                    k = **out; // First color index from current row.
                    *out += row->size;
                    dictInsert(&dict, createRowToAdd(&prevRow, k));
                    copyRow(&prevRow, row);
                } else {
                    k = prevRow.colorIndexes[0];
                    memcpy(*out, prevRow.colorIndexes,
                            prevRow.size * sizeof(uint8_t));
                    *out += prevRow.size;
                    **out = k;
                    *out += 1;
                    row = createRowToAdd(&prevRow, k);
                    copyRow(&prevRow, row);
                    dictInsert(&dict, row);
                }
            }
            if (dict.insertIndex >= (1 << codeSize) &&
                    codeSize < LZW_MAX_CODE_SIZE) {
                ++codeSize;
                dictResize(&dict, (1 << codeSize));
            }
        }
        if (((buffer - bufferStart) > blockSize)) {
            // Buffer pointer is over the block size byte.
            bufferOverBlock = true;
            --buffer;
        }
        blockSize = *buffer;
        ++buffer;
    }
    free(prevRow.colorIndexes);
    dictDestroy(&dict);
}

uint16_t getCode(uint8_t **buffer, uint8_t codeSize, bool endOfBlock) {
    static uint16_t word = 0;
    static uint8_t bitsRead = 16; // Init to 16, so new word is read at start.
    uint16_t mask = (1 << codeSize) - 1; // Mask is codeSize count of 1's.
    uint16_t code = 0;

    // For testing, there's no other way to reinit static variables.
    if (codeSize == 0xff) {
        word = 0;
        bitsRead = 0;
        return 0;
    }

    if ((16 - bitsRead) >= codeSize) {
        // All code bits are in the current word.
        code = word;
        word >>= codeSize; // Remove used code bits from the word.
        bitsRead += codeSize;
    } else {
        // New word has to be read.
        code = word; // Use bits left in the current word.
        memcpy(&word, *buffer, sizeof(uint16_t));
        *buffer += 2;
        if (endOfBlock) {
            uint8_t byte = word; // Save the first byte of the read word.
            // Read a byte from another block and add it to the word.
            memcpy(&word, *buffer, sizeof(uint8_t));
            word <<= 8;
            word |= byte;
        }
        // Add needed code bits from the new word.
        code |= word << (16 - bitsRead);
        bitsRead = codeSize - (16 - bitsRead);
        word >>= bitsRead; // Remove used code bits from the new word.
    }
    code &= mask; // Remove spare bits from the code.
    
    return code;
}

tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k) {
    tDictRow *rowToAdd = NULL;

    if (prevRow == NULL) {
        fprintf(stderr, "Previous row is null.\n");
        return NULL;
    }

    rowToAdd = malloc(sizeof(tDictRow));
    if (rowToAdd == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return NULL;
    }
    rowToAdd->size = prevRow->size + 1;
    rowToAdd->colorIndexes = malloc(rowToAdd->size * sizeof(uint8_t));
    if (rowToAdd->colorIndexes == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return NULL;
    }
    memset(rowToAdd->colorIndexes, 0, rowToAdd->size * sizeof(uint8_t));
    memcpy(rowToAdd->colorIndexes, prevRow->colorIndexes,
            prevRow->size * sizeof(uint8_t));
    rowToAdd->colorIndexes += prevRow->size;
    memcpy(rowToAdd->colorIndexes, &k, sizeof(uint8_t));
    rowToAdd->colorIndexes -= prevRow->size;

    return rowToAdd;
}

void freeGif(tGif *gif) {
    if ((gif->info).isGlobalTable > 0) {
        free(gif->globalColorTable);
    }
    for (int i = 0; i < (gif->info).imgCount; ++i) {
        if ((gif->images)[i].info.isLocalTable > 0) {
            free((gif->images)[i].localColorTable);
        }
    }
    free(gif->images);
    free(gif->gceArr);
    free(gif->colorIndexes);
}

void copyRow(tDictRow *dest, tDictRow *src) {
    if (dest->colorIndexes != NULL) {
        free(dest->colorIndexes);
    }
    dest->size = src->size;
    dest->colorIndexes = malloc(dest->size * sizeof(uint8_t));
    if (dest->colorIndexes == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return;
    }
    memcpy(dest->colorIndexes, src->colorIndexes,
            src->size * sizeof(uint8_t));
}
