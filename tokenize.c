#include "9cc.h"

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool startswith(char *p, char *q);
int is_alnum(char c);
Token *token;

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
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
bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

int is_alnum(char c) {
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
			cur = new_token(TK_IF, cur, p, 2);
			p += 2;
			continue;
		}

		if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
			cur = new_token(TK_RETURN, cur, p, 6);
			p += 6;
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

		if (strchr("+-*/()<>;=",*p)) {
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
