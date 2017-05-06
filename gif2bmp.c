#include "gif2bmp.h"

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {
    unsigned char *buffer = NULL;
    unsigned char *bufferStart = NULL;
    tGif gif;

    loadFileToBuffer(&buffer, inputFile, gif2bmp->gifSize);
    bufferStart = buffer;

    for (int i = 0; i < gif2bmp->gifSize - 1; ++i) {
        printf("%02x ", *buffer);
        ++buffer;
    }
        printf("%02x ", *buffer);
    buffer = bufferStart;

    if (0 != parseGif(&gif, buffer)) {
        printf("ERROR bitch\n\n");
        free(bufferStart);
        return 1;
    }


    freeGif(&gif);
    free(bufferStart);

    return 0;
}

void loadFileToBuffer(unsigned char **buffer, FILE *fp, int64_t size) {
    unsigned char *bufferStart = *buffer;

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
            **buffer = (unsigned char) c;
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

    row.size = 1;
    dict->rows = malloc(2 * size * sizeof(tDictRow *));
    memset(dict->rows, 0, 2 * size * sizeof(tDictRow *));
    for (uint16_t i = 0; i < size; ++i) {
        row.colorIndexes = malloc(sizeof(uint16_t));
        *(row.colorIndexes) = i;
        ((dict->rows)[i]) = malloc(sizeof(tDictRow));
        *((dict->rows)[i]) = row;
    }
    for (uint16_t i = size; i < size * 2; ++i) {
        ((dict->rows)[i]) = NULL;
    }

    dict->size = 2 * size;
}

void dictInsert(tDict *dict, uint16_t index, uint16_t colorIndex) {
    tDictRow *row;

    if (index > dict->size) {
        return;
    }
    row = (dict->rows)[index];
    if (row == NULL) {
        row == malloc(sizeof(tDictRow));
    }
    row->colorIndexes = realloc(row->colorIndexes, row->size + 1);
    row->colorIndexes += row->size;
    memcpy(&(row->colorIndexes), &colorIndex, sizeof(uint16_t));
    ++(row->size);

    ++(dict->size);
}

int dictSearch(tDict *dict, uint16_t index) {
    if (index > dict->size) {
        return -1;
    }
    if ((dict->rows)[index] != NULL) {
        return 1;
    } else {
        return 0;
    }
}

void dictDestroy(tDict *dict) {
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

int parseGif(tGif *gif, unsigned char *buffer) {
    tGifLsd lsd;
    tGifGce gce;
    tGifGce *gceArrPtr = NULL;
    tGifImg img;
    tGifImg *imgPtr = NULL;
    uint16_t word = 1;

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
    buffer += sizeof(tGifLsd);
    (gif->info).isGlobalTable = lsd.packedField & GIF_GCT_FLAG;
    (gif->info).isSort = lsd.packedField & GIF_SORT_FLAG;
    gif->lsd = lsd;

    // Parse global color table.
    if ((gif->info).isGlobalTable > 0) {
        (gif->info).globalTableSize = getColorTableSize(lsd.packedField);
        getColorTable(&(gif->globalColorTable), (gif->info).globalTableSize,
                buffer);
        buffer += (gif->info).globalTableSize * sizeof(tColor) - 1;
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

    // Parse image descriptors.
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
    }

    img.info.lzwMinCodeSize = word << (*buffer);
    ++buffer;
    decodeLzwData(&img, buffer);

    return 0;
}

int checkHeader(unsigned char *buffer) {
    if (0 != memcmp(buffer, GIF_HEADER_89, GIF_HEADER_SIZE)
         && 0 != memcmp(buffer, GIF_HEADER_87, GIF_HEADER_SIZE)) {
        return 1;
    }

    return 0;
}

void getGifLsd(tGifLsd *lsd, unsigned char *buffer) {
    memcpy(&(lsd->width), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(lsd->height), buffer, sizeof(uint16_t));
    buffer += 2;
    memcpy(&(lsd->packedField), buffer, sizeof(uint8_t));
    ++buffer;
    memcpy(&(lsd->bgColorIndex), buffer, sizeof(uint8_t));
    ++buffer;
    memcpy(&(lsd->pixelAspectRatio), buffer, sizeof(uint8_t));
    ++buffer;
}

uint16_t getColorTableSize(uint8_t pf) {
    // Get last 3 bits from packed field.
    uint8_t n = pf & 7;
    uint16_t res = 1;
    // Return size, it equals 2^(n + 1).
    return res << (n + 1);
}

void getColorTable(tColor **ct, uint16_t size, unsigned char *buffer) {
    tColor c;
    tColor *ctStart = NULL;

    *ct = malloc(size * sizeof(tColor));
    ctStart = *ct;
    for (; size > 0; --size) {
        c.red = *buffer;
        ++buffer;
        c.green = *buffer;
        ++buffer;
        c.blue = *buffer;
        ++buffer;
        memcpy(*ct, &c, sizeof(tColor));
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

int isGce(unsigned char *buffer) {
    uint16_t separator = GIF_EXT_SEPARATOR;

    if (0 == memcmp(buffer, &separator, sizeof(uint16_t))) {
        return 1;
    } else {
        return 0;
    }
}

void getGce(tGifGce *gce, unsigned char *buffer) {
    gce->size = *buffer;
    ++buffer;
    gce->packedField = *buffer;
    ++buffer;
    memcpy(&(gce->delay), buffer, sizeof(uint16_t));
    buffer += 2;
    gce->transColorIndex = *buffer;
}

int isImgDesc(unsigned char *buffer) {
    uint8_t separator = GIF_IMG_SEPARATOR;

    if (0 == memcmp(buffer, &separator, sizeof(uint8_t))) {
        return 1;
    } else {
        return 0;
    }
}

void getImgDesc(tGifImgDesc *id, unsigned char *buffer) {
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

void decodeLzwData(tGifImg *img, unsigned char *buffer) {
    tDict dict;

    dictInit(&dict, img->info.lzwMinCodeSize);
    dictDestroy(&dict); 
}

void freeGif(tGif *gif) {
    free(gif->images);
    free(gif->globalColorTable);
    free(gif->gceArr);
}
