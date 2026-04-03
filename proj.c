/* iaed26 - ist1119719 - project */

/**
 * @file proj.c
 * @brief 
 * Billing system in C for managing products, cart, and invoices.
 * Supports products with EAN-8/EAN-13 validation, price, VAT, and stock.
 * Uses VAT rates from file or default values.
 * Handles cart operations with stock control and consistency.
 * Generates invoices with NIF, client name, total with VAT, and rounding.
 * Provides command-based input with dynamic memory and no global variables.
 * @author Eduardo João Vianga, ist1119719
 * @date March 20, 2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

#include "structs.h"

#define MAX_PRODUCTS 10000
#define MAX_LINE 65536

/* --- Utilitários --- */

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
        while ((c = getchar()) != '"' && c != EOF) buffer[i++] = (char)c;
    } else {
        while (c != EOF && !isspace(c) && c != '\n') { buffer[i++] = (char)c; c = getchar(); }
        if (c != EOF) ungetc(c, stdin);
    }
    buffer[i] = '\0';
}

/* --- Funções auxiliares --- */

Product* find_product(Sistema *s, const char *ean) {
    Product *p = s->head_p;
    while (p && strcmp(p->ean, ean)) p = p->next;
    return p;
}

BasketItem* find_basket_item(Sistema *s, const char *ean) {
    BasketItem *b = s->head_b;
    while (b) {
        if (!strcmp(b->ean, ean)) return b;
        b = b->next;
    }
    return NULL;
}

int basket_quantity(Sistema *s, const char *ean) {
    int qty = 0;
    for (BasketItem *b = s->head_b; b; b = b->next)
        if (!strcmp(b->ean, ean)) qty += b->quantity;
    return qty;
}

int is_ean_valid(const char *e) {
    int len = strlen(e), soma = 0;
    if (len != 8 && len != 13) return 0;
    for (int i = 0; i < len; i++) if (!isdigit(e[i])) return 0;
    for (int i = 0; i < len - 1; i++) {
        int d = e[i] - '0';
        soma += (i % 2 == 0) ? d : d * 3;
    }
    int check = (10 - (soma % 10)) % 10;
    return (e[len - 1] - '0') == check;
}

long calc_total_iva(long price, int qty, int tax_percent) {
    double total = (price * (double)qty) * (1.0 + tax_percent / 100.0);
    return (long)(total + 0.5);
}

int match_wild(const char *p, const char *s) {
    if (!*p) return !*s;
    if (*p == '*') return match_wild(p+1, s) || (*s && match_wild(p, s+1));
    if (*s && (*p == '?' || *p == *s)) return match_wild(p+1, s+1);
    return 0;
}

/* --- Comandos --- */

void cmd_p(Sistema *s) {
    char ean[MAX_LINE], iva_c, desc[MAX_LINE];
    double pr;
    int qty;

    if (scanf("%s %c %lf %d", ean, &iva_c, &pr, &qty) != 4) return;
    if (scanf(" %[^\n]", desc) != 1) {desc[0] = '\0';}

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }
    if (iva_c < 'A' || iva_c > 'Z' || s->taxas[iva_c - 'A'] == -1) { printf("invalid iva\n"); return; }
    if (pr <= 0) { printf("invalid price\n"); return; }
    if (qty < 0) { printf("invalid quantity\n"); return; }
   
int len = strlen(desc);

if (len == 0 || len > 50) {
    printf("invalid description\n");
    return;
}

if (!isalpha((unsigned char)desc[0]) || 
    !isupper((unsigned char)desc[0])) {
    printf("invalid description\n");
    return;
}

/* RESTO: caracteres imprimíveis */
for (int i = 0; i < len; i++) {
    if (desc[i] < 32 || desc[i] > 126) {
        printf("invalid description\n");
        return;
    }
}

    long novo_preco = (long)(pr * 100 + 0.5);

    Product *p = find_product(s, ean);

    if (p) {
        /* Não permitir alterar produto no cesto */
        if (find_basket_item(s, ean)) {
            printf("product in use\n");
            return;
        }

        p->stock += qty;
        p->price = novo_preco;
        p->iva_code = iva_c;
        free(p->description);
        p->description = sstrdup(desc);
    } else {
        if (s->num_p >= MAX_PRODUCTS) { printf("invalid product\n"); return; }

        p = smalloc(sizeof(Product));
        strcpy(p->ean, ean);
        p->description = sstrdup(desc);
        p->price = novo_preco;
        p->iva_code = iva_c;
        p->stock = qty;
        p->sold_qty = 0;
        p->next = NULL;

        if (!s->head_p) s->head_p = p;
        else s->tail_p->next = p;
        s->tail_p = p;
        s->num_p++;
    }

    printf("%d\n", p->stock);
}

void cmd_l(Sistema *s) {
    char tok[MAX_LINE];
    int c;

    /* saltar espaços */
    while (isspace(c = getchar()) && c != '\n');

    /* ============================= */
    /* 🔹 CASO: SEM ARGUMENTOS (l) */
    /* ============================= */
    if (c == '\n' || c == EOF) {
        int found_any = 0;

        for (Product *p = s->head_p; p; p = p->next) {
            if (p->stock > 0) {
                int qty = basket_quantity(s, p->ean);
                printf("%s %c %.2f %d %d %s\n",
                    p->ean, p->iva_code,
                    p->price / 100.0,
                    p->sold_qty + qty,
                    p->stock,
                    p->description);
                found_any = 1;
            }
        }

        /* ⚠️ IMPORTANTE: só aqui imprime "*" */
        if (!found_any) {
            printf("*: no such product\n");
        }

        return;
    }

    /* ============================= */
    /* 🔹 CASO: COM ARGUMENTOS */
    /* ============================= */
    ungetc(c, stdin);

    while (scanf("%s", tok) == 1) {
        int found_any = 0;

        for (Product *p = s->head_p; p; p = p->next) {
            if (match_wild(tok, p->ean) && p->stock > 0) {
                int qty = basket_quantity(s, p->ean);
                printf("%s %c %.2f %d %d %s\n",
                    p->ean, p->iva_code,
                    p->price / 100.0,
                    p->sold_qty + qty,
                    p->stock,
                    p->description);
                found_any = 1;
            }
        }

        /* 🔥 ERRO POR TOKEN */
        if (!found_any) {
            printf("%s: no such product\n", tok);
        }

        while (isspace(c = getchar()) && c != '\n');
        if (c == '\n' || c == EOF) break;
        else ungetc(c, stdin);
    }
}

void cmd_a(Sistema *s) {
    char buf[MAX_LINE], ean[MAX_LINE];
    int qty = 1, c;

    while (isspace(c = getchar()) && c != '\n');

    /* LISTAR CESTO */
    if (c == '\n' || c == EOF) {
        /* ordenar por EAN */
        BasketItem *arr[10000];
        int n = 0;

        for (BasketItem *b = s->head_b; b; b = b->next)
            if (b->quantity > 0)
                arr[n++] = b;

        /* bubble sort simples */
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                if (strcmp(arr[i]->ean, arr[j]->ean) > 0) {
                    BasketItem *tmp = arr[i];
                    arr[i] = arr[j];
                    arr[j] = tmp;
                }

        for (int i = 0; i < n; i++) {
            Product *p = find_product(s, arr[i]->ean);
            if (p) {
                printf("%c %.2f %d %.2f %s\n",
                    p->iva_code,
                    p->price / 100.0,
                    arr[i]->quantity,
                    calc_total_iva(p->price, arr[i]->quantity,
                        s->taxas[p->iva_code - 'A']) / 100.0,
                    p->description);
            }
        }
        return;
    }

    ungetc(c, stdin);
    scanf("%s", buf);

   if (is_ean_valid(buf)) {
    strcpy(ean, buf);
    qty = 1;
} else {
    /* garantir que é número */
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

    /* ➕ ADICIONAR */
    if (qty > 0) {
        if (p->stock < qty) {
            printf("no stock\n");
            return;
        }

        p->stock -= qty;

        if (b) b->quantity += qty;
        else {
            b = smalloc(sizeof(BasketItem));
            strcpy(b->ean, ean);
            b->quantity = qty;
            b->next = s->head_b;
            s->head_b = b;
        }
    }

    /* ➖ REMOVER */
    else if (qty < 0) {
        if (!b || b->quantity < -qty) {
            printf("invalid quantity\n");
            return;
        }

        b->quantity += qty;  /* qty é negativo */
        p->stock -= qty;     /* devolve stock */

        if (b->quantity == 0) {
            /* remover da lista */
            BasketItem *prev = NULL, *cur = s->head_b;
            while (cur && cur != b) {
                prev = cur;
                cur = cur->next;
            }

            if (!prev) s->head_b = b->next;
            else prev->next = b->next;

            free(b);
            b = NULL;
        }
    }

    int total_qty = b ? b->quantity : 0;

    printf("%c %.2f %d %.2f %s\n",
        p->iva_code,
        p->price / 100.0,
        total_qty,
        calc_total_iva(p->price, total_qty,
            s->taxas[p->iva_code - 'A']) / 100.0,
        p->description);
}

void cmd_f(Sistema *s) {
    char line[MAX_LINE], nome[MAX_LINE] = "Cliente final";
    long nif = 999999999, total = 0;
    int items = 0, c;

    while (isspace(c = getchar()) && c != '\n');

    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);
        read_name_or_token(line);

        int valid_nif = 1;
        if (strlen(line) == 9) {
            for (int i = 0; i < 9; i++)
                if (!isdigit(line[i])) valid_nif = 0;

            if (valid_nif) {
                nif = atol(line);
                read_name_or_token(nome);
            } else {
                strcpy(nome, line);
            }
        } else {
            strcpy(nome, line);
        }
    }

    if (!strcmp(nome, "error")) {
        while (s->head_b) {
            BasketItem *t = s->head_b;
            Product *p = find_product(s, t->ean);
            if (p) p->stock += t->quantity;
            s->head_b = t->next;
            free(t);
        }
        return;
    }

    /* calcular totais */
   /* calcular totais */
for (BasketItem *b = s->head_b; b; b = b->next) {
    Product *p = find_product(s, b->ean);
    if (p && b->quantity > 0) {
        p->sold_qty += b->quantity;
        items += b->quantity;
        total += calc_total_iva(p->price, b->quantity, s->taxas[p->iva_code-'A']);
    }
}

    Invoice *nv = smalloc(sizeof(Invoice));
    Invoice *curr = s->head_i, *prev = NULL;

    nv->id = s->next_inv_id++;
    nv->nif = nif;
    nv->name = sstrdup(nome);
    nv->items_count = items;
    nv->total_cents = total;

    while (curr && (strcmp(curr->name, nome) < 0 ||
           (strcmp(curr->name, nome) == 0 && curr->id < nv->id))) {
        prev = curr;
        curr = curr->next;
    }

    nv->next = curr;
    if (!prev) s->head_i = nv;
    else prev->next = nv;

    printf("%d %.2f %d\n", items, total / 100.0, nv->id);

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

    /* 🔹 SEM ARGUMENTOS */
    if (c == '\n' || c == EOF) {
        long total_items = 0;
        long total_facturas = 0;
        long total_valor = 0;

        for (Invoice *i = s->head_i; i; i = i->next) {
            total_items += i->items_count;
            total_facturas++;
            total_valor += i->total_cents;
        }

        printf("%ld %ld %.2f\n",
            total_items,
            total_facturas,
            total_valor / 100.0);

        /* imprimir IVA ordenado */
        for (int j = 0; j < 26; j++) {
            if (s->taxas[j] != -1) {
                printf("%c %d%%\n", 'A' + j, s->taxas[j]);
            }
        }

        return;
    }

    /* 🔹 COM EAN */
    ungetc(c, stdin);

    if (scanf("%s", ean) != 1)
        return;

    if (!is_ean_valid(ean)) {
        printf("invalid ean\n");
        return;
    }

    Product *p = find_product(s, ean);
    if (!p) {
        printf("%s: no such product\n", ean);
        return;
    }

    int total_vendido = p->sold_qty + basket_quantity(s, ean);

    printf("%d %d %s\n",
        p->stock,
        total_vendido,
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
