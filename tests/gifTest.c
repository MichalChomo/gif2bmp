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

    getCode(NULL, 0xff);

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

    getCode(NULL, 0xff);

    for (; codeSize <= LZW_MAX_CODE_SIZE; ++codeSize) {
        code = getCode(&buffer, codeSize);
        codeSum += code;
        expectedSum += (1 << codeSize) - 1;
    }

    free(bufferStart);

    CuAssertIntEquals(tc, expectedSum, codeSum);
}

void testGetCodeSequence(CuTest *tc) {
    uint8_t *buffer = NULL;
    uint8_t *bufferStart = NULL;
    uint8_t codeSize = 4;
    uint16_t code = 0;
    uint16_t expectedCodes[6] = {1, 2, 3, 20, 18, 29};
    bool fail = false;

    buffer = malloc(4 * sizeof(uint8_t));
    memset(buffer, 0xff, 4 * sizeof(uint8_t));
    bufferStart = buffer;

    *buffer = 0x21;
    ++buffer;
    *buffer = 0x43;
    ++buffer;
    *buffer = 0x65;
    ++buffer;
    *buffer = 0x87;

    buffer = bufferStart;

    getCode(NULL, 0xff);

    for (uint8_t i = 0; i < 6; ++i) {
        if (i == 3) {
            ++codeSize;
        }
        code = getCode(&buffer, codeSize);
        if (code != expectedCodes[i]) {
            fail = true;
        }
    }

    free(bufferStart);

    CuAssertTrue(tc, !fail);
}

CuSuite* GifGetSuite(void) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testGetCodeZeroes);
    SUITE_ADD_TEST(suite, testGetCodeOnes);
    SUITE_ADD_TEST(suite, testGetCodeSequence);

    return suite;
}

