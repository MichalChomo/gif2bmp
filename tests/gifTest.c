#include "CuTest.h"
#include "../gif.h"

void testGetCodeZeroes(CuTest *tc) {
    uint8_t *buffer = NULL;
    uint8_t *bufferStart = NULL;
    uint8_t codeSize = 4;
    uint16_t code = 0;

    buffer = malloc(30 * sizeof(uint8_t));
    memset(buffer, 0, 30 * sizeof(uint8_t));
    bufferStart = buffer;

    for (; codeSize < 13; ++codeSize) {
        code = getCode(&buffer, codeSize);
        CuAssertIntEquals(tc, 0, code);
    }

    free(bufferStart);
}

CuSuite* GifGetSuite(void) {
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, testGetCodeZeroes);

    return suite;
}

