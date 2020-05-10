#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//
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

bool equal(char *op);
Token *skip(char *op);
bool consume(char *op);
void expect(char *op);
Token *consume_ident();
int expect_number();
bool at_eof();
Token *tokenize(char *input);

//
// parse.c
//
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
	ND_VAR, // 変数
	ND_FUNCALL, // Function call
	ND_FUNCDEF, // Function Definition
	ND_NUM, // 整数
	ND_RETURN, // return
	ND_IF, // if
	ND_WHILE, // while
	ND_FOR, // for
	ND_EXPR_STMT, // expression statement
	ND_BLOCK, // compound statement
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;
struct Node {
	NodeKind kind; // ノードの型
	Node *next;    // next node
	Node *lhs;     // 左辺 (left-hand side)
	Node *rhs;     // 右辺 (right-hand side)
	int val;       // kind が ND_NUM の場合のみ使う
	int offset;    // kind が ND_VAR の場合のみ使う

	// "if", "while", "for" statement
	Node *cond;
	Node *then;
	Node *els;
	Node *init;
	Node *inc;

	// compound statement
	Node *body;

	// Function call
	char *funcname;
	Node *args;
};

// ローカル変数の型
typedef struct Var Var;
struct Var {
	Var *next; // 次の変数かNULL
	char *name; // 変数の名前
	int len;    // 名前の長さ
	int offset; // RBPからのオフセット
};

typedef struct Function Function;
struct Function {
	Function *next;
	char *name;
	Var *params;

	Node *node;
	Var *locals;
	int stack_size;
};

Function *program();
extern Function *current_func;

//
// codegen.c
//
void codegen();

// container.c
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

//
// use with multiple files
//
// 入力プログラム
extern char *user_input;
// 現在着目しているトークン
extern Token *token;
// パースの結果としての複数のノードを保存しておくための配列の宣言
struct Function *code[100];
