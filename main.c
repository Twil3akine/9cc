#include "9cc.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        error("%s: invalid number of arguments.", argv[0]);
    }

    // 入力をトークン化する
    Token *tok = tokenize(argv[1]);

    printf("\n--- Token ---\n");
    Token *dum = tok;
    while (dum->next != NULL) {
        printf("%s", dum->loc);
        dum = dum->next;
    }
    printf("\n--- ----- ---\n");

    // トークン化されたものから抽象構文木を作る
    Node *node = parse(tok);

    // 抽象構文木にしたがって走る
    generate(node);

    return 0;
}