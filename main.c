#include "9cc.h"

char *user_input;

int main(int argc, char **argv) {

	if (argc != 2) {
		error("引数の個数が正しくありません");
		return 1;
	}

	// 引数で渡された文字列全体をトークナイズしてパースする
	user_input = argv[1];
	token = tokenize(user_input);
	program();
	codegen();
	return 0;
}
