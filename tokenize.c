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
	verror_at(tok->loc, fmt, ap);
}

bool equal(Token *tok, char *op) {
	return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

Token *skip(Token *tok, char *op) {
	if (!equal(tok, op)) {
		error_tok(tok, "expected '%s'", op);
	}
	return tok->next;
}

Token *new_token(TokenKind kind, char *start, char *end) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->loc = &start[0];
	tok->len = end - start;

	return tok;
}

bool startswith(char *p, char *q) {
	return memcmp(p, q, strlen(q)) == 0;
}

static int read_punct(char *p) {
	if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
		return 2;
	}
	return ispunct(*p) ? 1 : 0;
}

Token *tokenize(char *p) {
    user_input = p;
	Token head = {};
	Token *cur = &head;

	while (*p) {
		// 空白
		if (isspace(*p)) {
			p++;
			continue;
		}

		// 数字
		if (isdigit(*p)) {
			cur = cur->next = new_token(TK_NUM, p, p);
			// printf("%s is digit.\n", cur->loc);
			char *q = p;
			cur->val = strtol(p, &p, 10);
			cur->len = p - q;
			continue;
		}

		// 変数
		if ('a' <= *p && *p <= 'z') {
			cur = cur->next = new_token(TK_IDENT, p, p+1);
			p++;
			continue;
		}

		// 記号
		int punct_len = read_punct(p);
		if (punct_len) {
			cur = cur->next = new_token(TK_RESERVED, p, p+punct_len);
			p += cur->len;
			continue;
		}

		error_at(p, "Cannot Tokenize.");
	}

	cur = cur->next = new_token(TK_EOF, p, p);

	return head.next;
}