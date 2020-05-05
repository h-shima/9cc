#include "9cc.h"

char *user_input;
LVar *locals;

int main(int argc, char **argv) {

	if (argc != 2) {
		error("引数の個数が正しくありません");
		return 1;
	}

	// 引数で渡された文字列全体をトークナイズしてパースする
	user_input = argv[1];
	token = tokenize(user_input);
	program();

	// あらかじめ必要な変数領域を計算する
    // locals はポインタなので、実体が無い場合は 0 が入っている
	int offset;
	if (locals) {
		offset = locals->offset;
	} else {
		offset = 0;
	}

	// アセンブリの前半部分を出力
	printf(".intel_syntax noprefix\n");
	printf(".global main\n");
	printf("main:\n");

	// プロローグ
	// 変数領域を確保する
	printf("	push rbp\n");
	// ベースポインタが、スタックポインタと同じアドレスを指すようにrspの値をrbpにコピーする
	printf("	mov rbp, rsp\n");
	// 変数の領域分だけスタックポインタを下げて変数領域を確保する
	printf("	sub rsp, %d\n", offset);

	// 先頭の式から順にコード生成
	for (int i = 0; code[i] ; i++) {
		gen(code[i]);
	}

	// エピローグ
	// 最後の式の結果がRAXに残っているのでそれが返り値になる
	printf(".Lreturn:\n");
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	return 0;
}
