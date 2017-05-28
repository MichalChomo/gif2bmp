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
        ((dict->rows)[i]) = malloc(sizeof(tDictRow));
        *((dict->rows)[i]) = row;
    }
    for (; i < size; ++i) {
        ((dict->rows)[i]) = NULL;
    }

    dict->size = size;
    dict->insertIndex = (size / 2) + 2;
}

void dictInsert(tDict *dict, tDictRow *row) {
    if (dict == NULL || row == NULL) {
        return;
    }
    if (dict->insertIndex < dict->size) {
        ((dict->rows)[dict->insertIndex]) = row;
        ++(dict->insertIndex);
    }
}

void dictSearch(tDict *dict, uint16_t index, tDictRow **row) {
    if (dict == NULL || index > dict->size) {
        return;
    }
    *row = ((dict->rows)[index]);
}

void dictDestroy(tDict *dict) {
    if (dict == NULL) {
        return;
    }
    tDictRow *row;

    for (uint16_t i = 0; i < dict->size; ++i) {
        row = (dict->rows)[i];
        if (row != NULL) {
            if (row->colorIndexes != NULL) {
                free(row->colorIndexes);
            }
            free(row);
        }
    }
    free(dict->rows);
}

void dictResize(tDict *dict, uint16_t size) {
    dict->rows = realloc(dict->rows, size * sizeof(tDictRow *));
    for (uint16_t i = size / 2; i < size; ++i) {
        ((dict->rows)[i]) = NULL;
    }
    dict->size = size;
}
