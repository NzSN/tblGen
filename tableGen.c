/* tableGen.c
 * 2018/3/12 : Finished, multirow still unsupported.
 */

#include "tableGen.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>

/* Local Functions */
// Member Functions
static int tblConfig(struct tblGenerator *pTblGen, TBL_OPTION option, void *arg);
static int columnDefine(tblGenerator *pTblGen, char **columns, int numOfColumn);
static int newRow(tblGenerator *pTblGen);
static int rowElemAdd(tblGenerator *pTblGen, char *elem);
static int tblPrint(tblGenerator *pTblGen);
// Print Functions
static int rowSeperator(int *colWidth, int numOfCol, char seperator, char angle);
static int rowPrint(char **rowCOntent, char seperator, int *colWidth, int numOfCol);
static int tblPrint(tblGenerator *pTblGen);
// Row Processing
static row * prevRow(row *pRow);
static row *nextRow(row *pRow);
static char * rowGetElem(row *pRow, int column);
static int rowSetElem(row *pRow, char *element, int column);
static int rowAppend(row *pRow, row *pNew);
static int rowRelease(row *pRow, int numOfColumn);
// Misc
static int columnWidthCompute(int *colWidth, int totalLen, int numOfCol);

int tblGenConstruct(tblGenerator *pTblGen) {
    if (pTblGen == NULL)
        return -1;
    memset(pTblGen, 0, sizeof(tblGenerator));
    pTblGen->config = tblConfig;
    pTblGen->columnDefine = columnDefine;
    pTblGen->newRow = newRow;
    pTblGen->rowElemAdd = rowElemAdd;
    pTblGen->print = tblPrint;
    return 0;
}

int tblGenDestruct(tblGenerator *pTblGen) {
    row *pRow;
    int numOfColumns, numOfRows, idx;

    if (pTblGen == NULL)
        return -1;
    pRow = pTblGen->elements;
    numOfColumns = pTblGen->numOfColumns;
    numOfRows = pTblGen->numOfRows;

    for (idx = 0; idx < numOfRows; idx++) {
        rowRelease(pRow, numOfColumns);
        pRow = nextRow(pRow);
    }
    return 0;
}

// Member Functions
static int tblConfig(struct tblGenerator *pTblGen, TBL_OPTION option, void *arg) {
    int argInt;

    switch (option) {
        case TBL_CONFIG_WIDTH:
            argInt = *((int *)arg);
            pTblGen->tableWidth = argInt;
            break;
        default:
            return -1;
    }

    return 0;
}

static int columnDefine(tblGenerator *pTblGen, char **columns, int numOfColumn) {
    int *colWidth, idx = 0;
    if (pTblGen == NULL || columns == NULL || numOfColumn < 1)
        return -1;
    pTblGen->columns = columns;
    pTblGen->numOfColumns = numOfColumn;
    pTblGen->colWidth = colWidth = (int *)calloc(numOfColumn, sizeof(int));

    while (idx < numOfColumn) {
        colWidth[idx] = strlen(columns[idx]);
        idx++;
    }

    return 0;
}

static int newRow(tblGenerator *pTblGen) {
    row *pTmp;

    if (pTblGen == NULL)
        return -1;
    pTmp = (row *)calloc(1, sizeof(row));
    pTmp->elements = (char **)calloc(pTblGen->numOfColumns, sizeof(char *));
    if (pTblGen->current == NULL) {
        pTblGen->current = pTmp;
        pTblGen->elements = pTmp;
    } else {
        rowAppend(pTblGen->current, pTmp);
        pTblGen->current = pTmp;
    }
    pTblGen->numOfRows++;
    return 0;
}

static int rowElemAdd(tblGenerator *pTblGen, char *elem) {
    char **element;
    int numOfColumn, idx = 0, *colWidth;

    if (pTblGen == NULL || elem == NULL)
        return -1;
    numOfColumn = pTblGen->numOfColumns;
    element = pTblGen->current->elements;

    while (idx < numOfColumn) {
        if (*(element+idx) == NULL) {
            *(element+idx) = elem;
            break;
        }
        idx++;
    }

    // Column width update
    colWidth = pTblGen->colWidth;
    if (*(colWidth+idx) < strlen(elem))
        *(colWidth+idx) = strlen(elem);

    return 0;
}

static int columnWidthCompute(int *colWidth, int totalLen, int numOfCol) {
    int length, idx, temp, remain;
    float ratio;

    for (length = idx = 0; idx < numOfCol; idx++) {
        length += colWidth[idx];
    }

    // No need to do compute.
    if (length <= totalLen) {
        return 0;
    }

    // Do compute.
    ratio = totalLen / length;
    for (length = idx = 0; idx < numOfCol; idx++) {
        temp = colWidth[idx] * ratio;
        length += temp;
        colWidth[idx] = temp;
    }
    remain = totalLen - length;
    while (remain > 0) {
        // pending
    }

    return 0;
}

static int rowSeperator(int *colWidth, int numOfCol, char seperator, char angle) {
    int width, idx;

    idx = 0;
    while (idx < numOfCol) {
        printf("%c", angle);
        width = colWidth[idx];
        while (width) {
            printf("%c", seperator);
            width--;
        }
        idx++;
    }
    printf("%c\n", angle);
    return 0;
}

// Multirow feature still not be supported.
static int rowPrint(char **rowContent, char seperator, int *colWidth, int numOfCol) {
    int idx = 0, diff;

    while (idx < numOfCol) {
        printf("%c", seperator);
        printf(rowContent[idx]);
        diff = colWidth[idx] - strlen(rowContent[idx]);
        while (diff > 0) {
            printf(" ");
            diff--;
        }
        idx++;
    }

    printf("%c\n", seperator);
    return 0;
}

// Multirow feature pending.
static int tblPrint(tblGenerator *pTblGen) {
    row *pRow;
    int *colWidth, numOfColumns, tblWidth, numOfRow, idx;

    if (pTblGen == NULL)
        return -1;

    // Column width compute.
    colWidth = pTblGen->colWidth;
    numOfColumns = pTblGen->numOfColumns;
    tblWidth = pTblGen->tableWidth;
    // Do column width compute when column is too long.
    columnWidthCompute(colWidth, tblWidth, numOfColumns);

    // Header print.
    rowSeperator(colWidth, numOfColumns, ' ', ' ');
    rowPrint(pTblGen->columns, ' ', colWidth, numOfColumns);
    rowSeperator(colWidth, numOfColumns, '=', '=');
    fflush(stdout);

    // Content print.
    pRow = pTblGen->elements;
    numOfRow = pTblGen->numOfRows;
    for (idx = 0; idx < numOfRow; idx++) {
        rowPrint(pRow->elements, ' ', colWidth, numOfColumns);
        rowSeperator(colWidth, numOfColumns, ' ', ' ');
        pRow = nextRow(pRow);
    }
    return 0;
}

// Row & Elem Processing Functions
static row * prevRow(row *pRow) {
    return pRow->prev;
}

static row * nextRow(row *pRow) {
    return pRow->next;
}

static char * rowGetElem(row *pRow, int column) {
    return *(pRow->elements + (column - 1));
}

static int rowSetElem(row *pRow, char *elem, int column) {
    *(pRow->elements + (column - 1)) = elem;
    return 1;
}

static int rowAppend(row *pRow, row *pNew) {
    while (pRow->next != NULL) {
        pRow = nextRow(pRow);
    }
    pRow->next = pNew;
    pNew->prev = pRow;
    return 0;
}

static int rowRelease(row *pRow, int numOfColumn) {
    int idx;
    char **elements;

    while (pRow) {
        elements = pRow->elements;
        for (idx = 0; idx < numOfColumn; idx++) {
            free(*(elements+idx));
        }
        free(elements);
        pRow = nextRow(pRow);
    }
    return 0;
}

/* tblGen.c */
