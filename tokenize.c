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

// 次のトークンが期待している記号ならば、トークンを進めて
// True を返し、それ以外ならば、False を返す。
bool consume(Token *tok, char *op) {
	if (tok->kind != TK_RESERVED ||
        strlen(op) != tok->len ||
		memcmp(tok->str, op, tok->len)) {
		return false;
	}
	tok = tok->next;
	return true;
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

		error_at(p, "Cannot Tokenize.");
	}

	new_token(TK_EOF, cur, p, 0);

	return head.next;
}