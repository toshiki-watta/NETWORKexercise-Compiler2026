#include <string.h>
#include <stdio.h>
#include "symtab.h"

#define MAXSYMS 256

static SymEntry global_table[MAXSYMS];
static int global_count = 0;

static SymEntry local_table[MAXSYMS];
static int local_count = 0;

extern void error(char *s);

SymEntry* sym_lookup(const char *name) {
    // まずLocalTableから探す
    for (int i = 0; i < local_count; ++i) {
        if (strcmp(name, local_table[i].name) == 0) {
            return &local_table[i];
        }
    }
    // 見つからなければGlobalTableから探す
    for (int i = 0; i < global_count; ++i) {
        if (strcmp(name, global_table[i].name) == 0) {
            return &global_table[i];
        }
    }
    return NULL;
}

static SymEntry* sym_install_global(const char *name) {
    if (global_count >= MAXSYMS) error("Global symbol table overflow");
    SymEntry* entry = &global_table[global_count++];
    strcpy(entry->name, name);
    return entry;
}

static SymEntry* sym_install_local(const char *name) {
    if (local_count >= MAXSYMS) error("Local symbol table overflow");
    SymEntry* entry = &local_table[local_count++];
    strcpy(entry->name, name);
    return entry;
}

SymEntry* sym_install_global_var(const char *name) {
    SymEntry* e = sym_lookup(name);
    // 重複チェックは雑に (とりあえず既にGlobalにあればそれを返すようにする?)
    // 厳密には Global内だけで重複チェックすべきだが...
    for (int i = 0; i < global_count; ++i) {
        if (strcmp(name, global_table[i].name) == 0) return &global_table[i];
    }
    e = sym_install_global(name);
    e->type = SYM_GLOBAL_VAR;
    e->addr = global_count - 1; // 簡易的に index を addr にする(もしこれでよければ)
    // ただし、これだとアドレスが0, 1, ... になる。アセンブラ的にはこれでいい？
    // load r0, [addr] のときのアドレス。
    // parse.c 側では今まで "load r0, %d" で直接 index を指定していたので、同じ挙動にする。
    return e;
}

SymEntry* sym_install_proc(const char *name, int label) {
    SymEntry* e = sym_install_global(name);
    e->type = SYM_PROC;
    e->label = label;
    e->params = 0;
    return e;
}

SymEntry* sym_install_local_var(const char *name, int offset) {
    SymEntry* e = sym_install_local(name);
    e->type = SYM_LOCAL_VAR;
    e->addr = offset;
    return e;
}

SymEntry* sym_install_arg(const char *name, int offset) {
    SymEntry* e = sym_install_local(name);
    e->type = SYM_ARG;
    e->addr = offset;
    return e;
}

SymEntry* sym_install_temp(const char *name, int offset) {
    SymEntry* e = sym_install_local(name);
    e->type = SYM_TEMP;
    e->addr = offset;
    return e;
}

void sym_clear_local(void) {
    local_count = 0;
}

void sym_dump(void) {
    fprintf(stderr, "=== sym_dump ===\n");
    fprintf(stderr, "Global:\n");
    for (int i = 0; i < global_count; ++i) {
        fprintf(stderr, "  [%2d] name=\"%s\" type=%d addr/offset=%d label=L%d\n", i, global_table[i].name, global_table[i].type, global_table[i].addr, global_table[i].label);
    }
    fprintf(stderr, "Local:\n");
    for (int i = 0; i < local_count; ++i) {
        fprintf(stderr, "  [%2d] name=\"%s\" type=%d addr/offset=%d\n", i, local_table[i].name, local_table[i].type, local_table[i].addr);
    }
    fprintf(stderr, "================\n");
}
