#ifndef GEN_EMIT_H
#define GEN_EMIT_H
#include "symtab.h"
#include <stdio.h>
extern FILE* outfile;

static inline void emit_load(const char *reg, SymEntry *e) {
    if (e->type == SYM_GLOBAL_VAR || e->type == SYM_TEMP) {
        fprintf(outfile, "load %s, %d\n", reg, e->addr);
    } else {
        // Local / Arg: BRからのオフセット参照
        // SRマシンアセンブリでは load r0, [br + offset] みたいな構文があるのか？
        // 疑似コードでは mov r0, [BR + オフセット] とされている。
        // compkit/asm.pl によると MODADDR をサポートしており、
        // 0(r1) のように書けばベース相対のはず。
        // 例えば load r0, offset(br)
        fprintf(outfile, "load %s, %d(br)\n", reg, e->addr);
    }
}

static inline void emit_store(const char *reg, SymEntry *e) {
    if (e->type == SYM_GLOBAL_VAR || e->type == SYM_TEMP) {
        fprintf(outfile, "store %s, %d\n", reg, e->addr);
    } else {
        fprintf(outfile, "store %s, %d(br)\n", reg, e->addr);
    }
}
#endif
