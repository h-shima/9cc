#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Type Type;

//
// tokenize.c
//

// Token
typedef enum {
	TK_RESERVED, // Keywords or punctuators(,.)
	TK_IDENT,    // Identifiers
	TK_STR,      // String literals
	TK_NUM,      // Numeric literals
	TK_EOF,      // End-of-file markers
} TokenKind;

// Token type
typedef struct Token Token;
struct Token {
	TokenKind kind; // Token kind
	Token *next;    // Next token
	long val;       // If kind is TK_NUM, its value
	char *loc;      // Token location
	int len;        // Token length

	char *contents; // String literal contents including terminating '\0'
	char cont_len;  // string literal length
};

// http://wisdom.sakura.ne.jp/programming/c/c62.html
// `...` で可変個の引数を扱うことができる
void error(char *fmt, ...);
void error_tok(Token *tok, char *fmt, ...);
bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);
bool consume(Token **rest, Token *tok, char *str);
Token *tokenize_file(char *filename);

//
// parse.c
//

// Variable
typedef struct Var Var;
struct Var {
	Var *next;
	char *name;    // Variable name
	Type *ty;      // Type
	bool is_local; // local or global

	// Local variable
	int offset;

	// Global variable
	char *init_data;
};

// AST node
typedef enum {
	ND_ADD,       // +
	ND_SUB,       // -
	ND_MUL,       // *
	ND_DIV,       // /
	ND_EQ,        // ==
	ND_NE,        // !=
	ND_LT,        // <
	ND_LE,        // <=
	ND_ASSIGN,    // =
	ND_ADDR,      // unary &
	ND_DEREF,     // unary *
	ND_RETURN,    // "return"
	ND_IF,        // "if"
	ND_FOR,       // "for" or "while"
	ND_BLOCK,     // { ... }
	ND_FUNCALL,   // Function call
	ND_EXPR_STMT, // Expression statement
	ND_STMT_EXPR, // Statement expression
	ND_VAR,       // Variable
	ND_NUM,       // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
	NodeKind kind; // Node kind
	Node *next;    // Next node
	Type *ty;      // Type, e.g. int or pointer to int
	Token *tok;    // Representative token

	Node *lhs;     // Left-hand side
	Node *rhs;     // Right-hand side

	// "if" or "for" statement
	Node *cond;
	Node *then;
	Node *els;
	Node *init;
	Node *inc;

	// Block or statement expression
	Node *body;

	// Function call
	char *funcname;
	Node *args;

	Var *var;     // Used if kind == ND_VAR
	long val;     // Used if kind == ND_NUM
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

typedef struct {
	Var *globals;
	Function *fns;
} Program;

Program *parse(Token *tok);

//
// typing.c
//

typedef enum {
	TY_CHAR,
	TY_INT,
	TY_PTR,
	TY_FUNC,
	TY_ARRAY,
} TypeKind;

struct Type {
	TypeKind kind;

	int size; // sizeof() value この型のオブジェクト1つがメモリ上で何バイトを必要とするか

    // Pointer-to or array-of type. We intentionally use the same member
	// to represent pointer/array duality in C.
	//
	// In many contexts in which a pointer is expected, we examine this
	// member instead of "kind" member to determine whether a type is a
	// pointer or not. That means in many contexts "array of T" is
	// naturally handled as if it were "pointer to T", as required by
	// the C spec.
	Type *base;

	// Declaration
	Token *name;

	// Array
	int array_len; // Arrayの要素数

	// Function type
	Type *return_ty;
	Type *params;
	Type *next;
};

extern Type *ty_char;
extern Type *ty_int;

bool is_integer(Type *ty);
Type *copy_type(Type *ty);
Type *pointer_to(Type *base);
Type *func_type(Type *return_ty);
Type *array_of(Type *base, int size);
void add_type(Node *node);

//
// codegen.c
//

void codegen(Program *prog);
