#ifndef SYMTAB_H
#define SYMTAB_H

int sym_lookup(const char *name);
int sym_install(const char *name);
void sym_dump(void); /* シンボルテーブルの中身をデバッグ出力 */

#endif