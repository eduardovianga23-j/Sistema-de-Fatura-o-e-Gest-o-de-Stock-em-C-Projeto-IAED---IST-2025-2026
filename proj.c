#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

#include "structs.h"
#include "commands.h"

/** @brief Liberta toda a memória */
void free_all(Sistema *s) {
    Product *p = s->head_p;
    while (p) { Product *t = p->next; free(p->description); free(p); p = t; }

    BasketItem *b = s->head_b;
    while (b) { BasketItem *t = b->next; free(b); b = t; }

    Invoice *i = s->head_i;
    while (i) { Invoice *t = i->next; free(i->name); free(i); i = t; }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    Sistema s = {NULL, NULL, 0, NULL, NULL, 1, {-1}};

    /* 🔥 Inicializa todas as taxas como inválidas */
    for (int i = 0; i < 26; i++)
        s.taxas[i] = -1;

    if (argc > 1) {
        /* 🔥 Lê taxas do ficheiro (SUBSTITUI defaults) */
        FILE *f = fopen(argv[1], "r");
        if (f) {
            char ch; int v;
            while (fscanf(f, " %c %d", &ch, &v) == 2)
                s.taxas[ch - 'A'] = v;
            fclose(f);
        }
    } else {
        /* 🔥 Apenas se NÃO houver ficheiro → usar defaults */
        s.taxas[0] = 0;   /* A */
        s.taxas[1] = 6;   /* B */
        s.taxas[2] = 13;  /* C */
        s.taxas[3] = 23;  /* D */
    }

    char cmd;
    while (scanf(" %c", &cmd) == 1 && cmd != 'q') {
        if (cmd == 'p') cmd_p(&s);
        else if (cmd == 'l') cmd_l(&s);
        else if (cmd == 'a') cmd_a(&s);
        else if (cmd == 'r') cmd_r(&s);
        else if (cmd == 'f') cmd_f(&s);
        else if (cmd == 'c') cmd_c(&s);
        else if (cmd == 'd') cmd_d(&s);
    }

    free_all(&s);
    return 0;
}