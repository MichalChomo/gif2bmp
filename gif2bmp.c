#include "gif2bmp.h"

int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile) {
    unsigned char header[GIF_HEADER_SIZE] = {0};

    fread(&header, 1, GIF_HEADER_SIZE, inputFile);
    if (0 != memcmp(header, "GIF89a", GIF_HEADER_SIZE)) {
        fprintf(stderr, "Wrong input file format.\n");
        return 1;
    }

    return 0;
}

int createLogfile(char *fileName, tGIF2BMP *g2b) {
    FILE *lfp;
    char login[] = "xchomo01";

    lfp = fopen(fileName, "w");
    if (lfp == NULL) {
        fprintf(stderr, "Cannot open logfile %s\n.", fileName);
        return 1;
    }
    fprintf(lfp, "login = %s\nuncodedSize = %ld\ncodedSize = %ld\n", login,
            g2b->gifSize, g2b->bmpSize);
    fclose(lfp);

    return 0;
}
