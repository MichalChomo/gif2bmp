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

tDictNode *createNode() {
    tDictNode *tmpNode = NULL;

    tmpNode = malloc(sizeof(tDictNode));
    if (tmpNode != NULL) {
        tmpNode->isLeaf = false;
        for (int i = 0; i < DICT_LEAF_COUNT; ++i) {
            tmpNode->children[i] = NULL;
        }
    }

    return tmpNode;
}

void insert(tDictNode *root, unsigned char *key, int8_t keyLength) {
    tDictNode *tmpNode = root;

    while (keyLength > 0) {
        if (tmpNode->children[*key] == NULL) {
            tmpNode->children[*key] = createNode();
        }
        tmpNode = tmpNode->children[*key];
        ++key;
        --keyLength;
    }
    tmpNode->isLeaf = true;
}

void destroy(tDictNode *root) {
    if (root == NULL) {
        return;
    }

    for (int i = 0; i < DICT_LEAF_COUNT; ++i) {
        destroy(root->children[i]);
    }
    free(root);
}

int parseGif(tGif *gif, unsigned char *buffer) {
    tGifLsd lsd;
    tGifGce gce;
    tGifGce *gceArrPtr = NULL;
    tGifImg img;
    tGifImg *imgPtr = NULL;

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
        (gif->info).globalTableSize = getTableSize(lsd.packedField);
        getGlobalTable(&(gif->globalColorTable), (gif->info).globalTableSize,
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

    printf("\n\n");
    for (int i = 0; i < 6; ++i) {
        printf("%02x ", *buffer);
        ++buffer;
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
        getImgInfo(&(img.info), img.desc.packedField);
    }

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

uint16_t getTableSize(uint8_t pf) {
    // Get last 3 bits from packed field.
    uint8_t n = pf & 7;
    uint16_t res = 2;
    // Return size, it equals 2^(n + 1).
    return res << n;
}

void getGlobalTable(tColor **gct, uint16_t size, unsigned char *buffer) {
    tColor c;
    tColor *gctStart = NULL;

    *gct = malloc(size * sizeof(tColor));
    gctStart = *gct;
    for (; size > 0; --size) {
        c.red = *buffer;
        ++buffer;
        c.green = *buffer;
        ++buffer;
        c.blue = *buffer;
        ++buffer;
        memcpy(*gct, &c, sizeof(tColor));
        ++(*gct);
    }

    *gct = gctStart;
}

void printGct(tColor *gct, uint16_t size) {
    tColor c;

    printf("Global color table:\n");
    for (; size > 0; --size) {
        c = *gct;
        printf("%02x %02x %02x||", c.red, c.green, c.blue);
        ++gct;
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
    info->localTableSize = getTableSize(pf);
}

void freeGif(tGif *gif) {
    free(gif->globalColorTable);
    free(gif->gceArr);
}
