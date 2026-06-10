import re

with open('work/parse.c', 'r', encoding='utf-8') as f:
    text = f.read()

# inblock / paramlist を用意するためのヘッダを gen_emit と共に追加
text = text.replace('#include "label.h"\n', '#include "label.h"\n#include "gen_emit.h"\n\nvoid inblock(SymEntry* proc_sym);\n')

# val_stack 等の型を SymEntry* に変更
text = text.replace('static int val_stack[VAL_STACK_SIZE]', 'static SymEntry* val_stack[VAL_STACK_SIZE]')
text = text.replace('static void push_val(int v){', 'static void push_val(SymEntry* v){')
text = text.replace('static int pop_val(void){', 'static SymEntry* pop_val(void){')
text = re.sub(r'int final_addr = pop_val\(\);\s*fprintf\(outfile, "load r0, %d\\n", final_addr\);',
              r'SymEntry* final_addr = pop_val();\n    emit_load("r0", final_addr);', text)

# new_temp
text = re.sub(r'static int new_temp\(void\)\{\n\s*static int tmpcnt = 0;\n\s*char tmpname\[MAXIDLEN\+1\];\n\s*int idx;\n\s*snprintf\(tmpname, sizeof\(tmpname\), "__tmp%d", tmpcnt\+\+\);\n\s*idx = sym_install\(tmpname\);\n\s*return idx;\n\}',
r'''static SymEntry* new_temp(void){
    static int tmpcnt = 0;
    char tmpname[MAXIDLEN+1];
    snprintf(tmpname, sizeof(tmpname), "__tmp%d", tmpcnt++);
    return sym_install_global_var(tmpname);
}''', text)

# new_temp の戻り値を保持する変数を int から SymEntry* に
text = re.sub(r'int tmp = new_temp\(\);', r'SymEntry* tmp = new_temp();', text)

text = re.sub(r'fprintf\(outfile, "store r0, %d\\n", tmp\);', r'emit_store("r0", tmp);', text)
text = re.sub(r'fprintf\(outfile, "load r1, %d\\n", tmp\);', r'emit_load("r1", tmp);', text)
text = re.sub(r'fprintf\(outfile, "load r0, %d\\n", tmp\);', r'emit_load("r0", tmp);', text)
text = re.sub(r'fprintf\(outfile, "store r1, %d\\n", tmp\);', r'emit_store("r1", tmp);', text)

# bottom-up 内の IDENTIFIER の処理
old_ident_in_expr = r'''int v = sym_lookup\(tok\.charvalue\);
\s*if \(v == -1\) error\("未定義の変数です。"\);
\s*int tmp = new_temp\(\);
\s*fprintf\(outfile, "load r0, %d\\n", v\);'''
new_ident_in_expr = r'''SymEntry* v = sym_lookup(tok.charvalue);
            if (!v) error("未定義の変数です。");
            SymEntry* tmp = new_temp();
            emit_load("r0", v);'''
text = re.sub(old_ident_in_expr, new_ident_in_expr, text)

# bottom-up 内の pop_val の受け取り
text = re.sub(r'int a = pop_val\(\);\n\s*fprintf\(outfile, "load r0, %d\\n", a\);',
              r'SymEntry* a = pop_val();\n                emit_load("r0", a);', text)
text = re.sub(r'int right = pop_val\(\);\n\s*int left  = pop_val\(\);\n\s*fprintf\(outfile, "load r0, %d\\n", left\);\n\s*fprintf\(outfile, "load r1, %d\\n", right\);',
              r'SymEntry* right = pop_val();\n                SymEntry* left  = pop_val();\n                emit_load("r0", left);\n                emit_load("r1", right);', text)

# outblock 内の sym_install を sym_install_global_var へ
text = text.replace('sym_install(tok.charvalue); /* 登録（既存なら重複回避） */', 'sym_install_global_var(tok.charvalue); /* 登録（既存なら重複回避） */')

with open('work/parse.c', 'w', encoding='utf-8') as f:
    f.write(text)
