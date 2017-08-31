//  Name: Michal Chomo
// Login: xchomo01
//  Date: 5.5.2017
//
//  Definitions of functions for GIF to BMP conversion.

#include "gif2bmp.h"
#include "dictionary.h"
#include "gif.h"
#include "bmp.h"

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
    fclose(outputFile);

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
