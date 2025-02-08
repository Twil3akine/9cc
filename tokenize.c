#include "9cc.h"

// 入力
static char *user_input;

// エラー報告の関数
// printf と同じ引数
void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

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

static void verror_at(char *loc, char *fmt, va_list ap) {
	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, ""); // print pos spaces.
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_tok(Token *tok, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	verror_at(tok->str, fmt, ap);
}

bool equal(Token *tok, char *op) {
	return memcmp(tok->str, op, tok->len) == 0 && op[tok->len] == '\0';
}

Token *skip(Token *tok, char *op) {
	if (!equal(tok, op)) {
		error_tok(tok, "expected '%s'", op);
	}
	return tok->next;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	tok->len = len;

	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
    user_input = p;
	Token head = {};
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

		// 複数文字記号
		if (startswith(p, "==") || startswith(p, "!=") ||
			startswith(p, "<=") || startswith(p, ">=")) {
			cur = cur->next = new_token(TK_RESERVED, cur, p, 2);
			p += cur->len;
			continue;
		}

		// 単文字記号
		if (strchr("+-*/()&|^<>", *p)) {
			cur = cur->next = new_token(TK_RESERVED, cur, p++, 1);
			continue;
		}
		
		// 数字
		if (isdigit(*p)) {
			cur = cur->next = new_token(TK_NUM, cur, p, 0);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		error_at(p, "Cannot Tokenize.");
	}

	cur = cur->next = new_token(TK_EOF, cur, p, 0);

	return head.next;
}