#include "9cc.h"

static void push(void) {
	printf("	push rax\n");
}

static void pop(char *arg) {
	printf("	pop %s\n", arg);
}

static void gen_addr(Node *node) {
	if (node->kind == ND_LVAR) {
		int offset = (node->name - 'a' + 1) * 8;
		printf("	lea rax, [rbp-%d]\n", offset);
		return;
	}

	error("not an lvalue");
}

static void gen_expr(Node *node) {
	switch (node->kind) {
		case ND_NUM:
			printf("	mov rax, %d\n", node->val);
			return;

		case ND_NEG:
			gen_expr(node->lhs);
			printf("	neg rax\n");
			return;

		case ND_LVAR:
			gen_addr(node);
			printf("mov rax, [rax]\n");
			return;

		case ND_ASSIGN:
			gen_addr(node->lhs);
			push();
			gen_expr(node->rhs);
			pop("rdi");
			printf("	mov [rdi], rax\n");
			return;
	}

	gen_expr(node->rhs);
	push();
	gen_expr(node->lhs);
	pop("rdi");

	switch (node->kind) {
		case ND_ADD:
			printf("	add rax, rdi\n");
			return;
		case ND_SUB:
			printf("	sub rax, rdi\n");
			return;
		case ND_MUL:
			printf("	imul rax, rdi\n");
			return;
		case ND_DIV:
			printf("	cqo\n");
			printf("	idiv rdi\n");
			return;
		case ND_AND:
			printf("	and rax, rdi\n");
			return;
		case ND_OR:
			printf("	or rax, rdi\n");
			return;
		case ND_XOR:
			printf("	xor rax, rdi\n");
			return;
		case ND_EQ:
		case ND_NE:
		case ND_LT:
		case ND_LE:
			printf("	cmp rax, rdi\n");
            if (node->kind == ND_EQ) printf("	sete al\n");
			else if (node->kind == ND_NE) printf("	setne al\n");
			else if (node->kind == ND_LT) printf("	setl al\n");
			else if (node->kind == ND_LE) printf("	setle al\n");
			
            printf("	movzx rax, al\n");
			return;
	}
	
	// printf("	push rax\n");
}

static void gen_stmt(Node *node) {
	if (node->kind == ND_STMT) {
		gen_expr(node->lhs);
		return;
	}

	error("invalid statement\n");
}

void generate(Node *node) {
    // アセンブリ前半の出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	printf("	push rbp\n");
	printf("	mov rbp, rsp\n");
	printf("	sub rsp, 208\n");

    for (Node *n = node; n; n = n->next) {
		gen_stmt(n);
	}

    // スタックトップに式全体があるはずなので、
	// RAX にロードして、関数の返り値にする。
	printf("	mov rsp, rbp\n");
	printf("	pop rbp\n");
	printf("	ret\n");
	printf(".section .note.GNU-stack, \"\", @progbits\n");
}