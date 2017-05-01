#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>

typedef struct {
    int64_t bmpSize;
    int64_t gifSize;
}tGIF2BMP;

/**
 * Convert image in .gif format to .bmp format.
 * @param gif2bmp Sizes of both files.
 * @param inputFile
 * @param outpuFile
 * @return 0 OK, 1 error
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

#endif
