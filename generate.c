#include "9cc.h"

static void expression(Node *node) {
	if (node->kind == ND_NUM) {
		printf("	mov rax, %d\n", node->val);
		return;
	}
	if (node->kind == ND_NEG) {
		expression(node->lhs);
		printf("	neg rax\n");
		return;
	}

	expression(node->rhs);
	printf("	push rax\n");
	expression(node->lhs);
	printf("	pop rdi\n");

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
			
            printf("	movzb rax, al\n");
			return;
	}
	
	// printf("	push rax\n");
}

void generate(Node *node) {
    // アセンブリ前半の出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

    expression(node);

    // スタックトップに式全体があるはずなので、
	// RAX にロードして、関数の返り値にする。
	// printf("	pop rax\n");
	printf("	ret\n");
	printf(".section .note.GNU-stack, \"\", @progbits\n");
}