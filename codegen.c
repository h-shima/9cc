#include "9cc.h"

static int top;
static int labelseq = 1;
static char *argreg8[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static char *argreg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
static Function *current_fn;

static char *reg(int idx) {
	static char *r[] = {"r10", "r11", "r12", "r13", "r14", "r15"};
	if (idx < 0 || sizeof(r) / sizeof(*r) <= idx)
		error("register out of range: %d", idx);
	return r[idx];
}

static void gen_expr(Node *node);
static void gen_stmt(Node *node);

// Pushes the given node's address to the stack.
static void gen_addr(Node *node) {
	switch (node->kind) {
		case ND_VAR:
			if (node->var->is_local)
				printf("  lea %s, [rbp-%d]\n", reg(top++), node->var->offset);
			else
				printf("  mov %s, offset %s\n", reg(top++), node->var->name);
			return;
		case ND_DEREF:
			gen_expr(node->lhs);
			return;
	}

	error_tok(node->tok, "not an lvalue");
}

// Load a value from where the stack top is pointing to.
static void load(Type *ty) {
	if (ty->kind == TY_ARRAY) {
		// If it is an array, do nothing because in general we can't load
		// an entire array to a register. As a result, the result of an
		// evaluation of an array becomes not the array itself but the
		// address of the array. In other words, this is where "array is
		// automatically converted to a pointer to the first element of
		// the array in C" occurs.
		return;
	}

	char *r = reg(top - 1);
	// char型の場合
	if (ty->size == 1)
		printf("  movsx %s, byte ptr [%s]\n", r, r);
	else
		printf("  mov %s, [%s]\n", reg(top - 1), reg(top - 1));
}

static void store(Type *ty) {
	char *rd = reg(top - 1);
	char *rs = reg(top - 2);

	if (ty->size == 1)
		printf("  mov [%s], %sb\n", rd, rs);
	else
		printf("  mov [%s], %s\n", rd, rs);
	top--;
}

// Generate code for a given node.
static void gen_expr(Node *node) {
	switch (node->kind) {
		case ND_NUM:
			printf("  mov %s, %lu\n", reg(top++), node->val);
			return;
		case ND_VAR:
			gen_addr(node);
			load(node->ty);
			return;
		case ND_DEREF:
			gen_expr(node->lhs);
			load(node->ty);
			return;
		case ND_ADDR:
			gen_addr(node->lhs);
			return;
		case ND_ASSIGN:
			if (node->ty->kind == TY_ARRAY)
				error_tok(node->tok, "not an lvalue");

			gen_expr(node->rhs);
			gen_addr(node->lhs);
			store(node->ty);
			return;
		case ND_STMT_EXPR:
			for (Node *n = node->body; n; n = n->next)
				gen_stmt(n);
			top++;
			return;
		case ND_FUNCALL: {
			int nargs = 0;
			for (Node *arg = node->args; arg; arg = arg->next) {
				gen_expr(arg);
				nargs++;
			}

			for (int i = 1; i <= nargs; i++)
				printf("  mov %s, %s\n", argreg64[nargs - i], reg(--top));

			printf("  push r10\n");
			printf("  push r11\n");
			printf("  mov rax, 0\n");
			printf("  call %s\n", node->funcname);
			printf("  pop r11\n");
			printf("  pop r10\n");
			printf("  mov %s, rax\n", reg(top++));
			return;
		}
	}

	gen_expr(node->lhs);
	gen_expr(node->rhs);

	char *rd = reg(top - 2);
	char *rs = reg(top - 1);
	top--;

	switch (node->kind) {
		case ND_ADD:
			printf("  add %s, %s\n", rd, rs);
			return;
		case ND_SUB:
			printf("  sub %s, %s\n", rd, rs);
			return;
		case ND_MUL:
			printf("  imul %s, %s\n", rd, rs);
			return;
		case ND_DIV:
			printf("  mov rax, %s\n", rd);
			printf("  cqo\n");
			printf("  idiv %s\n", rs);
			printf("  mov %s, rax\n", rd);
			return;
		case ND_EQ:
			printf("  cmp %s, %s\n", rd, rs);
			printf("  sete al\n");
			printf("  movzx %s, al\n", rd);
			return;
		case ND_NE:
			printf("  cmp %s, %s\n", rd, rs);
			printf("  setne al\n");
			printf("  movzx %s, al\n", rd);
			return;
		case ND_LT:
			printf("  cmp %s, %s\n", rd, rs);
			printf("  setl al\n");
			printf("  movzx %s, al\n", rd);
			return;
		case ND_LE:
			printf("  cmp %s, %s\n", rd, rs);
			printf("  setle al\n");
			printf("  movzx %s, al\n", rd);
			return;
		default:
			error_tok(node->tok, "invalid expression");
	}
}

static void gen_stmt(Node *node) {
	switch (node->kind) {
		case ND_IF: {
			int seq = labelseq++;
			if (node->els) {
				gen_expr(node->cond);
				printf("  cmp %s, 0\n", reg(--top));
				printf("  je  .L.else.%d\n", seq);
				gen_stmt(node->then);
				printf("  jmp .L.end.%d\n", seq);
				printf(".L.else.%d:\n", seq);
				gen_stmt(node->els);
				printf(".L.end.%d:\n", seq);
			} else {
				gen_expr(node->cond);
				printf("  cmp %s, 0\n", reg(--top));
				printf("  je  .L.end.%d\n", seq);
				gen_stmt(node->then);
				printf(".L.end.%d:\n", seq);
			}
			return;
		}
		case ND_FOR: {
			int seq = labelseq++;
			if (node->init)
				gen_stmt(node->init);
			printf(".L.begin.%d:\n", seq);
			if (node->cond) {
				gen_expr(node->cond);
				printf("  cmp %s, 0\n", reg(--top));
				printf("  je  .L.end.%d\n", seq);
			}
			gen_stmt(node->then);
			if (node->inc)
				gen_stmt(node->inc);
			printf("  jmp .L.begin.%d\n", seq);
			printf(".L.end.%d:\n", seq);
			return;
		}
		case ND_BLOCK:
			for (Node *n = node->body; n; n = n->next)
				gen_stmt(n);
			return;
		case ND_RETURN:
			gen_expr(node->lhs);
			printf("  mov rax, %s\n", reg(--top));
			printf("  jmp .L.return.%s\n", current_fn->name);
			return;
		case ND_EXPR_STMT:
			gen_expr(node->lhs);
			top--;
			return;
		default:
			error_tok(node->tok, "invalid statement");
	}
}

static void emit_data(Program *prog) {
	printf(".data\n");

	for (Var *var = prog->globals; var; var = var->next) {
		printf("%s:\n", var->name);

		if (!var->init_data) {
			printf("  .zero %d\n", var->ty->size);
			continue;
		}

		for (int i = 0; i < var->ty->size; i++) {
			printf("  .byte %d\n", var->init_data[i]);
		}
	}
}

static void emit_text(Program *prog) {
	printf(".text\n");

	for (Function *fn = prog->fns; fn; fn = fn->next) {
		printf(".globl %s\n", fn->name);
		printf("%s:\n", fn->name);
		current_fn = fn;

		// Prologue. r12-15 are callee-saved registers.
		printf("  push rbp\n");
		printf("  mov rbp, rsp\n");
		printf("  sub rsp, %d\n", fn->stack_size);
		printf("  mov [rbp-8], r12\n");
		printf("  mov [rbp-16], r13\n");
		printf("  mov [rbp-24], r14\n");
		printf("  mov [rbp-32], r15\n");

		// Save arguments to the stack
		int i = 0;
		for (Var *var = fn->params; var; var = var->next)
			i++;
		for (Var *var = fn->params; var; var = var->next) {
			if (var->ty->size == 1)
				printf("  mov [rbp-%d], %s\n", var->offset, argreg8[--i]);
			else
				printf("  mov [rbp-%d], %s\n", var->offset, argreg64[--i]);
		}

		// Emit code
		for (Node *n = fn->node; n; n = n->next) {
			gen_stmt(n);
			assert(top == 0);
		}

		// Epilogue
		printf(".L.return.%s:\n", fn->name);
		printf("  mov r12, [rbp-8]\n");
		printf("  mov r13, [rbp-16]\n");
		printf("  mov r14, [rbp-24]\n");
		printf("  mov r15, [rbp-32]\n");
		printf("  mov rsp, rbp\n");
		printf("  pop rbp\n");
		printf("  ret\n");
	}
}

void codegen(Program *prog) {
	printf(".intel_syntax noprefix\n");
	emit_data(prog);
	emit_text(prog);
}