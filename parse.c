#include "9cc.h"

LVar *locals;

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

Node *new_unary(NodeKind kind, Node *expr) {
	Node *node = new_node(kind);
	node->lhs = expr;

	return node;
}

Node *new_num(int val) {
	Node *node = new_node(ND_NUM);
	node->val = val;

	return node;
}

Node *new_var_node(LVar *var) {
	Node *node = new_node(ND_LVAR);
	node->var = var;

	return node;
}

LVar *new_lvar(char *name) {
	LVar *var = calloc(1, sizeof(LVar));
	var->name = name;
	var->next = locals;
	locals = var;

	return var;
}

static LVar *find_var(Token *tok) {
	for (LVar *var = locals; var; var = var->next) {
		if (strlen(var->name) == tok->len && !strncmp(tok->loc, var->name, tok->len)) return var;
	}
	return NULL;
}

/*
 program    = stmt*
 stmt       = expr ';'
 expr       = assign
 assign     = or ('=' assign)?
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
static Node *assign(Token **rest, Token *tok);
static Node *expr_stmt(Token **rest, Token *tok);
static Node *expr(Token **rest, Token *tok);

static Node *primary(Token **rest, Token *tok) {
	// 次トークンが '(' -> ')' になるはず
	if (equal(tok, "(")) {
		Node *node = expr(&tok, tok->next);
        *rest = skip(tok, ")");

		return node;
	}

	if (tok->kind == TK_IDENT) {
		LVar *var = find_var(tok);
		if (!var) var = new_lvar(strndup(tok->loc, tok->len));
		*rest = tok->next;
		
		return new_var_node(var);
	}

	// でないなら、数値になるはず
	if (tok->kind == TK_NUM) {
		Node *node = new_num(tok->val);
		*rest = tok->next;

		return node;
	}
}

static Node *unary(Token **rest, Token *tok) {
	if (equal(tok, "+")) {
		return unary(rest, tok->next);
	}
	if (equal(tok, "-")) {
		return new_unary(ND_NEG, unary(rest, tok->next));
	}
	return primary(rest, tok);
}

static Node *mul(Token **rest, Token *tok) {
	Node *node = unary(&tok, tok);
	for (;;) {
		if (equal(tok, "*")) {
			node = new_binary(ND_MUL, node, unary(&tok, tok->next));
			continue;
		} 
		if (equal(tok, "/")) {
			node = new_binary(ND_DIV, node, unary(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *add(Token **rest, Token *tok) {
	Node *node = mul(&tok, tok);
	for (;;) {
		if (equal(tok, "+")) {
			node = new_binary(ND_ADD, node, mul(&tok, tok->next));
			continue;
		}
		if (equal(tok, "-")) {
			node = new_binary(ND_SUB, node, mul(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *relational(Token **rest, Token *tok) {
	Node *node = add(&tok, tok);
	for (;;) {
		if (equal(tok, "<")) {
			node = new_binary(ND_LT, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, "<=")) {
			node = new_binary(ND_LE, node, add(&tok, tok->next));
			continue;
		}
		if (equal(tok, ">")) {
			node = new_binary(ND_LT, add(&tok, tok->next), node);
			continue;
		}
		if (equal(tok, ">=")) {
			node = new_binary(ND_LE, add(&tok, tok->next), node);
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *equality(Token **rest, Token *tok) {
	Node *node = relational(&tok, tok);

	for (;;) {
		if (equal(tok, "==")) {
			node = new_binary(ND_EQ, node, relational(&tok, tok->next));
			continue;
		}
		if (equal(tok, "!=")) {
			node = new_binary(ND_NE, node, relational(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *and(Token **rest, Token *tok) {
	Node *node = equality(&tok, tok);
	for (;;) {
		if (equal(tok, "&")) {
			node = new_binary(ND_AND, node, equality(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *xor(Token **rest, Token *tok) {
	Node *node = and(&tok, tok);
	for (;;) {
		if (equal(tok, "^")) {
			node = new_binary(ND_XOR, node, and(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *or(Token **rest, Token *tok) {
	Node *node = xor(&tok, tok);
	for (;;) {
		if (equal(tok, "|")) {
			node = new_binary(ND_OR, node, xor(&tok, tok->next));
			continue;
		}

		*rest = tok;
		return node;
	}
}

static Node *assign(Token **rest, Token *tok) {
	Node *node = or(&tok, tok);

	if (equal(tok, "=")) {
		node = new_binary(ND_ASSIGN, node, assign(&tok, tok->next));
	}

	*rest = tok;
	return node;
}

static Node *expr(Token **rest, Token *tok) {
	return assign(rest, tok);
}

static Node *expr_stmt(Token **rest, Token *tok) {
	Node *node = new_unary(ND_STMT, expr(&tok, tok));

	*rest = skip(tok, ";");
	return node;
}

static Node *stmt(Token **rest, Token *tok) {
	return expr_stmt(rest, tok);
}

Function *parse(Token *tok) {
    Node head = {};
	Node *cur = &head;

	while (tok->kind != TK_EOF) {
		cur = cur->next = stmt(&tok, tok);
	}

	Function *prog = calloc(1, sizeof(Function));
	prog->body = head.next;
	prog->locals = locals;

    return prog;
}