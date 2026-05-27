#include <string.h>
#include <getsym.h>
#include "symtab.h"
#include <stdio.h> /* fprintf */

#define MAXSYMS 256

typedef struct {
    int addr;
    char v[MAXIDLEN+1];
} s_entry;

static s_entry s_table[MAXSYMS];
static int s_count = 0;

int sym_lookup(const char *name){
    int i;
    for (i = 0; i < s_count; ++i)
        if (strcmp(name, s_table[i].v) == 0) return i;
    return -1;
}

int sym_install(const char *name){
    int idx = sym_lookup(name);
    if (idx != -1) return idx;
    if (s_count >= MAXSYMS) {
        extern void error(char *s);
        error("symbol table overflow");
    }
    strcpy(s_table[s_count].v, name);
    s_table[s_count].addr = s_count;
    return s_count++;
}

void sym_dump(void){
    int i;
    fprintf(stderr, "=== sym_dump (count=%d) ===\n", s_count);
    for (i = 0; i < s_count; ++i){
        fprintf(stderr, "  [%2d] name=\"%s\" addr=%d\n", i, s_table[i].v, s_table[i].addr);
    }
    fprintf(stderr, "===========================\n");
}