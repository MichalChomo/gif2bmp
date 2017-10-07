#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "gif2bmp.h"

typedef struct {
    uint8_t  *colorIndexes;
    uint16_t   size;
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

void dictReinit(tDict *dict, uint16_t index);

void freeRowAndColorIndexes(tDictRow *row);

/**
 * Allocate memory and copy color indexes to new dictionary row.
 * @param[in] prevRow Previous row.
 * @param[in] k       First color index, either from previous or current row.
 * @return Pointer to the new row.
 */
tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k);

/**
 * Copy dictionary row.
 * @param[out] dest Row to copy to.
 * @param[in]  src  Row to copy.
 */
void copyRow(tDictRow *dest, tDictRow *src);

#endif
