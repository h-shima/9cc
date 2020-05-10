#include "9cc.h"

Var *locals;
static int labelseq = 1;
static void store();
static void load();
static void gen_addr(Node *node);
static void gen_expr(Node *node);
static void gen_stmt(Node *node);
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// 式を左辺値（左辺に書くことができる値のこと）として評価する
// 変数のノードを引数として取り、その変数のアドレスを計算してそれをスタックにプッシュする
static void gen_addr(Node *node) {
	if (node->kind != ND_VAR)
		error("代入の左辺値が変数ではありません");

	printf("	mov rax, rbp\n");
	printf("	sub rax, %d\n", node->offset);
	printf("	push rax\n"); // 引数nodeのローカル変数の格納されているアドレスがスタックにプッシュされる
}

static void store() {
	printf("	pop rdi\n");
	printf("	pop rax\n");
	// raxレジスタの値（変数が格納されているアドレス）をアドレスとみなしてrdiレジスタの値（代入する数値）をストアする
	printf("	mov [rax], rdi\n");
	printf("	push rdi\n");
}

static void load() {
	printf("	pop rax\n");
	// nodeの変数が格納されているアドレスからローカル変数の値を読み出し、
	// raxレジスタにコピーする
	printf("	mov rax, [rax]\n");
	printf("	push rax\n");
}

static void gen_expr(Node *node) {
	switch (node->kind) {
		case ND_NUM:
			printf("	push %d\n", node->val);
			return;
		case ND_VAR:
			// nodeの変数が格納されているアドレスをスタックにプッシュする
			gen_addr(node);
			load();
			return;
		case ND_ASSIGN:
			// ノードの左辺（代入先の変数）の変数が格納されているアドレスをスタックにプッシュする
			gen_addr(node->lhs);
			// ノードの右辺（左辺に代入する変数や数値を計算して数値にした値）をスタックにプッシュする
			gen_expr(node->rhs);
			store();
			return;
		case ND_FUNCALL: {
			int nargs = 0;

			for (Node *arg = node->args; arg; arg = arg->next) {
				gen_expr(arg);
				nargs++;
			}

			for (int i = 1; i <= nargs; i++) {
				printf("	pop rax\n");
				printf("	mov %s, rax\n", argreg[nargs - i]);
			}

			printf("	mov rax, 0\n");
			printf("	call %s\n", node->funcname);
			// 関数からの返り値がraxに格納されているのでスタックにプッシュする
			printf("	push rax\n");
			return;
		}
	}

	// 現ノードの左側をコンパイル
	gen_expr(node->lhs);
	// 現ノードの右側をコンパイル
	gen_expr(node->rhs);

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

static void gen_stmt(Node *node) {
	switch (node->kind) {
	case ND_EXPR_STMT:
		gen_expr(node->lhs);
		printf("	pop rax\n");
		return;
    case ND_BLOCK:
    	for (Node *n = node->body; n; n = n->next)
    		gen_stmt(n);
    	return;
    case ND_IF: {
    	int seq = labelseq++;

    	// if (A) B における A の評価結果がスタックトップに入る
    	gen_expr(node->cond);
    	printf("	pop rax\n");
    	printf("	cmp rax, 0\n");
    	printf("	je .Lelse%d\n", seq);
    	// if (A) B における B のアセンブリを出力
    	gen_stmt(node->then);
    	printf("	jmp .Lend%d\n", seq);
    	printf(".Lelse%d:\n", seq);
    	if (node->els) {
    		gen_stmt(node->els);
    	}
    	printf(".Lend%d:\n", seq);
    	return;
    }
    case ND_WHILE: {
    	int seq = labelseq++;

		printf(".Lbegin%d:\n", seq);
		gen_expr(node->cond);
		printf("	pop rax\n");
		printf("	cmp rax, 0\n");
		printf("	je .Lend%d\n", seq);
		gen_stmt(node->then);
		printf("	jmp .Lbegin%d\n", seq);
		printf(".Lend%d:\n", seq);
		return;
    }
    case ND_FOR: {
    	int seq = labelseq++;

		if (node->init) {
			gen_stmt(node->init);
		}
		printf(".Lbegin%d:\n", seq);
		if (node->cond) {
			gen_expr(node->cond);
			printf("	pop rax\n");
			printf("	cmp rax, 0\n");
			printf("	je .Lend%d\n", seq);
		}
		gen_stmt(node->then);
		if (node->inc) {
			gen_stmt(node->inc);
		}
		printf("	jmp .Lbegin%d\n", seq);
		printf(".Lend%d:\n", seq);
		return;
    }
    case ND_RETURN:
    	// returnの返り値になっている式の出力
    	gen_expr(node->lhs);
    	printf("	pop rax\n");
    	printf("jmp .Lreturn.%s\n", current_func->name);
    	return;
	}
}
// TODO: ハウスキーピング周りを綺麗にする。関数の引数とローカル変数の扱いを良い感じにする。
void codegen() {
	printf(".intel_syntax noprefix\n");

	while(current_func) {
		// あらかじめ必要な変数領域を計算する
		int offset;
		if (current_func->locals) {
			offset = current_func->locals->offset;
		} else {
			offset = 0;
		}

		// アセンブリの前半部分を出力
		printf(".global %s\n", current_func->name);
		printf("%s:\n", current_func->name);

		// プロローグ
		// 変数領域を確保する
		printf("        push rbp\n");
		// ベースポインタが、スタックポインタと同じアドレスを指すようにrspの値をrbpにコピーする
		printf("        mov rbp, rsp\n");
		// 変数の領域分だけスタックポインタを下げて変数領域を確保する
		printf("        sub rsp, %d\n", offset);

		for (Node *node = current_func->node; node; node = node->next) {
			gen_stmt(node);
		}

		// エピローグ
		// 最後の式の結果がRAXに残っているのでそれが返り値になる
		printf(".Lreturn.%s:\n", current_func->name);
		printf("        mov rsp, rbp\n");
		printf("        pop rbp\n");
		printf("        ret\n");

		current_func = current_func->next;
	}
}