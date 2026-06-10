#ifndef SYMTAB_H
#define SYMTAB_H

#include <getsym.h>

typedef enum {
    SYM_GLOBAL_VAR,
    SYM_LOCAL_VAR,
    SYM_ARG,
    SYM_PROC,
    SYM_TEMP
} SymType;

typedef struct {
    char name[MAXIDLEN+1];
    SymType type;
    int addr;   // offset for local/arg/temp
    int label;  // label for proc
    int params; // number of args for proc
} SymEntry;

// 検索（Local -> Global の順）
SymEntry* sym_lookup(const char *name);

// 登録
SymEntry* sym_install_global_var(const char *name);
SymEntry* sym_install_local_var(const char *name, int offset);
SymEntry* sym_install_arg(const char *name, int offset);
SymEntry* sym_install_proc(const char *name, int label);
SymEntry* sym_install_temp(const char *name, int offset);

// 旧版互換(parse.cで書き換えるまでの仮) または全て新しいものに書き換えるため削除
// int sym_install(const char *name);

void sym_clear_local(void);
void sym_dump(void);

#endif
