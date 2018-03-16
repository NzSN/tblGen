#include "tablegen.h"

int main() {
    tblGenerator tblGen;

    char *columns[] = {"CLASS", "NAME", "AGE"};

    tblGenConstruct(&tblGen);
    tblGen.columnDefine(&tblGen, columns, 3);
    // Maximum width of the table.
    tblGen.config(&tblGen, TBL_CONFIG_WIDTH, 17);
    tblGen.config(&tblGen, TBL_CONFIG_MEM_COPY, TBL_OPTION_OPEN);

    tblGen.newRow(&tblGen);
    tblGen.rowAppendWhole(&tblGen, "1", "Ayden", "16");

    tblGen.newRow(&tblGen);
    tblGen.rowAppendWhole(&tblGen, "1", "Alice", "16");

    tblGen.print(&tblGen);
    return 0;
}
