#include <stdio.h>
#include "label.h"

extern FILE *outfile;

static int lbl_count = 0;

int new_label(void){
    return lbl_count++;
}

void emit_label(int lbl){
    fprintf(outfile, "L%d:\n", lbl);
}