#ifndef GIF2BMP_H
#define GIF2BMP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

#define BUFFER_PART 256

#define LZW_MAX_CODE_SIZE 12

/**
 * Structure with file sizes.
 */
typedef struct {
    int64_t bmpSize;
    int64_t gifSize;
}tGIF2BMP;

/**
 * Convert image in GIF format to BMP format.
 * @param[in]  gif2bmp   Sizes of both files.
 * @param[in]  inputFile Pointer to input GIF image file.
 * @param[out] outpuFile Pointer to output BMP image file.
 * @return 0 OK, 1 error
 */
int gif2bmp(tGIF2BMP *gif2bmp, FILE *inputFile, FILE *outputFile);

/**
 * Load file contents to buffer.
 * @param[out] buffer Pointer to pointer to buffer.
 * @param[in]  fp     Pointer to file.
 * @param[in]  size   Size of the file.
 */
void loadFileToBuffer(uint8_t **buffer, FILE *fp, int64_t size);

/**
 * Write information about sizes to log file.
 * @param[in] fileName  Name of the log file.
 * @param[in] codedSize Struct with file sizes.
 * @return 0 success, 1 fail.
 */
int createLogfile(char *fileName, tGIF2BMP *g2b);

/**
 * Get the size of the file in bytes.
 * @param[in] fileName Name of the file.
 * @return Size of the file.
 */
int64_t getFileSize(const char *fileName);

#endif
