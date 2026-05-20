#include <string.h>
#include <getsym.h>
#include "symtab.h"

#define MAXSYMS 32

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