#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED, // 予約語, 記号
	TK_IDENT, // 識別子
	TK_NUM, // 整数トークン
	TK_EOF, // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
	TokenKind kind; // トークンの型
	Token *next;    // 次の入力のトークン
	int val;        // kindがTK_NUMの場合、その数値
	char *str;      // トークン文字列
	int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_EQ,  // ==
	ND_NE,  // !=
	ND_LT,  // <
	ND_LE,  // <=
	ND_ASSIGN, // =
	ND_LVAR, // ローカル変数
	ND_NUM, // 整数
	ND_RETURN, // return
	ND_IF, // if
	ND_WHILE, // while
	ND_FOR, // for
	ND_EXPR_STMT, // expression statement
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind; // ノードの型
	Node *lhs;     // 左辺 (left-hand side)
	Node *rhs;     // 右辺 (right-hand side)
	int val;       // kind が ND_NUM の場合のみ使う
	int offset;    // kind が ND_LVAR の場合のみ使う

	// "if", "while", "for" statement
	Node *cond;
	Node *then;
	Node *els;
	Node *init;
	Node *inc;
};

// ローカル変数の型
typedef struct LVar LVar;
struct LVar {
	LVar *next; // 次の変数かNULL
	char *name; // 変数の名前
	int len;    // 名前の長さ
	int offset; // RBPからのオフセット
};

// 入力プログラム
extern char *user_input;
// 現在着目しているトークン
extern Token *token;
// ローカル変数(ローカル変数リストのトップ)
extern LVar *locals;

Token *tokenize(char *input);
bool equal(char *op);
void program();
void gen_lval(Node *node);
void gen(Node *node);
// パースの結果としての複数のノードを保存しておくための配列の宣言
struct Node *code[100];

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
