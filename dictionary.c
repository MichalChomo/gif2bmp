#include "dictionary.h"

void dictInit(tDict *dict, uint16_t size) {
    tDictRow row;
    uint16_t i = 0;

    row.size = 1;
    dict->rows = malloc(size * sizeof(tDictRow *));
    memset(dict->rows, 0, size * sizeof(tDictRow *));
    for (; i < size / 2; ++i) {
        row.colorIndexes = malloc(sizeof(uint8_t));
        *(row.colorIndexes) = i;
        (dict->rows)[i] = malloc(sizeof(tDictRow));
        *((dict->rows)[i]) = row;
    }
    for (; i < size; ++i) {
        (dict->rows)[i] = NULL;
    }

    dict->size = size;
    dict->insertIndex = (size / 2) + 2;
}

void dictInsert(tDict *dict, tDictRow *row) {
    if (dict == NULL || row == NULL) {
        return;
    }
    if (dict->insertIndex < dict->size) {
        freeRowAndColorIndexes((dict->rows)[dict->insertIndex]);
        (dict->rows)[dict->insertIndex] = row;
        ++(dict->insertIndex);
    } else {
        // Shouldn't happen, but at least prevent memory leak.
        freeRowAndColorIndexes(row);
        printf("debug insertIndex overflow\n");
    }
}

void dictSearch(tDict *dict, uint16_t index, tDictRow **row) {
    if (dict == NULL || index > dict->size) {
        return;
    }
    *row = (dict->rows)[index];
}

void dictDestroy(tDict *dict) {
    tDictRow *row;

    if (dict == NULL) {
        return;
    }
    for (uint16_t i = 0; i < dict->size; ++i) {
        freeRowAndColorIndexes((dict->rows)[i]); 
    }
    free(dict->rows);
}

void dictResize(tDict *dict, uint16_t size) {
    tDictRow **tmpRows;

    // When the dictionary is shrinking, free the old content.
    if (size <= dict->size) {
        for (uint16_t i = size / 2; i < dict->size; ++i) {
            freeRowAndColorIndexes((dict->rows)[i]); 
        }
    }
    tmpRows = realloc(dict->rows, size * sizeof(tDictRow *));
    if (tmpRows == NULL) {
        fprintf(stderr, "realloc failed while resizing dictionary.\n");
    } else {
        dict->rows = tmpRows;
        for (uint16_t i = size / 2; i < size; ++i) {
            (dict->rows)[i] = NULL; 
        }
        dict->size = size;
    }
}

void dictReinit(tDict *dict, uint16_t index) {
    dict->insertIndex = index;
}

void freeRowAndColorIndexes(tDictRow *row) {
    if (row != NULL) {
        if (row->colorIndexes != NULL) {
            free(row->colorIndexes);
        }
        free(row);
        row = NULL;
    }
}

tDictRow *createRowToAdd(tDictRow *prevRow, uint8_t k) {
    tDictRow *rowToAdd = NULL;

    if (prevRow == NULL) {
        fprintf(stderr, "Previous row is null.\n");
        return NULL;
    }

    rowToAdd = malloc(sizeof(tDictRow));
    if (rowToAdd == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return NULL;
    }
    rowToAdd->size = prevRow->size + 1;
    rowToAdd->colorIndexes = malloc(rowToAdd->size * sizeof(uint8_t));
    if (rowToAdd->colorIndexes == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return NULL;
    }
    memset(rowToAdd->colorIndexes, 0, rowToAdd->size * sizeof(uint8_t));
    memcpy(rowToAdd->colorIndexes, prevRow->colorIndexes,
            prevRow->size * sizeof(uint8_t));
    rowToAdd->colorIndexes += prevRow->size;
    memcpy(rowToAdd->colorIndexes, &k, sizeof(uint8_t));
    rowToAdd->colorIndexes -= prevRow->size;

    return rowToAdd;
}

void copyRow(tDictRow *dest, tDictRow *src) {
    if (dest->colorIndexes != NULL) {
        free(dest->colorIndexes);
    }
    dest->size = src->size;
    dest->colorIndexes = malloc(dest->size * sizeof(uint8_t));
    if (dest->colorIndexes == NULL) {
        fprintf(stderr, "malloc failed.\n");
        return;
    }
    memcpy(dest->colorIndexes, src->colorIndexes,
            src->size * sizeof(uint8_t));
}
