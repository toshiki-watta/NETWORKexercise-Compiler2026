#include <stdio.h>
#include <stdlib.h>


#include "symtab.h"
#include "label.h"
#include "gen_emit.h"

void inblock(SymEntry* proc_sym);

#include <string.h>

extern TOKEN tok;
extern FILE *infile;
extern FILE *outfile;

void error(char *s);
void statement(void);
void outblock(void);
static void parse_expression_bottomup(void);
// static void term(void);
// static void factor(void);
static SymEntry* new_temp(void);

static int fits_immed(int v);
static void emit_load_const_to_reg(int reg, int val);

void compiler(void){
	init_getsym();

	getsym();

	if (tok.attr == RWORD && tok.value == PROGRAM){

		getsym();

		if (tok.attr == IDENTIFIER){

			getsym();

			if (tok.attr == SYMBOL && tok.value == SEMICOLON){

				getsym();

				outblock();

                /* 変数登録の直後にシンボルテーブルをダンプ*/
                sym_dump();

				// // チェックよう
				// printf("Current token: attr=%d, value=%d\n", tok.attr, tok.value);

				// if (tok.attr == SYMBOL && tok.value == PERIOD){
				// 	fprintf(stderr, "Parsing Done. No errors found.\n");
				// } else error("At the end, a period is required.");

                statement();
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

/* 簡易一時確保: __tmp0, __tmp1 ... を symtab に登録してインデックスを返す */
static SymEntry* new_temp(void){
    static int tmpcnt = 0;
    char tmpname[MAXIDLEN+1];
    snprintf(tmpname, sizeof(tmpname), "__tmp%d", tmpcnt++);
    return sym_install_global_var(tmpname);
}

/* ラッパー*/
static void eval_to_r0(void){
    parse_expression_bottomup();
}

/* これ以下はトップダウンの時のコメントアウト　*/

// /* factor -> IDENTIFIER | NUMBER | '(' expression ')' */
// static void factor(void){
//     if (tok.attr == IDENTIFIER) {
//         int idx = sym_lookup(tok.charvalue);
//         if (idx == -1) error("未定義の変数です。");
//         fprintf(outfile, "load r0, %d\n", idx);
//         getsym();
//     } else if (tok.attr == NUMBER) {
//         /* 大きな即値は emit_load_const_to_reg() に任せる */
//         emit_load_const_to_reg(0, tok.value); /* r0 に定数を生成 */
//         getsym();
//     } else if (tok.attr == SYMBOL && tok.value == LPAREN) {
//         getsym(); /* '(' を消費 */
//         expression();
//         if (!(tok.attr == SYMBOL && tok.value == RPAREN)) error(") が必要です。");
//         getsym(); /* ')' を消費 */
//     } else {
//         error("factor に識別子・数・'(' が必要です。");
//     }
// }

// /* term -> factor { (* | div ) factor } */
// static void term(void){
//     factor(); /* left -> r0 */
//     for (;;) {
//         if ((tok.attr == SYMBOL && tok.value == TIMES) ||
//             (tok.attr == RWORD && tok.value == DIV)) {
//             int op = tok.value;
//             int opattr = tok.attr;
//             getsym(); /* 演算子消費 */

//             if (tok.attr == NUMBER) {
//                 if (fits_immed(tok.value)) {
//                     if (opattr == SYMBOL && op == TIMES) fprintf(outfile, "muli r0, %d\n", tok.value);
//                     else fprintf(outfile, "divi r0, %d\n", tok.value);
//                 } else {
//                     /* 大きい即値: r1 に作ってレジスタ演算 */
//                     emit_load_const_to_reg(1, tok.value);
//                     if (opattr == SYMBOL && op == TIMES) fprintf(outfile, "mulr r0, r1\n");
//                     else fprintf(outfile, "divr r0, r1\n");
//                 }
//                 getsym();
//             } else if (tok.attr == IDENTIFIER) {
//                 int rhs = sym_lookup(tok.charvalue);
//                 if (rhs == -1) error("右辺の変数が未定義です。");
//                 fprintf(outfile, "load r1, %d\n", rhs);
//                 if (opattr == SYMBOL && op == TIMES) fprintf(outfile, "mulr r0, r1\n");
//                 else fprintf(outfile, "divr r0, r1\n");
//                 getsym();
//             } else {
//                 /* 複雑な RHS: 左を一時保存して RHS を評価し結合する */
//                 SymEntry* tmp = new_temp();
//                 emit_store("r0", tmp); /* left -> mem */
//                 factor(); /* RHS -> r0 */
//                 emit_load("r1", tmp); /* r1 = left */
//                 if (opattr == SYMBOL && op == TIMES) fprintf(outfile, "mulr r1, r0\n");
//                 else fprintf(outfile, "divr r1, r0\n");
//                 emit_store("r1", tmp);
//                 emit_load("r0", tmp); /* 結果 -> r0 */
//             }
//             continue;
//         }
//         break;
//     }
// }

// /* expression -> term { (+ | -) term } */
// static void expression(void){
//     term(); /* left -> r0 */
//     while (tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS)) {
//         int op = tok.value;
//         SymEntry* tmp = new_temp();
//         emit_store("r0", tmp); /* save left */
//         getsym(); /* consume + or - */
//         term();  /* parse entire RHS into r0 */
//         emit_load("r1", tmp); /* r1 = left */
//         if (op == PLUS) fprintf(outfile, "addr r1, r0\n");
//         else fprintf(outfile, "subr r1, r0\n");
//         emit_store("r1", tmp);
//         emit_load("r0", tmp); /* result -> r0 */
//     }
// }


// /* Bottom-up で expression を評価するための前方宣言と実装 */
// static void parse_expression_bottomup(void){
//     /* ここでは expression -> term { (+|-) term } を bottom-up で処理する。
//        まず最初の term を評価して r0 に置き、次のトークンが + または - ならループで処理する。 */
//     while (tok.attr == SYMBOL && (tok.value == PLUS || tok.value == MINUS)) {
//         int op = tok.value;
//         SymEntry* tmp = new_temp();
//         emit_store("r0", tmp); /* left -> mem */
//         getsym(); /* + または - を消費 */
//         term(); /* 次の term -> r0 */
//         emit_load("r1", tmp); /* r1 = left */
//         if (op == PLUS) fprintf(outfile, "addr r1, r0\n");
//         else fprintf(outfile, "subr r1, r0\n");
//         emit_store("r1", tmp);
//         emit_load("r0", tmp); /* 結果 -> r0 */
//     }
// }



/* 右辺を r0/r1 に評価して比較命令を出力する。比較演算子を返す。 */
static int emit_compare_and_consume(void){
    int op = tok.value; /* 比較演算子を保持 */
    getsym(); /* 演算子を消費して RHS の先頭へ */

    if (tok.attr == NUMBER) {
        if (fits_immed(tok.value)) {
            fprintf(outfile, "cmpi r0, %d\n", tok.value);
            getsym();
        } else {
            emit_load_const_to_reg(1, tok.value); /* r1 に定数 */
            fprintf(outfile, "cmpr r0, r1\n");
            getsym();
        }
    } else if (tok.attr == IDENTIFIER) {
        SymEntry* rhs = sym_lookup(tok.charvalue);
        if (!rhs) error("右辺の変数が未定義です。");
        emit_load("r1", rhs);
        fprintf(outfile, "cmpr r0, r1\n");
        getsym();
    } else {
        /* 複雑な RHS：左を一時保存して expression() で評価 */
        SymEntry* tmp = new_temp();
        emit_store("r0", tmp); /* left -> mem */
        parse_expression_bottomup(); /* RHS -> r0 */
        emit_load("r1", tmp); /* r1 = left */
        fprintf(outfile, "cmpr r1, r0\n");      /* compare left,right */
    }
    return op;
}

// 変数宣言
void outblock(void){
    /* VAR 宣言を読み、識別子をカンマ区切りで登録し、セミコロンで終える。
       VAR が複数並ぶ場合は繰り返す。 */
    while (tok.attr == RWORD && tok.value == VAR) {
        getsym(); /* VAR の次のトークンへ */
        if (tok.attr != IDENTIFIER) error("var宣言で識別子が必要です。");
        for (;;) {
            sym_install_global_var(tok.charvalue); /* 登録（既存なら重複回避） */
            getsym();
            if (tok.attr == SYMBOL && tok.value == COMMA) {
                getsym(); /* 次の識別子へ */
                if (tok.attr != IDENTIFIER) error("var宣言で識別子が必要です。");
                continue;
            } else if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
                getsym(); /* セミコロンの次へ（次の宣言または文頭） */
                // statement(); /* VAR 宣言の後は文が続く */

                // ここ？procedure

                break;
            } else {
                error("var宣言の文法エラー。',' または ';' が必要です。");
            }
        }
    }
    /* outblock 終了時点で tok は次の文（通常 BEGIN など）の先頭を指す */
}

// // procedure宣言の処理もここに追加する必要がある
// void inblock(void){
//     /* procedure 宣言を読み、識別子を登録し、セミコロンで終える。
//        procedure が複数並ぶ場合は繰り返する。 */
//     while (tok.attr == RWORD && tok.value == PROCEDURE) {
//         getsym(); /* procedure の次のトークンへ */
//         if (tok.attr != IDENTIFIER) error("procedure宣言で識別子が必要です。");
//         sym_install(tok.charvalue); /* 登録（既存なら重複回避） */
//         getsym();
//         if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
//             getsym(); /* セミコロンの次へ（次の宣言または文頭） */
//             statement(); /* procedure 宣言の後は文が続く */
//             break;
//         } else {
//             error("procedure宣言の文法エラー。';' が必要です。");
//         }
//     }

void statement(void){
    static int depth = 0;
    int is_outer = (depth == 0); /* 外側からの呼び出しかどうか */
    depth++;
    // わざわざhaltを考えるためにこれあるのなぁ

    /* ブロック: begin - end */
    if (tok.attr == RWORD && tok.value == BEGIN) {
        getsym(); /* 'begin' を消費して中身へ */
        while (1) {
            statement(); /* 再帰的に中の文を処理 */
            if (tok.attr == SYMBOL && tok.value == SEMICOLON) {
                getsym(); /* ';' を消費して次の文へ */
                continue;
            } else break; /* ';' が無ければブロック終端へ向かう */
        }
        if (tok.attr == RWORD && tok.value == END) {
            getsym(); /* 'end' を消費してブロックを閉じる */
        } else error("endが必要です。");

    /* write 文: write expr {, expr} */
    } else if (tok.attr == RWORD && tok.value == WRITE) {
        getsym(); /* 'write' 読む */
        for (;;) {
            /* 引数が識別子ならそのアドレスを load、数値なら loadi */
            if (tok.attr == IDENTIFIER) {
                SymEntry* e = sym_lookup(tok.charvalue);
                if (!e) error("writeで未定義の変数です。");
                emit_load("r0", e);
                getsym();
            } else if (tok.attr == NUMBER) {
                fprintf(outfile, "loadi r0, %d\n", tok.value); /* 即値を r0 に */
                getsym();
            } else {
                error("writeの引数が不正です。");
            }
            fprintf(outfile, "writed r0\n"); /* 整数出力 */

            /* 引数区切りのカンマがあればスペース出力して次の引数へ */
            if (tok.attr == SYMBOL && tok.value == COMMA) {
                /* 環境により文字扱いが違うので ASCII 値 32 を推奨 */
                fprintf(outfile, "loadi r1, 32\n"); /* ' ' の ASCII */
                fprintf(outfile, "writec r1\n");
                getsym(); /* ',' を消費して次の引数へ */
                continue;
            } else break;
        }
        /* 最後に改行を出力 */
        fprintf(outfile, "loadi r1, 10\n");
        fprintf(outfile, "writec r1\n");

    /* 代入: ident := expression */

    // ここを変更しないといけない
    // 手続き処理のParamlistを追加する
    } else if (tok.attr == IDENTIFIER) {
        char name[MAXIDLEN+1];
        strcpy(name, tok.charvalue); /* トークンの識別子名をコピー */
        SymEntry* lhs = sym_lookup(name);
        if (!lhs) error("未宣言の変数に代入しようとしています。");
        getsym(); /* 識別子を消費 */

        if (!(tok.attr == SYMBOL && tok.value == BECOMES)) error("':=' が必要です。");
        getsym(); /* ':=' を消費 */

        /* 右辺を r0 に評価　*/
        eval_to_r0();

        /* 計算結果を左辺のアドレスへ格納 */
        emit_store("r0", lhs);

    /* if 文: if condition then statement [ else statement ] */
    } else if (tok.attr == RWORD && tok.value == IF) {
        getsym(); /* 'if' を消費 */
        /* 条件の左辺を r0 に評価 */
        eval_to_r0();

        /* 比較演算子のチェックと右辺評価 */
        if (!(tok.attr == SYMBOL &&
              (tok.value == EQL || tok.value == NOTEQL || tok.value == LESSTHAN ||
               tok.value == LESSEQL || tok.value == GRTRTHAN || tok.value == GRTREQL)))
    error("条件の比較演算子が必要です。");
        int op = emit_compare_and_consume();


        printf("%d\n", tok.attr);
        printf("%d\n", tok.value);
        if (!(tok.attr == RWORD && tok.value == THEN)) error("then が必要です。");
        getsym(); /* 'then' を消費 */

        int else_lbl = new_label();
        int end_lbl = new_label();

        /* 結果が偽のとき else に飛ばす逆条件ジャンプ */
        switch(op){
        case EQL:    fprintf(outfile, "jnz L%d\n", else_lbl); break;
        case NOTEQL: fprintf(outfile, "jz L%d\n", else_lbl); break;
        case LESSTHAN: fprintf(outfile, "jge L%d\n", else_lbl); break;
        case LESSEQL:  fprintf(outfile, "jgt L%d\n", else_lbl); break;
        case GRTRTHAN: fprintf(outfile, "jle L%d\n", else_lbl); break;
        case GRTREQL:  fprintf(outfile, "jlt L%d\n", else_lbl); break;
        default: error("不明な比較演算子です。");
        }

        /* then 部分を処理 */
        statement();

        if (tok.attr == RWORD && tok.value == ELSE) {
            /* else がある場合は then のあとに end ラベルへジャンプを置き、else ラベルを出力 */
            fprintf(outfile, "jmp L%d\n", end_lbl);
            emit_label(else_lbl);
            getsym(); /* 'else' を消費 */
            statement();
            emit_label(end_lbl);
        } else {
            /* else がなければ else ラベルをここに置くだけ */
            emit_label(else_lbl);
        }

    /* while 文: while condition do statement */
    } else if (tok.attr == RWORD && tok.value == WHILE) {
        getsym(); /* 'while' を消費 */
        int start_lbl = new_label();
        int end_lbl = new_label();
        emit_label(start_lbl); /* ループ先頭ラベル */

        /* 条件の左辺を評価して比較を発行 */
        eval_to_r0();
        if (!(tok.attr == SYMBOL || tok.attr == RWORD)) error("条件の比較演算子が必要です。");
        int op = emit_compare_and_consume();

        /* 偽ならループ脱出 */
        switch(op){
        case EQL:    fprintf(outfile, "jnz L%d\n", end_lbl); break;
        case NOTEQL: fprintf(outfile, "jz L%d\n", end_lbl); break;
        case LESSTHAN: fprintf(outfile, "jge L%d\n", end_lbl); break;
        case LESSEQL:  fprintf(outfile, "jgt L%d\n", end_lbl); break;
        case GRTRTHAN: fprintf(outfile, "jle L%d\n", end_lbl); break;
        case GRTREQL:  fprintf(outfile, "jlt L%d\n", end_lbl); break;
        default: error("不明な比較演算子です。");
        }

        if (!(tok.attr == RWORD && tok.value == DO)) error("do が必要です。");
        getsym(); /* 'do' を消費 */

        /* ループ本体 */
        statement();

        /* ループの後で先頭へ戻るジャンプを出力し、終了ラベルを置く */
        fprintf(outfile, "jmp L%d\n", start_lbl);
        emit_label(end_lbl);

    } else {
        error("不正な文です。");
    }

    depth--;
    /* 外側からの最初の呼び出しの終わりで halt を出力（必要なら compiler() 側に移動） */
    if (is_outer) {
        fprintf(outfile, "halt\n");
    }
}

static int fits_immed(int v){
    return (v >= -(1<<15) && v <= (1<<15)-1); /* -32768..32767 */
}

static void emit_load_const_to_reg(int reg, int val){
    const char *r = (reg == 0) ? "r0" : "r1";
    if (fits_immed(val)){
        fprintf(outfile, "loadi %s, %d\n", r, val);
        return;
    }
    int absval = val < 0 ? -val : val;
    int a = 0, b = 0;
    for (int i = 2; i <= 32767 && i <= absval; ++i) {
        if (absval % i == 0) {
            int j = absval / i;
            if (j <= 32767) { a = i; b = j; break; }
        }
    }
    if (a) {
        if (val < 0) fprintf(outfile, "loadi %s, -%d\n", r, a);
        else fprintf(outfile, "loadi %s, %d\n", r, a);
        fprintf(outfile, "muli %s, %d\n", r, b);
        return;
    }
    error("定数が大きすぎて処理できません。");
}

/* 演算子種別に対応するインデックス */
enum {
    OP_PLUS,    /* + */
    OP_MINUS,   /* - */
    OP_TIMES,   /* * */
    OP_DIV,     /* div */
    OP_UNARY,   /* 単項- ！*/
    OP_LPAREN,  /* ( */
    OP_RPAREN,  /* ) */
    OP_IDENT,   /* i */
    OP_END,     /* $ 式終端 */
    OP_NUMOPS
};

/* rank_f, rank_g （指導書の表に合わせた値）*/
static const int rank_f[OP_NUMOPS] = {
    2,  /* + */
    2,  /* - */
    4,  /* * */
    4,  /* div */
    6,  /* 単項 - ( ! ) */
    0,  /* ( */
    11, /* ) */
    11, /* i */
    0   /* $ */
};

static const int rank_g[OP_NUMOPS] = {
    1,  /* + */
    1,  /* - */
    3,  /* * */
    3,  /* div */
    15, /* 単項 - ( ! ) */
    10, /* ( */
    0,  /* ) */
    10, /* i */
    0   /* $ */
};

/* スタック風定義 */
#define OP_STACK_SIZE 256
#define VAL_STACK_SIZE 256

static int op_stack[OP_STACK_SIZE];
static int op_top = -1;

static SymEntry* val_stack[VAL_STACK_SIZE]; /* 被演算子はsymtab のインデックスを積む */
static int val_top = -1;

/* op_stack 操作 */
static void push_op(int o){
    if (op_top >= OP_STACK_SIZE-1) error("演算子スタックオーバーフロー");
    op_stack[++op_top] = o;
}
static int pop_op(void){
    if (op_top < 0) error("演算子スタックアンダーフロー");
    return op_stack[op_top--];
}
static int top_op(void){
    if (op_top < 0) return OP_END;
    return op_stack[op_top];
}

/* val_stack 操作 */
static void push_val(SymEntry* v){
    if (val_top >= VAL_STACK_SIZE-1) error("被演算子スタックオーバーフロー");
    val_stack[++val_top] = v;
}
static SymEntry* pop_val(void){
    if (val_top < 0) error("被演算子スタックアンダーフロー");
    return val_stack[val_top--];
}

/* トークンが式の終端（演算子ではない）なら OP_END を返す */
static int token_to_inop(int expect_operand){
    if (tok.attr == SYMBOL){
        switch(tok.value){
            case PLUS:  return OP_PLUS;
            case MINUS: return expect_operand ? OP_UNARY : OP_MINUS;
            case TIMES: return OP_TIMES;
            case LPAREN: return OP_LPAREN;
            case RPAREN: return OP_RPAREN;
            default:    return OP_END; /* ; , then do などは式終り扱い */
        }
    } else if (tok.attr == RWORD && tok.value == DIV){
        return OP_DIV;
    } else {
        return OP_END;
    }
}

static const char *op_names[OP_NUMOPS] = {
    "+", "-", "*", "div", "unary-", "(", ")", "i", "$"
};

static const char *op_name(int op){
    if (op >= 0 && op < OP_NUMOPS) return op_names[op];
    return "unknown";
}

/* 数字は常に一時メモリアドレスを積む */
static void parse_expression_bottomup(void){
    /* 初期化 */
    op_top = val_top = -1;
    push_op(OP_END); /* 番兵 */
    int expect_operand = 1; /* 式先頭/'(' の直後はオペランドをまつ */

    for (;;) {
        /* 数値の処理：読んで一時的にスタックへ */
        if (tok.attr == IDENTIFIER) {
            SymEntry* v = sym_lookup(tok.charvalue);
            if (!v) error("未定義の変数です。");
            SymEntry* tmp = new_temp();
            emit_load("r0", v);
            emit_store("r0", tmp);
            push_val(tmp);
            getsym();
            expect_operand = 0;
            continue;
        }
        if (tok.attr == NUMBER) {
            SymEntry* tmp = new_temp();
            emit_load_const_to_reg(0, tok.value); /* r0 に値 */
            emit_store("r0", tmp);
            push_val(tmp);
            getsym();
            expect_operand = 0;
            continue;
        }

        /* 入力側の演算子に変換（式終端なら OP_END） */
        int in_op = token_to_inop(expect_operand);
        int top = top_op();

        /* 優先度比較 */
        int f = rank_f[top];
        int g = rank_g[in_op];

        if (f < g) {
            /* シフト：入力演算子をスタックに積んでトークンを進める */
            push_op(in_op);
            /* '(' や 単項は次にオペランドをまつ */
            if (in_op == OP_LPAREN || in_op == OP_UNARY) expect_operand = 1;
            else expect_operand = 1;
            /* 演算子トークンを消費（OP_END のときは消費しない） */
            if (in_op != OP_END) getsym();
            continue;
        } else if (f > g) {
            /* 還元：スタックから演算子を取り出し、数字を取り出してプッシュ */
            int op = pop_op();
            if (op == OP_UNARY) {
                /* 単項マイナス: 1オペランド */
                if (val_top < 0) error("単項演算子の被演算子が不足しています。");
                SymEntry* a = pop_val();
                emit_load("r0", a);
                /* r0 = -r0 を作る（安全のため r1 に -1 を作って乗算） */
                emit_load_const_to_reg(1, -1);
                fprintf(outfile, "mulr r0, r1\n");
                SymEntry* tmp = new_temp();
                emit_store("r0", tmp);
                push_val(tmp);
            } else {
                /* 二項演算 */
                if (val_top < 1) error("二項演算子の被演算子が不足しています。");
                SymEntry* right = pop_val();
                SymEntry* left  = pop_val();
                emit_load("r0", left);
                emit_load("r1", right);
                switch(op){
                    case OP_PLUS:  fprintf(outfile, "addr r0, r1\n"); break;
                    case OP_MINUS: fprintf(outfile, "subr r0, r1\n"); break;
                    case OP_TIMES: fprintf(outfile, "mulr r0, r1\n"); break;
                    case OP_DIV:   fprintf(outfile, "divr r0, r1\n"); break;
                    default:
                        /* 想定外の演算子がポップされたデバッグ出力 */
                        // なんでこうなんねん
                        fprintf(stderr, "reduce: unexpected op=%s (%d)\n", op_name(op), op);
                        error("不明な演算子で還元しました。");
                }
                SymEntry* tmp = new_temp();
                emit_store("r0", tmp);
                push_val(tmp);
            }
            /* 還元時はトークンを進めず、同じ入力トークンで再比較する */
            expect_operand = 0;
            continue;
        } else {
            /* f == g の場合：括弧の消去や終了 */
            if (top == OP_LPAREN && in_op == OP_RPAREN) {
                pop_op();   /* '(' を取り出して消去 */
                getsym();   /* ')' を消費 */
                expect_operand = 0;
                continue;
            } else if (top == OP_END && in_op == OP_END) {
                /* 式の終了 */
                break;
            } else {
                fprintf(stderr, "reduce: unexpected op=%s (%d)\n", op_name(top), top);
                error("優先度の等価ケースが未定義です（構文エラー）。");
            }
        }
    }

    /* 終了時：val_stack のトップを r0 に */
    if (val_top < 0) error("空の式です。");
    SymEntry* final_addr = pop_val();
    emit_load("r0", final_addr);
}
