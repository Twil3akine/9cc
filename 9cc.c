#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
	TK_RESERVED,		// 記号
	TK_NUM,					// 整数
	TK_EOF,					// 入力の終了
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
};

// 現在見ているトークン
Token *token;

// エラー報告の関数
// printf と同じ引数
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

// 次のトークンが期待している記号ならば、トークンを進めて
// True を返し、それ以外ならば、False を返す。
bool consume(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		return false;
	}
	token = token->next;
	return true;
}

// 次のトークンが期待している記号の時は、トークンを進めて、
// それ以外の場合はエラーを返す。
void expect_symbol(char op) {
	if (token->kind != TK_RESERVED || token->str[0] != op) {
		error("Current token is not '%c'.", op);
	}
	token = token->next;
}

// 次のトークンが数値の場合、トークンを進め、その値を返す。
// それ以外ならば、エラーを返す。
int expect_number() {
	if (token->kind != TK_NUM) {
		error("Current token is not a number");
	}
	int val = token->val;
	token = token->next;

	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;

	return tok;
}

Token *tokenize(char *p) {
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		if (*p == '+' || *p == '-') {
			cur = new_token(TK_RESERVED, cur, p++);
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

		error("Cannot Tokenize.");
	}

	new_token(TK_EOF, cur, p);

	return head.next;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		error("Not corrently numbers of arguments.");
		return 1;
	}

	// トークナイズ
	token = tokenize(argv[1]);

	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	printf("	mov rax, %d\n", expect_number());

	while (!at_eof()) {
		if (consume('+')) {
			printf("	add rax, %d\n", expect_number());
			continue;
		}
		expect_symbol('-');
		printf("	sub	rax, %d\n", expect_number());
	}

	printf("	ret \n");
	printf(".section .note.GNU-stack, \"\", @progbits\n");

	return 0;
}
