/* tableGen.h */

#ifndef _TABLE_GEN_
#define _TABLE_GEN_

typedef enum {
    TBL_CONFIG_WIDTH
} TBL_OPTION;

typedef struct row {
    char **elements;
    struct row *prev;
    struct row *next;
} row;

typedef struct tblGenerator {
    int numOfColumns;
    int numOfRows;
    int tableWidth;
    int *colWidth;
    char **columns;
    row *elements;
    row *current;
    int (*config)(struct tblGenerator *, TBL_OPTION option, void *arg);
    int (*columnDefine)(struct tblGenerator *, char **columns, int numOfColumn);
    // Before add elems to a row you should create a new row
    int (*newRow)(struct tblGenerator *);
    int (*rowElemAdd)(struct tblGenerator *, char *elem);
    int (*print)(struct tblGenerator *);
} tblGenerator;

int tblGenConstruct(tblGenerator *pTblGen);
int tblGenDestruct(tblGenerator *pTblGen);

#endif /* _TABLE_GEN_ */
