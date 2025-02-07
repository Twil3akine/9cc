#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 入力プログラム
char *user_input;

// エラー箇所の報告
void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;

	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	
	exit(1);
}

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
	int len;
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
bool consume(char *op) {
	if (token->kind != TK_RESERVED ||
		  strlen(op) != token->len ||
			memcmp(token->str, op, token->len)) {
		return false;
	}
	token = token->next;
	return true;
}

// 次のトークンが記号の場合、トークンを進める。
// それ以外の場合は、エラーを返す。
void expect_symbol(char *op) {
	if (token->kind != TK_RESERVED ||
		  strlen(op) != token->len ||
			memcmp(token->str, op, token->len)) {
		error_at(token->str, "expected \"%s\"", op);
	}
	token = token->next;
}

// 次のトークンが数値の場合、トークンを進め、その値を返す。
// それ以外ならば、エラーを返す。
int expect_number() {
	if (token->kind != TK_NUM) {
		error_at(token->str, "Current token is not a number");
	}
	int val = token->val;
	token = token->next;

	return val;
}

// EOFなのかを判断
bool at_eof() {
	return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;
	cur->next = tok;

	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
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

		// 複数文字記号
		if (startswith(p, "==") || startswith(p, "!=") ||
			  startswith(p, "<=") || startswith(p, ">=")) {
			cur = new_token(TK_RESERVED, cur, p, 2);
			p += 2;
			continue;
		}

		// 単文字記号
		if (strchr("+-*/()&|^<>", *p)) {
			cur = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}
		
		// 数字
		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(token->str, "Cannot Tokenize.");
	}

	new_token(TK_EOF, cur, p, 0);

	return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_AND, // &
	ND_OR , // |
	ND_XOR, // ^
	ND_EQ , // ==
	ND_NE , // !=
	ND_LT , // <
	ND_LE , // <=
	ND_NUM, // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node *new_node(NodeKind kind) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;

	return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = new_node(kind);
	node->lhs = lhs;
	node->rhs = rhs;

	return node;
}

Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;

	return node;
}

/*
 expr       = or
 or         = xor ('|' xor)*
 xor        = and ('^' and)*
 equality   = relational ('==' relational | '!=' relational)*
 relational = add ('<' add | '<=' add | '>' add | '>=' add)*
 and        = add ('&' add)*
 add        = mul ('+' mul | '-' mul)*
 mul        = unary ('*' unary | '/' unary)*
 unary      = ('+' | '-')? primary
 primary    = num | '(' or ')'
 */

Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *and();
Node *xor();
Node *or();
Node *expr();

Node *primary() {
	// 次トークンが '(' -> ')' になるはず
	if (consume("(")) {
		Node *node = or();
		expect_symbol(")");
		return node;
	}
	// でないなら、数値になるはず
	return new_num(expect_number());
}

Node *unary() {
	if (consume("+")) {
		return unary();
	} else if (consume("-")) {
		return new_binary(ND_SUB, new_num(0), unary());
	}
	return primary();
}

Node *mul() {
	Node *node = unary();
	for (;;) {
		if (consume("*")) {
			node = new_binary(ND_MUL, node, unary());
		} else if (consume("/")) {
			node = new_binary(ND_DIV, node, unary());
		} else {
			return node;
		}
	}
}

Node *add() {
	Node *node = mul();
	for (;;) {
		if (consume("+")) {
			node = new_binary(ND_ADD, node, mul());
		} else if (consume("-")) {
			node = new_binary(ND_SUB, node, mul());
		} else {
			return node;
		}
	}
}

Node *relational() {
	Node *node = add();
	for (;;) {
		if (consume("<")) {
			node = new_binary(ND_LT, node, add());
		} else if (consume("<=")) {
			node = new_binary(ND_LE, node, add());
		} else if (consume(">")) {
			node = new_binary(ND_LT, add(), node);
		} else if (consume(">=")) {
			node = new_binary(ND_LE, add(), node);
		} else {
			return node;
		}
	}
}

Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("==")) {
			node = new_binary(ND_EQ, node, relational());
		} else if (consume("!=")) {
			node = new_binary(ND_NE, node, relational());
		} else {
			return node;
		}
	}
}

Node *and() {
	Node *node = equality();
	for (;;) {
		if (consume("&")) {
			node = new_binary(ND_AND, node, equality());
		} else {
			return node;
		}
	}
}
 
Node *xor() {
	Node *node = and();
	for (;;) {
		if (consume("^")) {
			node = new_binary(ND_XOR, node, and());
		} else {
			return node;
		}
	}
}

Node *or() {
	Node *node = xor();
	for (;;) {
		if (consume("|")) {
			node = new_binary(ND_OR, node, xor());
		} else {
			return node;
		}
	}
}

Node *expr() {
	return or();
}

void generate(Node *node) {
	if (node->kind == ND_NUM) {
		printf("	push %d\n", node->val);
		return;
	}

	generate(node->lhs);
	generate(node->rhs);

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
		case ND_AND:
			printf("	and rax, rdi\n");
			break;
		case ND_OR:
			printf("	or rax, rdi\n");
			break;
		case ND_XOR:
			printf("	xor rax, rdi\n");
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
	
	printf("	push rax\n");
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		error("Number of arguments is not correctly.");
		return 1;
	}

	// トークナイズしたのちに、パース
	user_input = argv[1];
	token = tokenize(user_input);
	Node *node = or();

	// アセンブリ前半の出力
	printf(".intel_syntax noprefix\n");
	printf(".globl main\n");
	printf("main:\n");

	// 抽象構文木を歩きながら、コード生成
	generate(node);

	// スタックトップに式全体があるはずなので、
	// RAX にロードして、関数の返り値にする。
	printf("	pop rax\n");
	printf("	ret\n");
	printf(".section .note.GNU-stack, \"\", @progbits\n");

	return 0;
}
