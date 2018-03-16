/* tableGen.c
 * 2018/3/12 : Finished, multirow still unsupported.
 */

#include "tableGen.h"
#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h>

#define TRUE 1
#define FALSE 0

/* Defines */
typedef struct {
    int OPTION_MEM_COPY;
} tableOpt;

/* Local Variable */
static tableOpt TABLE_OPT;

/* Local Functions */
// Member Functions
static int tblConfig(struct tblGenerator *pTblGen, TBL_OPTION option, int arg);
static int columnDefine(tblGenerator *pTblGen, char **columns, int numOfColumn);
static int newRow(tblGenerator *pTblGen);
static int rowElemAdd(tblGenerator *pTblGen, char *elem);
static int rowAppendWhole(tblGenerator *pTblGen, ...);
static int tblPrint(tblGenerator *pTblGen);
// Print Functions
static int rowSeperator(int *colWidth, int numOfCol, char seperator, char angle);
static int rowPrint(char **rowCOntent, char seperator, int *colWidth, int numOfCol, int multirow);
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
    memset(&TABLE_OPT, 0, sizeof(tableOpt));
    pTblGen->config = tblConfig;
    pTblGen->columnDefine = columnDefine;
    pTblGen->newRow = newRow;
    pTblGen->rowElemAdd = rowElemAdd;
    pTblGen->rowAppendWhole = rowAppendWhole;
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
static int tblConfig(struct tblGenerator *pTblGen, TBL_OPTION option, int arg) {
    if (pTblGen == NULL)
        return -1;

    switch (option) {
        case TBL_CONFIG_WIDTH:
            pTblGen->tableWidth = arg;
            break;
        case TBL_CONFIG_MEM_COPY:
            TABLE_OPT.OPTION_MEM_COPY = arg;
            break;
        default:
            return -2;
    }

    return 0;
}

static int columnDefine(tblGenerator *pTblGen, char **columns, int numOfColumn) {
    int *colWidth, idx = 0;
    char **columns_;
    if (pTblGen == NULL || columns == NULL || numOfColumn < 1)
        return -1;

    pTblGen->numOfColumns = numOfColumn;
    pTblGen->colWidth = colWidth = (int *)calloc(numOfColumn, sizeof(int));
    pTblGen->columns = (char **)malloc(numOfColumn * sizeof(char *));
    columns_ = pTblGen->columns;

    while (idx < numOfColumn) {
        columns_[idx] = strdup(columns[idx]);
        idx++;
    }

    idx = 0;
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
    char **element, *newElem;
    int numOfColumn, idx = 0, *colWidth;

    if (pTblGen == NULL || elem == NULL)
        return -1;

    if (TABLE_OPT.OPTION_MEM_COPY)
        newElem = strdup(elem);
    else
        newElem = elem;

    numOfColumn = pTblGen->numOfColumns;
    element = pTblGen->current->elements;

    while (idx < numOfColumn) {
        if (*(element+idx) == NULL) {
            *(element+idx) = newElem;
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

static int rowAppendWhole(tblGenerator *pTblGen, ...) {
    int numOfElems;
    va_list elemList;

    if (pTblGen == NULL) {
        return -1;
    }
    numOfElems = pTblGen->numOfColumns;

    va_start(elemList, pTblGen);
    while (numOfElems) {
        rowElemAdd(pTblGen, va_arg(elemList, char *));
        numOfElems--;
    }
    return 0;
}

static int columnWidthCompute(int *colWidth, int totalLen, int numOfCol) {
    int length, idx, temp, remain;
    float ratio, reassignLen;

    for (length = idx = 0; idx < numOfCol; idx++) {
        length += colWidth[idx];
    }

    // No need to do compute.
    if (length <= totalLen) {
        return 0;
    }

    // Do compute.
    ratio = (float)totalLen / (float)length;
    for (length = idx = 0; idx < numOfCol; idx++) {
        temp = colWidth[idx] * ratio;
        length += temp;
        colWidth[idx] = temp;
    }

    return 1;
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
static int rowPrint(char **rowContent, char seperator, int *colWidth, int numOfCol, int multirow) {
    char *breakBackup;
    int idx = 0, diff, numOfLine, *tracker, offset, flag;

    // Prepare for multiline per row.
    if (multirow) {
        breakBackup = (char *)malloc(numOfCol * sizeof(char));
        tracker = (int *)malloc(2 *numOfCol * sizeof(int));
        for (idx = 0; idx < numOfCol; idx++) {
            breakBackup[idx] = rowContent[idx][0];
            tracker[idx] = 0;
            tracker[numOfCol+idx] = strlen(rowContent[idx]);
        }
    }

    numOfLine = 1;
    do {
        flag = 1;
        for (idx = 0; idx < numOfCol; idx++) {
            printf("%c", seperator);

            // Multiline Prepare.
            if (multirow && tracker[numOfCol+idx] - tracker[idx] > colWidth[idx]) {
                if (flag) {
                    numOfLine++;
                    flag = 0;
                }
                rowContent[idx][tracker[idx]] = breakBackup[idx];
                breakBackup[idx] = rowContent[idx][colWidth[idx]];
                rowContent[idx][colWidth[idx]] = '\0';
            } else if (multirow){
                if (tracker[idx] != tracker[idx+numOfCol])
                    rowContent[idx][tracker[idx]] = breakBackup[idx];
            }
            if (multirow) {
                offset = tracker[idx];
            } else {
                offset = 0;
            }

            if (multirow && offset != tracker[idx+numOfCol])
                printf(rowContent[idx]+offset);
            else if (!multirow && offset != strlen(rowContent[idx]))
                printf(rowContent[idx]+offset);

            diff = colWidth[idx] - strlen(rowContent[idx]+offset);
            while (diff > 0) {
                printf(" ");
                diff--;
            }

            if (multirow && tracker[idx] < tracker[idx+numOfCol]) {
                if (tracker[idx+numOfCol] - tracker[idx] > colWidth[idx]) {
                    tracker[idx] += colWidth[idx];
                } else {
                    tracker[idx] = tracker[idx+numOfCol];
                }
            }
        }
        printf("%c\n", seperator);
    } while (--numOfLine);
    return 0;
}

// Multirow feature pending.
static int tblPrint(tblGenerator *pTblGen) {
    row *pRow;
    int *colWidth, numOfColumns, tblWidth, numOfRow, idx,
        multirow;

    if (pTblGen == NULL)
        return -1;

    // Column width compute.
    colWidth = pTblGen->colWidth;
    numOfColumns = pTblGen->numOfColumns;
    tblWidth = pTblGen->tableWidth;
    // Do column width compute when column is too long.
    multirow = columnWidthCompute(colWidth, tblWidth, numOfColumns);

    // Header print.
    rowSeperator(colWidth, numOfColumns, ' ', ' ');
    rowPrint(pTblGen->columns, ' ', colWidth, numOfColumns, multirow);
    rowSeperator(colWidth, numOfColumns, '=', '=');
    fflush(stdout);

    // Content print.
    pRow = pTblGen->elements;
    numOfRow = pTblGen->numOfRows;
    for (idx = 0; idx < numOfRow; idx++) {
        rowPrint(pRow->elements, ' ', colWidth, numOfColumns, multirow);
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
