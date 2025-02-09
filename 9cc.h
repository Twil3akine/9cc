#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 tokenize.c
 */

// トークンの種類
typedef enum {
	TK_RESERVED,     // 記号
	TK_NUM,          // 整数
	TK_EOF,          // 入力の終了
} TokenKind;

// トークン型
typedef struct Token Token;

struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *loc;
	int len;
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

bool equal(Token *tok, char *op);
Token *skip(Token *tok, char *op);

bool at_eof();

Token *new_token(TokenKind kind, char *start, char *end);

bool startswith(char *p, char *q);

Token *tokenize(char *p);

/*
 parse.c
 */

// 抽象構文木のノードの種類
typedef enum {
	ND_ADD, // +
	ND_SUB, // -
	ND_MUL, // *
	ND_DIV, // /
	ND_NEG, // unary -
	ND_AND, // &
	ND_OR , // |
	ND_XOR, // ^
	ND_EQ , // ==
	ND_NE , // !=
	ND_LT , // <
	ND_LE , // <=
	ND_NUM, // 整数
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
};

Node *new_node(NodeKind kind);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);

Node *new_num(int val);

static Node *primary();
static Node *unary();
static Node *mul();
static Node *add();
static Node *relational();
static Node *equality();
static Node *and();
static Node *xor();
static Node *or();
static Node *expr();

Node *parse(Token *tok);

/*
 generate.c
 */

static void expression(Node *node);
void generate(Node *node);

/*
 main.c
 */
int main(int argc, char *argv[]);
