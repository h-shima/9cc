#include "9cc.h"

static Node *new_node(NodeKind kind);
static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_unary(NodeKind kind, Node *expr);
static Node *new_num(int val);

static Function *funcdef();
static Node *stmt();
static Node *expr();
static Node *assign();
static Node *equality();
static Node *relational();
static Node *add();
static Node *mul();
static Node *unary();
static Node *primary();
static Node *compound_stmt();
static Node *expr_stmt();
static void func_call(Token *tok, Node *node);

static Var *find_var(Token *tok);

// パースの時に、引数のトークンの変数がすでにパース済みか検索する
// なければNULLを返す
static Var *find_var(Token *tok) {
	for (Var *var = locals; var; var = var->next) {
		if (var->len == tok->len && !memcmp(tok->str, var->name, var->len)) {
			return var;
		}
	}
	return NULL;
}

static Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	return node;
}

static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs  = lhs;
	node->rhs  = rhs;
	return node;
}

static Node *new_unary(NodeKind kind, Node *expr) {
	Node *node = new_node(kind);
	node->lhs = expr;
	return node;
}

static Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;
	return node;
}

//
// ****** EBNF ******
//

// program = funcdef*

// funcdef = ident "("(ident ("," ident)*)?")" compound-stmt

// stmt	= "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//      | "while" "(" expr ")" stmt
//      | compound-stmt
//      | expr_stmt ";"

// compound-stmt = "{" stmt* "}"

// expr-stmt = expr

// expr = assign

// assign = equality ("=" assign)?

// equality = relational ("==" relational | "!=" relational)*

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*

// add = mul ("+" mul | "-" mul)*

// mul = unary ("*" unary | "/" unary)*

// unary = ("+" | "-") unary
//       | primary

// primary = "(" expr ")"
//         | ident     // identの一つ先のトークンを読み、
//         | funcall // そのidentが変数名なのか関数名なのか見分ける
//         | num

// funcall = ident "(" (assign ("," assign)*)? ")"

//
// ****** EBNF ******
//

// program = funcdef*
Function *program() {
	Function head = {};
	Function *cur = &head;
	while (!at_eof()) {
		cur->next = funcdef();
		cur = cur->next;
	}

	return head.next;
}

// TODO: 引数を取れるように修正
// funcdef = ident "(" ")" compound_stmt  今
// funcdef = ident "("(ident ("," ident)*)?")" compound-stmt　これから
static Function *funcdef() {
	Function *fn = calloc(1, sizeof(Function));
	Node *node = calloc(1, sizeof(Node));
	Node *blk;

	if (token->kind == TK_IDENT) {
		node->kind = ND_FUNCDEF;
		node->funcname = strndup(token->str, token->len);
		token = token->next;
	} else {
		error("識別子ではありません");
	}

	expect("(");
	expect(")");

	blk = compound_stmt();
	node->body = blk->body;

	fn->name = node->funcname;
	fn->node = node->body;
	return fn;
}

// stmt	= "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "for" "(" expr_stmt? ";" expr? ";" expr_stmt? ")" stmt
//      | "while" "(" expr ")" stmt
//      | compound-stmt
//      | expr_stmt ";"
static Node *stmt() {
	Node *node;

	if(equal("return")) {
		token = token->next;
		node = new_node(ND_RETURN);
		node->lhs = expr();

		expect(";");
		return node;
	} else if (equal("if")) {
		token = token->next;

		// if (A) B
		// IFノードのlhsにAの評価結果, rhsにBの評価結果が格納される
		node = new_node(ND_IF);
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
		node = new_node(ND_WHILE);
		expect("(");
		node->cond = expr();
		expect(")");

		node->then = stmt();
		return node;
	} else if (equal("for")) {
		token = token->next;

		// "for" "(" expr? ";" expr? ";" expr? ";" ")" stmt
		node = new_node(ND_FOR);
		expect("(");

		if (!consume(";")) {
			node->init = new_unary(ND_EXPR_STMT, expr());
			consume(";");
		}

		if (!consume(";")) {
			node->cond = expr();
			consume(";");
		}

		if (!consume(")")) {
			node->inc = new_unary(ND_EXPR_STMT, expr());
			consume(")");
		}

		node->then = stmt();
		return node;
	} else if (equal("{")) {
		compound_stmt();
	} else {
		node = new_unary(ND_EXPR_STMT, expr()); // statement == expr ";" の時、exprは式文なので
		expect(";");
		return node;
	}
}

// compound_stmt = "{" stmt* "}"
static Node *compound_stmt() {
	consume("{");
	Node head = {};
	Node *cur = &head;
	while(!consume("}"))
		cur = cur->next = stmt();

	Node *node = new_node(ND_BLOCK);
	node->body = head.next;
	return node;
}

// expr_stmt = expr
static Node *expr_stmt() {
	Node *node = expr();
	return node;
}

// expr = assign
static Node *expr() {
	Node *node = assign();
	return node;
}

// assign = equality ("=" assign)?
static Node *assign() {
	Node *node = equality();

	if (consume("=")) {
		node = new_binary(ND_ASSIGN, node, assign());
	}

	return node;
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_binary(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_binary(ND_NE, node, relational());
		else
			return node;
	}
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
	Node *node = add();

	for (;;) {
		if (consume("<"))
			node = new_binary(ND_LT, node, add());
		else if (consume("<="))
			node = new_binary(ND_LE, node, add());
		else if (consume(">"))
			node = new_binary(ND_LT, add(), node);
		else if (consume(">="))
			node = new_binary(ND_LE, add(), node);
		else
			return node;
	}
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_binary(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_binary(ND_SUB, node, mul());
		else
			return node;
	}
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_binary(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_binary(ND_DIV, node, unary());
		else
			return node;
	}
}

// unary = ("+" | "-")? unary
//       | primary
static Node *unary() {
	if (consume("+"))
		return unary();
	if (consume("-"))
		return new_binary(ND_SUB, new_num(0), unary());

	return primary();
}

// primary = num
//         | func_call identの一つ先のトークンを読み、そのidentが変数名なのか関数名なのか見分ける
//         | ident
//         | "(" expr ")"
static Node *primary() {
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
			func_call(tok, node);
		} else {
			// 変数名の場合
			node->kind = ND_VAR;

			Var *var = find_var(tok);

			if (var) {
				node->offset = var->offset;
			} else {
				var = calloc(1, sizeof(Var));
				var->next   = locals;
				var->name   = tok->str;
				var->len    = tok->len;
				// locals はポインタなので、実体が無い場合は 0 が入っている
				if (locals) {
					var->offset = locals->offset + 8;
				} else {
					var->offset = 8;
				}
				node->offset = var->offset;
				locals = var;
			}
		}

		return node;
	}

	return new_num(expect_number());
}

//ident "(" (expression ("," expression)*)? ")"
static void func_call(Token *tok, Node *node) {
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
}
