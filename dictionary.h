#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "gif2bmp.h"

typedef struct {
    uint8_t  *colorIndexes;
    uint8_t   size;
}tDictRow;

typedef struct {
    tDictRow **rows;
    uint16_t   size;
    uint16_t   insertIndex;
}tDict;

void dictInit(tDict *dict, uint16_t size);

void dictInsert(tDict *dict, tDictRow *row);

void dictSearch(tDict *dict, uint16_t index, tDictRow **row);

void dictDestroy(tDict *dict);

void dictResize(tDict *dict, uint16_t size);

#endif
