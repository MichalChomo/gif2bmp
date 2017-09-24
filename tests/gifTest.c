#include "CuTest.h"
#include "../gif.h"
#include "../gif2bmp.h"

void testGetCodeZeroes(CuTest *tc) {
    uint8_t *buffer = NULL;
    uint8_t *bufferStart = NULL;
    uint8_t codeSize = 4;
    uint16_t code = 0;
    uint16_t codeSum = 0;

    buffer = malloc(9 * sizeof(uint8_t));
    memset(buffer, 0, 9 * sizeof(uint8_t));
    bufferStart = buffer;

    getCode(&buffer, 255);

    for (; codeSize <= LZW_MAX_CODE_SIZE; ++codeSize) {
        code = getCode(&buffer, codeSize);
        codeSum += code;
    }

    free(bufferStart);

    CuAssertIntEquals(tc, 0, codeSum);
}

void testGetCodeOnes(CuTest *tc) {
    uint8_t *buffer = NULL;
    uint8_t *bufferStart = NULL;
    uint8_t codeSize = 4;
    uint16_t code = 0;
    uint16_t codeSum = 0;
    uint16_t expectedSum = 0;

    buffer = malloc(9 * sizeof(uint8_t));
    memset(buffer, 0xff, 9 * sizeof(uint8_t));
    bufferStart = buffer;

    getCode(&buffer, 255);

    for (; codeSize <= LZW_MAX_CODE_SIZE; ++codeSize) {
        code = getCode(&buffer, codeSize);
        codeSum += code;
        expectedSum += (1 << codeSize) - 1;
    }

    free(bufferStart);

    CuAssertIntEquals(tc, expectedSum, codeSum);
}

CuSuite* GifGetSuite(void) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testGetCodeZeroes);
    SUITE_ADD_TEST(suite, testGetCodeOnes);

    return suite;
}

