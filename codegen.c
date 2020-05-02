#include "9cc.h"

// 式を左辺値（左辺に書くことができる値のこと）として評価する
// 変数のノードを引数として取り、その変数のアドレスを計算してそれをスタックにプッシュする
void gen_lval(Node *node) {
	if (node->kind != ND_LVAR)
		error("代入の左辺値が変数ではありません");

	printf("	mov rax, rbp\n");
	printf("	sub rax, %d\n", node->offset);
	printf("	push rax\n"); // 引数nodeのローカル変数の格納されているアドレスがスタックにプッシュされる
}

void gen(Node *node) {
	switch (node->kind) {
		case ND_NUM:
			printf("	push %d\n", node->val);
			return;
		case ND_LVAR:
			// nodeの変数が格納されているアドレスをスタックにプッシュする
			gen_lval(node);
			printf("	pop rax\n");
			// nodeの変数が格納されているアドレスからローカル変数の値を読み出し、
			// raxレジスタにコピーする
			printf("	mov rax, [rax]\n");
			printf("	push rax\n");
			return;
		case ND_ASSIGN:
			// ノードの左辺（代入先の変数）の変数が格納されているアドレスをスタックにプッシュする
			gen_lval(node->lhs);
			// ノードの右辺（左辺に代入する変数や数値を計算して数値にした値）をスタックにプッシュする
			gen(node->rhs);

			printf("	pop rdi\n");
			printf("	pop rax\n");
			// raxレジスタの値（変数が格納されているアドレス）をアドレスとみなしてrdiレジスタの値（代入する数値）をストアする
			printf("	mov [rax], rdi\n");
			printf("	push rdi\n");
			return;
	}

	// 現ノードの左側をコンパイル
	gen(node->lhs);
	// 現ノードの右側をコンパイル
	gen(node->rhs);

	printf("	pop rdi\n");
	printf("	pop rax\n");

	switch (node->kind) {
		case ND_ADD:
			printf("	add rax, rdi\n");
			break;
		case ND_SUB:
			printf("	sub rax, rdi\n");
			break;
		case ND_MUL:
			printf("	imul rax, rdi\n");
			break;
		case ND_DIV:
			printf("	cqo\n");
			printf("	idiv rdi\n");
			break;
		case ND_EQ:
			printf("	cmp rax, rdi\n");
			printf("	sete al\n");
			printf("	movzb rax, al\n");
			break;
		case ND_NE:
			printf("	cmp rax, rdi\n");
			printf("	setne al\n");
			printf("	movzb rax, al\n");
			break;
		case ND_LT:
			printf("	cmp rax, rdi\n");
			printf("	setl al\n");
			printf("	movzb rax, al\n");
			break;
		case ND_LE:
			printf("	cmp rax, rdi\n");
			printf("	setle al\n");
			printf("	movzb rax, al\n");
			break;
	}

	// 計算結果のraxレジスタをプッシュする
	printf("	push rax\n");
}
