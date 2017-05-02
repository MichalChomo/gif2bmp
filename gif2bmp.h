#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>

#define GIF_HEADER_SIZE 6

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

/**
 * @param fileName Name of the log file.
 * @param uncodedSize GIF file size.
 * @param codedSize BMP file size.
 * @return 0 success, 1 fail.
 */
int createLogfile(char *fileName, tGIF2BMP *g2b);

#endif
