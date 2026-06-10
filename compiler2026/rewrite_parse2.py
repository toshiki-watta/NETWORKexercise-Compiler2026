import re

with open('work/parse.c', 'r', encoding='utf-8') as f:
    text = f.read()

# write文のロード
old_write = r'''int idx = sym_lookup\(tok\.charvalue\);
\s*if \(idx == -1\) error\("writeで未定義の変数です。"\);
\s*fprintf\(outfile, "load r0, %d\\n", idx\); /\* 変数の値を r0 に \*/'''
new_write = r'''SymEntry* e = sym_lookup(tok.charvalue);
                if (!e) error("writeで未定義の変数です。");
                emit_load("r0", e);'''
text = re.sub(old_write, new_write, text)

# emit_compare_and_consume のロード
old_cmp = r'''int rhs = sym_lookup\(tok\.charvalue\);
\s*if \(rhs == -1\) error\("右辺の変数が未定義です。"\);
\s*fprintf\(outfile, "load r1, %d\\n", rhs\);
\s*fprintf\(outfile, "cmpr r0, r1\\n"\);'''
new_cmp = r'''SymEntry* rhs = sym_lookup(tok.charvalue);
        if (!rhs) error("右辺の変数が未定義です。");
        emit_load("r1", rhs);
        fprintf(outfile, "cmpr r0, r1\n");'''
text = re.sub(old_cmp, new_cmp, text)

# outblock 内での procedure ループ
outblock_addition = r'''
    while (tok.attr == RWORD && tok.value == PROCEDURE) {
        getsym(); 
        if (tok.attr != IDENTIFIER) error("procedure宣言で識別子が必要です。");
        char proc_name[MAXIDLEN+1];
        strcpy(proc_name, tok.charvalue);
        int proc_lbl = new_label();
        SymEntry* proc_sym = sym_install_proc(proc_name, proc_lbl);
        getsym();
        
        int arg_count = 0;
        if (tok.attr == SYMBOL && tok.value == LPAREN) {
            getsym();
            while (tok.attr == IDENTIFIER) {
                // [BR+2], [BR+3]... PCと動的リンクの分+2
                sym_install_arg(tok.charvalue, 2 + arg_count);
                arg_count++;
                getsym();
                if (tok.attr == SYMBOL && tok.value == COMMA) {
                    getsym();
                } else {
                    break;
                }
            }
            if (tok.attr == SYMBOL && tok.value == RPAREN) {
                getsym();
            } else {
                error(") が必要です。");
            }
        }
        proc_sym->params = arg_count;

        if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
            getsym();
        } else {
            error("procedure 宣言のあとに ';' が必要です。");
        }

        inblock(proc_sym);

        if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
            getsym();
        } else {
            error("procedure の末尾に ';' が必要です。");
        }
    }
'''
text = text.replace('// ここ？procedure', outblock_addition)

# inblock の追加
inblock_body = r'''
void inblock(SymEntry* proc_sym) {
    int local_count = 0;

    // 局所変数の宣言をパース
    while (tok.attr == RWORD && tok.value == VAR) {
        getsym(); 
        if (tok.attr != IDENTIFIER) error("var宣言で識別子が必要です。");
        for (;;) {
            local_count++;
            sym_install_local_var(tok.charvalue, -local_count);
            getsym();
            if (tok.attr == SYMBOL && tok.value == COMMA) {
                getsym(); continue;
            } else if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
                getsym(); break;
            } else {
                error("var宣言の文法エラー。',' または ';' が必要です。");
            }
        }
    }

    emit_label(proc_sym->label);
    fprintf(outfile, "push br\n");
    fprintf(outfile, "loadr br, sp\n");
    fprintf(outfile, "subi sp, %d\n", local_count);

    statement();

    fprintf(outfile, "loadr sp, br\n");
    fprintf(outfile, "pop br\n");
    fprintf(outfile, "return\n");

    sym_clear_local();
}
'''
# outblock の終わりに差し込むか、// procedure宣言の処理もここに追加... の部分を置換するか
text = re.sub(r'// // procedure宣言の処理もここに追加する必要がある.*?(?=void statement)', inblock_body, text, flags=re.DOTALL)

# statement 内の indentifier
old_assign = r'''// ここを変更しないといけない
\s*// 手続き処理のParamlistを追加する
\s*\} else if \(tok.attr == IDENTIFIER\) \{
\s*char name\[MAXIDLEN\+1\];
\s*strcpy\(name, tok\.charvalue\); /\* トークンの識別子名をコピー \*/
\s*int lhs = sym_lookup\(name\);
\s*if \(lhs == -1\) error\("未宣言の変数に代入しようとしています。"\);
\s*getsym\(\); /\* 識別子を消費 \*/

\s*if \(!\(tok\.attr == SYMBOL && tok\.value == BECOMES\)\) error\("':=' が必要です。"\);
\s*getsym\(\); /\* ':=' を消費 \*/

\s*/\* 右辺を r0 に評価　\*/
\s*eval_to_r0\(\);

\s*/\* 計算結果を左辺のアドレスへ格納 \*/
\s*fprintf\(outfile, "store r0, %d\\n", lhs\);'''

new_assign = r'''} else if (tok.attr == IDENTIFIER) {
        char name[MAXIDLEN+1];
        strcpy(name, tok.charvalue);
        SymEntry* target = sym_lookup(name);
        if (!target) error("未定義の識別子です。");
        getsym();

        if (target->type == SYM_PROC) {
            int actual_args = 0;
            if (tok.attr == SYMBOL && tok.value == LPAREN) {
                getsym();
                while (tok.attr != SYMBOL || tok.value != RPAREN) {
                    eval_to_r0(); 
                    fprintf(outfile, "push r0\n"); 
                    actual_args++;
                    if (tok.attr == SYMBOL && tok.value == COMMA) {
                        getsym();
                    } else if (tok.attr == SYMBOL && tok.value == RPAREN) {
                        break;
                    } else {
                        error("引数リストが不正です");
                    }
                }
                getsym(); // ')' を消費
            }
            fprintf(outfile, "call L%d\n", target->label);
            if (actual_args > 0) {
                fprintf(outfile, "addi sp, %d\n", actual_args);
            }
        } else {
            if (!(tok.attr == SYMBOL && tok.value == BECOMES)) error("':=' が必要です。");
            getsym();
            eval_to_r0();
            emit_store("r0", target);
        }'''

text = re.sub(old_assign, new_assign, text)

# // // procedure宣言の処理もここに追加... の部分が re.sub で漏れていたら困るので、文字列置換もダメ押しでやっておく
if '// // procedure宣言の処理もここに追加する必要がある' in text:
    pass # ok

with open('work/parse.c', 'w', encoding='utf-8') as f:
    f.write(text)
