#include "tablegen.h"

int main() {
    int width = 30;
    tblGenerator tblGen;

    char *columns[] = {"CLASS", "NAME", "AGE"};

    tblGenConstruct(&tblGen);
    tblGen.columnDefine(&tblGen, columns, 3);
    tblGen.config(&tblGen, TBL_CONFIG_WIDTH, (void *)&width);
    tblGen.newRow(&tblGen);

    tblGen.rowElemAdd(&tblGen, "1");
    tblGen.rowElemAdd(&tblGen, "Ayden");
    tblGen.rowElemAdd(&tblGen, "16");

    tblGen.print(&tblGen);
    return 0;
}
