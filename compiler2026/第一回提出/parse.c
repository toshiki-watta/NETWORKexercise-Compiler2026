#include <stdio.h>
#include <stdlib.h>
#include <getsym.h>
extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;

void error(char *s);
void statement(void);

void compiler(void){
	init_getsym();

	getsym();

	if (tok.attr == RWORD && tok.value == PROGRAM){

		getsym();

		if (tok.attr == IDENTIFIER){

			getsym();

			if (tok.attr == SYMBOL && tok.value == SEMICOLON){

				getsym();

				statement();

				// // チェックよう
				// printf("Current token: attr=%d, value=%d\n", tok.attr, tok.value);

				if (tok.attr == SYMBOL && tok.value == PERIOD){
					fprintf(stderr, "Parsing Done. No errors found.\n");
				} else error("At the end, a period is required.");
			}else error("After program name, a semicolon is needed.");
		}else error("Program identifier is needed.");
	}else error("At the first, program declaration is required.");
}

void error(char *s){
	fprintf(stderr, "%s\n", s);
	exit(1);
}

//
// Parser
//

void statement(void){
    // 再帰の深さを管理する静的変数
	// 最後にhaltを出すために変なことをしている
    static int depth = 0;
    int is_outer = (depth == 0);  // 一番外側からかどうか
    depth++;

    if (tok.attr == RWORD && tok.value == BEGIN) {
        getsym();
        // begin ... end の複数文
        while (1) {
            // 再帰的に処理
            statement();

            if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
                getsym();
                continue;   // 次の文へ
            } else {
                break;      // ';' でなければブロック終端方向へ
            }
        }
        if (tok.attr == RWORD && tok.value == END) {
            getsym();       // END の次（通常は '.'）へ進める
        } else {
            error("endが必要です。");
        }

    } else if (tok.attr == NUMBER) {
        // 単独式
		// 左辺
        fprintf(outfile, "loadi r0 %d\n", tok.value);
        getsym();

        // 演算子判定
        if ((tok.attr == SYMBOL && (tok.value == TIMES || tok.value == PLUS || tok.value == MINUS)) ||
            (tok.attr == RWORD && tok.value == DIV)) {

            int operando = tok.value;
            int operattr = tok.attr;
            getsym();

            if (tok.attr == NUMBER) {
                // 右辺
                if (operattr == SYMBOL) {
                    if (operando == TIMES)
                        fprintf(outfile, "muli r0 %d\n", tok.value);
                    else if (operando == PLUS)
                        fprintf(outfile, "addi r0 %d\n", tok.value);
                    else if (operando == MINUS)
                        fprintf(outfile, "subi r0 %d\n", tok.value);
                } else if (operattr == RWORD && operando == DIV) {
                    fprintf(outfile, "divi r0 %d\n", tok.value);
                }
                getsym();

                // 結果を出力
                fprintf(outfile, "writed r0\n");
                // 改行も出力
                fprintf(outfile, "loadi r1 '\\n'\n");
                fprintf(outfile, "writec r1\n");
            } else {
                error("右辺が必要です。");
            }
        } else {
            error("演算子が必要です。");
        }
    }

    // ここまででこの statement 呼び出しの処理は終了
    depth--;

    // 一番外側（compiler() から直接呼ばれたとき）だけ、最後に halt を1回出す
    if (is_outer) {
        fprintf(outfile, "halt\n");
    }
}
