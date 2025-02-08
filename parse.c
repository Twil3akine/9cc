#include "9cc.h"

// 次のトークンが記号の場合、トークンを進める。
// それ以外の場合は、エラーを返す。
void expect_symbol(Token *tok, char *op) {
	if (tok->kind != TK_RESERVED ||
        strlen(op) != tok->len ||
        memcmp(tok->str, op, tok->len)) {
		error_at(tok->str, "expected \"%s\"", op);
	}
	tok = tok->next;
}

// 次のトークンが数値の場合、トークンを進め、その値を返す。
// それ以外ならば、エラーを返す。
int expect_number(Token *tok) {
	if (tok->kind != TK_NUM) {
		error_at(tok->str, "Current token is not a number");
	}
	int val = tok->val;
	tok = tok->next;

	return val;
}


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
 and        = equality ('&' equality)*
 equality   = relational ('==' relational | '!=' relational)*
 relational = add ('<' add | '<=' add | '>' add | '>=' add)*
 add        = mul ('+' mul | '-' mul)*
 mul        = unary ('*' unary | '/' unary)*
 unary      = ('+' | '-')? primary
 primary    = num | '(' or ')'
 */

static Node *primary(Token **rest, Token *tok);
static Node *unary(Token **rest, Token *tok);
static Node *mul(Token **rest, Token *tok);
static Node *add(Token **rest, Token *tok);
static Node *relational(Token **rest, Token *tok);
static Node *equality(Token **rest, Token *tok);
static Node *and(Token **rest, Token *tok);
static Node *xor(Token **rest, Token *tok);
static Node *or(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);

static Node *primary(Token **rest, Token *tok) {
	// 次トークンが '(' -> ')' になるはず
	if (consume(tok, "(")) {
		Node *node = or(&tok, tok->next);
		expect_symbol(tok, ")");
        *rest = tok->next;
		return node;
	}
	// でないなら、数値になるはず
	return new_num(expect_number(tok));
}

static Node *unary(Token **rest, Token *tok) {
	if (consume(tok, "+")) {
		return unary(rest, tok->next);
	} else if (consume(tok, "-")) {
		return new_binary(ND_SUB, new_num(0), unary(rest, tok->next));
	}
	return primary(rest, tok);
}

static Node *mul(Token **rest, Token *tok) {
	Node *node = unary(&tok, tok);
	for (;;) {
		if (consume(tok, "*")) {
			node = new_binary(ND_MUL, node, unary(&tok, tok->next));
		} else if (consume(tok, "/")) {
			node = new_binary(ND_DIV, node, unary(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *add(Token **rest, Token *tok) {
	Node *node = mul(&tok, tok);
	for (;;) {
		if (consume(tok, "+")) {
			node = new_binary(ND_ADD, node, mul(&tok, tok->next));
		} else if (consume(tok, "-")) {
			node = new_binary(ND_SUB, node, mul(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *relational(Token **rest, Token *tok) {
	Node *node = add(&tok, tok);
	for (;;) {
		if (consume(tok, "<")) {
			node = new_binary(ND_LT, node, add(&tok, tok->next));
		} else if (consume(tok, "<=")) {
			node = new_binary(ND_LE, node, add(&tok, tok->next));
		} else if (consume(tok, ">")) {
			node = new_binary(ND_LT, add(&tok, tok->next), node);
		} else if (consume(tok, ">=")) {
			node = new_binary(ND_LE, add(&tok, tok->next), node);
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *equality(Token **rest, Token *tok) {
	Node *node = relational(&tok, tok);

	for (;;) {
		if (consume(tok, "==")) {
			node = new_binary(ND_EQ, node, relational(&tok, tok->next));
		} else if (consume(tok, "!=")) {
			node = new_binary(ND_NE, node, relational(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *and(Token **rest, Token *tok) {
	Node *node = equality(&tok, tok);
	for (;;) {
		if (consume(tok, "&")) {
			node = new_binary(ND_AND, node, equality(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *xor(Token **rest, Token *tok) {
	Node *node = and(&tok, tok);
	for (;;) {
		if (consume(tok, "^")) {
			node = new_binary(ND_XOR, node, and(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *or(Token **rest, Token *tok) {
	Node *node = xor(&tok, tok);
	for (;;) {
		if (consume(tok, "|")) {
			node = new_binary(ND_OR, node, xor(&tok, tok->next));
		} else {
            *rest = tok;
			return node;
		}
	}
}

static Node *expr(Token **rest, Token *tok) {
	return or(rest, tok);
}

Node *parse(Token *tok) {
    Node *node = expr(&tok, tok);
    return node;
}