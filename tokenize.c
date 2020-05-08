#include "9cc.h"

static Token *new_token(TokenKind kind, Token *cur, char *str, int len);
static bool startswith(char *p, char *q);
static int is_alnum(char c);
Token *token;

//
// Token operators
//
bool equal(char *op) {
	return strlen(op) == token->len &&
		   !strncmp(token->str, op, token->len);
}

Token *skip(char *op) {
	if (!equal(op))
		error("expected '%s'\n", op);
	return token->next;
}

bool consume(char *op) {
	if (!equal(op))
		return false;
	token = token->next;
	return true;
}

void expect(char *op) {
	if (!equal(op))
		error_at(token->str,"'%s'ではありません", op);
	token = token->next;
}

Token *consume_ident() {
	Token *tok = token;

	if (token->kind == TK_IDENT) {
		token = token->next;
		return tok;
	}
	return NULL;
}

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

// 新しいトークンを作成してcurに繋げる
static Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	tok->len = len;
	return tok;
}

// #memcmp
// int memcmp(const void *str1, const void *str2, size_t len);
// str1とstr2を"len文字分"比較して等しい場合は0を返す
static bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

static int is_alnum(char c) {
	return ('a' <= c && c <= 'z') ||
	       ('A' <= c && c <= 'Z') ||
			('0' <= c && c <= '9') ||
			(c == '_');
}

// Tokenize `user_input` and returns new tokens.
Token *tokenize(char *input) {
	char *p = input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RESERVED, cur, p, 6);
			p += 6;
			continue;
		}

		if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
			cur = new_token(TK_RESERVED, cur, p, 5);
			p += 5;
			continue;
		}

		if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
			cur = new_token(TK_RESERVED, cur, p, 3);
			p += 3;
			continue;
		}

		if (isalpha(*p)) {
			char *q = p;
			p++;
			while (isalnum(*p)) {
				p++;
			}

			cur = new_token(TK_IDENT, cur, q, p - q);
			continue;
		}

		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		if (strchr("+-*/()<>;={},",*p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0); // 整数が何文字続くかわからないため、一旦lenに0を渡す
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q; // strtolで抽出された整数の数分だけポインタが先に進むので、p(先に進んだポインタ) - q(元々のポインタ) = 整数トークンの長さ　となる
			continue;
		}

		error_at(p, "数ではありません");
	}

	new_token(TK_EOF, cur, p, 0);
	return head.next;
}
