#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node Node;
typedef struct Token Token;
typedef struct LVar LVar;
typedef struct Function Function;

// local variable
struct LVar {
	LVar *next;
	char *name;
	int offset;
};

// function
struct Function {
	Node *body;
	LVar *locals;
	int stack_size;
};

/*
 tokenize.c
 */

// トークンの種類
typedef enum {
	TK_RETURN,       // return
	TK_RESERVED,     // 記号
	TK_IDENT,        // 識別子
	TK_NUM,          // 整数
	TK_EOF,          // 入力の終了
} TokenKind;

// トークン型
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
	ND_ADD,    // +
	ND_SUB,    // -
	ND_MUL,    // *
	ND_DIV,    // /
	ND_NEG,    // unary -
	ND_AND,    // &
	ND_OR ,    // |
	ND_XOR,    // ^
	ND_EQ ,    // ==
	ND_NE ,    // !=
	ND_LT ,    // <
	ND_LE ,    // <=
	ND_STMT,   // ;
	ND_ASSIGN, // =
	ND_LVAR,   // 変数
	ND_NUM,    // 整数
	ND_RETURN  // return
} NodeKind;

// 抽象構文木のノードの型
struct Node {
	NodeKind kind;
	Node *next;
	Node *lhs;
	Node *rhs;
	LVar *var;
	int val;
};

Node *new_node(NodeKind kind);

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs);

Node *new_num(int val);

Function *parse(Token *tok);

/*
 generate.c
 */
void generate(Function *prog);

/*
 main.c
 */
int main(int argc, char *argv[]);
