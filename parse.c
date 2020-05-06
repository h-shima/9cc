#include "9cc.h"

// セミコロン区切りで複数の式が書けるため、パースの結果としての複数のノードを保存しておくための配列
Node *code[100];

bool consume(char *op);
Token *consume_ident();
void expect(char *op);
int  expect_number();
bool at_eof();

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_ident(Token *tok);

Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

// パースの時に、引数のトークンの変数がすでにパース済みか検索する
// なければNULLを返す
LVar *find_lvar(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}
	return NULL;
}

// 次のトークンが期待している記号の時には、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) {
		return false;
	}

	token = token->next;
	return true;
}

// Check whether current token is `op` or not.
bool equal(char *op) {
	return strlen(op) == token->len &&
			!strncmp(token->str, op, token->len);
}

Token *consume_ident() {
	Token *tok = token;

	if (token->kind == TK_IDENT) {
		token = token->next;
		return tok;
	}

	return NULL;
}

// 次のトークンが期待している記号の時には、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len)) {
		error_at(token->str,"'%s'ではありません", op);
	}

	token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "数ではありません");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs  = lhs;
	node->rhs  = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val  = val;
	return node;
}

Node *new_node_ident(Token *tok) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_LVAR;
	node->offset = (tok->str[0] - 'a' + 1) * 8;
	return node;
}

void program() {
	int i = 0;
	while (!at_eof())
		code[i++] = stmt();

	code[i] = NULL;
}

Node *stmt() {
	Node *node;

	if(equal("return")) {
		token = token->next;

		node = calloc(1, sizeof(Node));
		node->kind = ND_RETURN;
		node->lhs = expr();

		expect(";");
		return node;
	} else if (equal("if")) {
		token = token->next;

		// if (A) B
		// IFノードのlhsにAの評価結果, rhsにBの評価結果が格納される
		node = calloc(1, sizeof(Node));
		node->kind = ND_IF;
		expect("(");
		node->cond = expr();
		expect(")");

		node->then = stmt();

		if (equal("else")) {
			token = token->next;

			node->els = stmt();
		}

		return node;
	} else if (equal("while")) {
		token = token->next;

		// "while" "(" expr ")" stmt
		node = calloc(1, sizeof(Node));
		node->kind = ND_WHILE;
		expect("(");
		node->cond = expr();
		expect(")");

		node->then = stmt();
		return node;
	} else if (equal("for")) {
		token = token->next;

		// "for" "(" expr? ";" expr? ";" expr? ";" ")" stmt
		node = calloc(1, sizeof(Node));
		node->kind = ND_FOR;
		expect("(");

		if (!consume(";")) {
			node->init = new_node(ND_EXPR_STMT, expr(), NULL);
			consume(";");
		}

		if (!consume(";")) {
			node->cond = expr();
			consume(";");
		}

		if (!consume(")")) {
			node->inc = new_node(ND_EXPR_STMT, expr(), NULL);
			consume(")");
		}

		node->then = stmt();
		return node;
	} else if (consume("{")) {
		// "{" stmt* "}"
		Node head = {};
		Node *cur = &head;
		while(!consume("}"))
			cur = cur->next = stmt();

		Node *node = calloc(1, sizeof(Node));
		node->kind = ND_BLOCK;
		node->body = head.next;
		return node;
	} else {
		node = new_node(ND_EXPR_STMT, expr(), NULL); // statement == expr ";" の時、exprは式文なので
		expect(";");
		return node;
	}
}

Node *expr() {
	Node *node = assign();
	return node;
}

Node *assign() {
	Node *node = equality();

	if (consume("=")) {
		node = new_node(ND_ASSIGN, node, assign());
	}

	return node;
}

Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NE, node, relational());
		else
			return node;
	}
}

Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume("<="))
			node = new_node(ND_LE, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume(">="))
			node = new_node(ND_LE, add(), node);
		else
			return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else
			return node;
	}
}

// unary = ("+" | "-")? unary
//       | primary
Node *unary() {
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), unary());

	return primary();
}

// primary = num
//         | ident ("(" (expression ("," expression)*)? ")")? identの一つ先のトークンを読み、そのidentが変数名なのか関数名なのか見分ける
//         | "(" expr ")"
Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}

	Token *tok = consume_ident();
	if (tok) {
		Node *node = calloc(1, sizeof(Node));

		// 関数呼び出しの場合
		if (consume("(")) {
			node->kind = ND_FUNCALL;
			node->funcname = strndup(tok->str, tok->len);

			Node head = {};
			Node *cur = &head;

			if (!equal(")")) {
				cur = cur->next = expr();

				while (consume(",")) {
					cur = cur->next = expr();
				}
			}

			node->args = head.next;
			expect(")");
		} else {
			// 変数名の場合
			node->kind = ND_LVAR;

			LVar *lvar = find_lvar(tok);

			if (lvar) {
				node->offset = lvar->offset;
			} else {
				lvar = calloc(1, sizeof(LVar));
				lvar->next   = locals;
				// TODO: strndupに変更する
				lvar->name   = tok->str;
				lvar->len    = tok->len;
				// locals はポインタなので、実体が無い場合は 0 が入っている
				if (locals) {
					lvar->offset = locals->offset + 8;
				} else {
					lvar->offset = 8;
				}
				node->offset = lvar->offset;
				locals = lvar;
			}
		}

		return node;
	}

	return new_node_num(expect_number());
}
