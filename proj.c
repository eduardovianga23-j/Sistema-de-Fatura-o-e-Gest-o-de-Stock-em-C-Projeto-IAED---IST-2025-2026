/* iaed26 - ist1119719 - project */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "structs.h"

#define MAX_PRODUCTS 10000
#define MAX_LINE 65536

/* ================= UTIL ================= */

void* smalloc(size_t size) {
    void *p = malloc(size);
    if (!p) { printf("No memory.\n"); exit(0); }
    return p;
}

char* sstrdup(const char *s) {
    char *d = smalloc(strlen(s) + 1);
    return strcpy(d, s);
}

void free_all(Sistema *s) {
    Product *p = s->head_p;
    while (p) { Product *t = p->next; free(p->description); free(p); p = t; }

    BasketItem *b = s->head_b;
    while (b) { BasketItem *t = b->next; free(b); b = t; }

    Invoice *i = s->head_i;
    while (i) { Invoice *t = i->next; free(i->name); free(i); i = t; }
}

void read_name_or_token(char *buffer) {
    int c, i = 0;
    while (isspace(c = getchar()) && c != '\n');

    if (c == '"') {
        while ((c = getchar()) != '"' && c != EOF)
            buffer[i++] = (char)c;
    } else {
        while (c != EOF && !isspace(c) && c != '\n') {
            buffer[i++] = (char)c;
            c = getchar();
        }
        if (c != EOF) ungetc(c, stdin);
    }
    buffer[i] = '\0';
}

/* ================= AUX ================= */

Product* find_product(Sistema *s, const char *ean) {
    for (Product *p = s->head_p; p; p = p->next)
        if (!strcmp(p->ean, ean)) return p;
    return NULL;
}

BasketItem* find_basket_item(Sistema *s, const char *ean) {
    for (BasketItem *b = s->head_b; b; b = b->next)
        if (!strcmp(b->ean, ean)) return b;
    return NULL;
}

int is_ean_valid(const char *e) {
    int len = strlen(e), soma = 0;
    if (len != 8 && len != 13) return 0;

    for (int i = 0; i < len; i++)
        if (!isdigit(e[i])) return 0;

    for (int i = 0; i < len - 1; i++) {
        int d = e[i] - '0';
        soma += (i % 2 == 0) ? d : d * 3;
    }

    int check = (10 - (soma % 10)) % 10;
    return (e[len - 1] - '0') == check;
}

long calc_total_iva(long price, int qty, int tax) {
    double t = price * (double)qty * (1.0 + tax / 100.0);
    return (long)(t + 0.5);
}

int match_wild(const char *p, const char *s) {
    if (!*p) return !*s;
    if (*p == '*') return match_wild(p+1, s) || (*s && match_wild(p, s+1));
    if (*s && (*p == '?' || *p == *s)) return match_wild(p+1, s+1);
    return 0;
}

/* ================= cmd_p ================= */

void cmd_p(Sistema *s) {
    char ean[MAX_LINE], iva, desc[MAX_LINE];
    double pr;
    int qty;

    if (scanf("%s %c %lf %d", ean, &iva, &pr, &qty) != 4) return;
    if (scanf(" %[^\n]", desc) != 1) desc[0] = '\0';

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }
    if (iva < 'A' || iva > 'Z' || s->taxas[iva-'A'] == -1) {
        printf("invalid iva\n"); return;
    }
    if (pr <= 0) { printf("invalid price\n"); return; }
    if (qty < 0) { printf("invalid quantity\n"); return; }

    int len = strlen(desc);
    if (len == 0 || len > 50 || !isupper((unsigned char)desc[0])) {
        printf("invalid description\n");
        return;
    }

    long price = (long)(pr * 100 + 0.5);
    Product *p = find_product(s, ean);

    if (p) {
        if (p->in_basket > 0) {
            printf("product in use\n");
            return;
        }
        p->stock += qty;
        p->price = price;
        p->iva_code = iva;
        free(p->description);
        p->description = sstrdup(desc);
    } else {
        if (s->num_p >= MAX_PRODUCTS) {
            printf("invalid product\n"); return;
        }

        p = smalloc(sizeof(Product));
        strcpy(p->ean, ean);
        p->description = sstrdup(desc);
        p->price = price;
        p->iva_code = iva;
        p->stock = qty;
        p->sold_qty = 0;
        p->in_basket = 0;
        p->next = NULL;

        if (!s->head_p) s->head_p = p;
        else s->tail_p->next = p;
        s->tail_p = p;
        s->num_p++;
    }

    printf("%d\n", p->stock);
}

/* ================= cmd_l ================= */

void cmd_l(Sistema *s) {
    char tok[MAX_LINE];
    int c;

    while (isspace(c = getchar()) && c != '\n');

    if (c == '\n' || c == EOF) {
        int found = 0;

        for (Product *p = s->head_p; p; p = p->next) {
            if (p->stock > 0) {
                printf("%s %c %.2f %d %d %s\n",
                    p->ean, p->iva_code,
                    p->price / 100.0,
                    p->sold_qty + p->in_basket,
                    p->stock,
                    p->description);
                found = 1;
            }
        }

        if (!found) printf("*: no such product\n");
        return;
    }

    ungetc(c, stdin);

    while (scanf("%s", tok) == 1) {
        int found = 0;

        for (Product *p = s->head_p; p; p = p->next) {
            if (match_wild(tok, p->ean) && p->stock > 0) {
                printf("%s %c %.2f %d %d %s\n",
                    p->ean, p->iva_code,
                    p->price / 100.0,
                    p->sold_qty + p->in_basket,
                    p->stock,
                    p->description);
                found = 1;
            }
        }

        if (!found) printf("%s: no such product\n", tok);

        while (isspace(c = getchar()) && c != '\n');
        if (c == '\n' || c == EOF) break;
        else ungetc(c, stdin);
    }
}

/* ================= cmd_a ================= */

void cmd_a(Sistema *s) {
    char buf[MAX_LINE], ean[MAX_LINE];
    int qty = 1, c;

    while (isspace(c = getchar()) && c != '\n');

    /* ================= LISTAR CESTO ================= */
    if (c == '\n' || c == EOF) {
        for (BasketItem *b = s->head_b; b; b = b->next) {
            Product *p = find_product(s, b->ean);
            if (p && b->quantity > 0) {
                printf("%c %.2f %d %.2f %s\n",
                    p->iva_code,
                    p->price / 100.0,
                    b->quantity,
                    calc_total_iva(p->price, b->quantity,
                        s->taxas[p->iva_code - 'A']) / 100.0,
                    p->description);
            }
        }
        return;
    }

    /* ================= INPUT ================= */
    ungetc(c, stdin);
    scanf("%s", buf);

    if (is_ean_valid(buf)) {
        strcpy(ean, buf);
        qty = 1;
    } else {
        /* validar inteiro */
        int is_number = 1;
        for (int i = 0; buf[i]; i++) {
            if (!isdigit(buf[i]) && !(i == 0 && buf[i] == '-')) {
                is_number = 0;
                break;
            }
        }

        if (!is_number) {
            printf("invalid ean\n");
            return;
        }

        qty = atoi(buf);
        scanf("%s", ean);
    }

    if (!is_ean_valid(ean)) {
        printf("invalid ean\n");
        return;
    }

    Product *p = find_product(s, ean);
    if (!p) {
        printf("%s: no such product\n", ean);
        return;
    }

    BasketItem *b = find_basket_item(s, ean);

    /* ================= ADICIONAR ================= */
    if (qty > 0) {
        if (p->stock < qty) {
            printf("no stock\n");
            return;
        }

        p->stock -= qty;
        p->in_basket += qty;

        if (b) {
            b->quantity += qty;
        } else {
            b = smalloc(sizeof(BasketItem));
            strcpy(b->ean, ean);
            b->quantity = qty;
            b->next = s->head_b;
            s->head_b = b;
        }
    }

    /* ================= REMOVER ================= */
    else if (qty < 0) {
        if (!b || b->quantity < -qty) {
            printf("invalid quantity\n");
            return;
        }

        b->quantity += qty;      /* qty negativo */
        p->stock -= qty;         /* devolve stock */
        p->in_basket += qty;

        /* 🔥 REMOÇÃO CORRETA DA LISTA */
        if (b->quantity == 0) {
            BasketItem *prev = NULL, *cur = s->head_b;

            while (cur && cur != b) {
                prev = cur;
                cur = cur->next;
            }

            if (!prev) s->head_b = cur->next;
            else prev->next = cur->next;

            free(cur);
            b = NULL;   /* 🔥 evita use-after-free */
        }
    }

    /* ================= OUTPUT ================= */
    int total = (b) ? b->quantity : 0;

    printf("%c %.2f %d %.2f %s\n",
        p->iva_code,
        p->price / 100.0,
        total,
        calc_total_iva(p->price, total,
            s->taxas[p->iva_code - 'A']) / 100.0,
        p->description);
}

/* ================= cmd_f ================= */

void cmd_f(Sistema *s) {
    char nome[MAX_LINE] = "Cliente final";
    long nif = 999999999, total = 0;
    int items = 0;

    for (BasketItem *b = s->head_b; b; b = b->next) {
        Product *p = find_product(s, b->ean);
        if (p) {
            p->sold_qty += b->quantity;
            p->in_basket -= b->quantity;
            items += b->quantity;
            total += calc_total_iva(p->price, b->quantity,
                s->taxas[p->iva_code-'A']);
        }
    }

    Invoice *nv = smalloc(sizeof(Invoice));
    nv->id = s->next_inv_id++;
    nv->nif = nif;
    nv->name = sstrdup(nome);
    nv->items_count = items;
    nv->total_cents = total;
    nv->next = s->head_i;
    s->head_i = nv;

    printf("%d %.2f %d\n", items, total/100.0, nv->id);

    while (s->head_b) {
        BasketItem *t = s->head_b;
        s->head_b = t->next;
        free(t);
    }
}

void cmd_d(Sistema *s) {
    char arg[MAX_LINE];
    int c;

    if (scanf("%s", arg) != 1)
        return;

    while ((c = getchar()) == ' ' || c == '\t');

    /* === CASO PRODUTO (tem quantidade) === */
    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);

        int qty;
        if (scanf("%d", &qty) != 1)
            return;

        if (!is_ean_valid(arg)) {
            printf("invalid ean\n");
            return;
        }

        Product *p = find_product(s, arg);
        if (!p) {
            printf("%s: no such product\n", arg);
            return;
        }

        for (BasketItem *b = s->head_b; b; b = b->next) {
            if (!strcmp(b->ean, arg) && b->quantity > 0) {
                printf("product in use\n");
                return;
            }
        }

        if (qty <= 0 || qty > p->stock) {
            printf("invalid quantity\n");
            return;
        }

        p->stock -= qty;

        printf("%d %s\n", p->stock, p->description);

        if (p->stock == 0) {
            Product *prev = NULL, *cur = s->head_p;

            while (cur && cur != p) {
                prev = cur;
                cur = cur->next;
            }

            if (!prev) s->head_p = cur->next;
            else prev->next = cur->next;

            if (s->tail_p == cur)
                s->tail_p = prev;

            free(cur->description);
            free(cur);
            s->num_p--;
        }
    }

    /* === CASO FATURA === */
    else {
        int id = atoi(arg);

        Invoice *prev = NULL, *cur = s->head_i;

        while (cur && cur->id != id) {
            prev = cur;
            cur = cur->next;
        }

        if (!cur) {
            printf("%d: no such invoice\n", id);
            return;
        }

        printf("%.2f %ld %s\n",
            cur->total_cents / 100.0,
            cur->nif,
            cur->name);

        if (!prev) s->head_i = cur->next;
        else prev->next = cur->next;

        free(cur->name);
        free(cur);
    }
}

void cmd_r(Sistema *s) {
    char ean[MAX_LINE];
    int c;

    while (isspace(c = getchar()) && c != '\n');

    if (c == '\n' || c == EOF) {
        long items = 0, f = 0, total = 0;

        for (Invoice *i = s->head_i; i; i = i->next) {
            items += i->items_count;
            f++;
            total += i->total_cents;
        }

        printf("%ld %ld %.2f\n", items, f, total/100.0);

        for (int i = 0; i < 26; i++)
            if (s->taxas[i] != -1)
                printf("%c %d%%\n", 'A'+i, s->taxas[i]);

        return;
    }

    ungetc(c, stdin);
    scanf("%s", ean);

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }

    Product *p = find_product(s, ean);
    if (!p) { printf("%s: no such product\n", ean); return; }

    printf("%d %d %s\n",
        p->stock,
        p->sold_qty + p->in_basket,
        p->description);
}

void cmd_c(Sistema *s) {
    char nome[MAX_LINE]; int c, fnd = 0;
    while (isspace(c = getchar()) && c != '\n');

    if (c == '\n' || c == EOF) {
        for (Invoice *i = s->head_i; i; i = i->next)
            printf("%d %.2f %s\n", i->id, i->total_cents/100.0, i->name);
    } else {
        ungetc(c, stdin);
        read_name_or_token(nome);
        for (Invoice *i = s->head_i; i; i = i->next)
            if (!strcmp(i->name, nome)) {
                printf("%d %.2f %s\n", i->id, i->total_cents/100.0, i->name);
                fnd = 1;
            }
        if (!fnd) printf("%s: no such client\n", nome);
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    Sistema s = {NULL, NULL, 0, NULL, NULL, 1, {-1}};

    for (int i = 0; i < 26; i++) s.taxas[i] = -1;
    s.taxas[0]=0; s.taxas[1]=6; s.taxas[2]=13; s.taxas[3]=23;

   if (argc > 1) {
    /* limpar taxas */
    for (int i = 0; i < 26; i++)
        s.taxas[i] = -1;

    FILE *f = fopen(argv[1], "r");
    if (f) {
        char ch;
        int v;
        while (fscanf(f, " %c %d", &ch, &v) == 2) {
            s.taxas[ch - 'A'] = v;
        }
        fclose(f);
    }
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
