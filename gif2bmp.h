//  Name: Michal Chomo
// Login: xchomo01
//  Date: 5.5.2017
//
//  Header file with constants, data structures and function declarations.

#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

#define LOGIN "xchomo01"

#define BUFFER_PART 256

#define LZW_MAX_CODE_SIZE 12

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

void loadFileToBuffer(uint8_t **buffer, FILE *fp, int64_t size);

/**
 * @param fileName Name of the log file.
 * @param uncodedSize GIF file size.
 * @param codedSize BMP file size.
 * @return 0 success, 1 fail.
 */
int createLogfile(char *fileName, tGIF2BMP *g2b);

/**
 * Get the size of the file in bytes.
 * @para fileName Name of the file.
 * @return Size of the file.
 */
int64_t getFileSize(const char *fileName);

#endif
