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

int read_name_or_token(char *buffer, int *is_quoted) {
    int c, i = 0;
    *is_quoted = 0;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '"') {
        *is_quoted = 1;

        while ((c = getchar()) != '"' && c != '\n' && c != EOF) {
            buffer[i++] = (char)c;
        }

        if (c != '"') return 0;
    } 
    else {
        while (c != EOF && c != '\n' && c != ' ' && c != '\t') {
            buffer[i++] = (char)c;
            c = getchar();
        }

        if (c != EOF)
            ungetc(c, stdin);
    }

    buffer[i] = '\0';
    return 1;
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

int match_wild(const char *p, const char *s) {
    const char *star = NULL, *ss = NULL;

    while (*s) {
        if (*p == '?' || *p == *s) {
            p++; s++;
        }
        else if (*p == '*') {
            star = p++;
            ss = s;
        }
        else if (star) {
            p = star + 1;
            s = ++ss;
        }
        else {
            return 0;
        }
    }

    while (*p == '*') p++;

    return *p == '\0';
}

int valid_name(const char *name) {
    if (!name || !isalpha((unsigned char)name[0]))
        return 0;

    for (int i = 0; name[i]; i++)
        if (!isprint((unsigned char)name[i]))
            return 0;

    return 1;
}

int valid_nif(const char *s) {
    if (strlen(s) != 9) return 0;

    for (int i = 0; i < 9; i++)
        if (!isdigit(s[i])) return 0;

    if (s[0] == '0') return 0;

    return 1;
}

int basket_quantity(Sistema *s, const char *ean) {
    int q = 0;
    for (BasketItem *b = s->head_b; b; b = b->next)
        if (!strcmp(b->ean, ean)) q += b->quantity;
    return q;
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

    return ((10 - (soma % 10)) % 10) == (e[len - 1] - '0');
}

long calc_total_iva(long price, int qty, int tax) {
    long long base = (long long)price * qty;
    long long total = base * (100 + tax);
    return (long)((total + 50) / 100);
}

int valid_description(const char *d) {
    int len = strlen(d);
    if (len == 0 || len > 50) return 0;

    unsigned char c = (unsigned char)d[0];
    if (!((c >= 'A' && c <= 'Z') || c >= 192)) return 0;

    for (int i = 0; i < len; i++)
        if ((unsigned char)d[i] < 32) return 0;

    return 1;
}

/* ================= BASKET ================= */

void sort_basket(BasketItem **arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp(arr[j]->ean, arr[j + 1]->ean) > 0) {
                BasketItem *t = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = t;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

void list_basket(Sistema *s) {
    BasketItem *arr[10000];
    int n = 0;

    for (BasketItem *b = s->head_b; b; b = b->next)
        if (b->quantity > 0)
            arr[n++] = b;

    sort_basket(arr, n);

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
}

void clear_basket(Sistema *s) {
    while (s->head_b) {
        BasketItem *t = s->head_b;
        s->head_b = t->next;
        free(t);
    }
}

void calculate_totals(Sistema *s, int *items, long *total) {
    for (BasketItem *b = s->head_b; b; b = b->next) {
        Product *p = find_product(s, b->ean);
        if (p && b->quantity > 0) {
            p->sold_qty += b->quantity;
            *items += b->quantity;
            *total += calc_total_iva(p->price, b->quantity,
                s->taxas[p->iva_code - 'A']);
        }
    }
}

long long convert_str_centimos(const char *preco) {
    if (!preco || preco[0] == '-') return -1;

    long long euros = 0, cents = 0;
    char *dot = strchr(preco, '.');

    if (!dot) {
        return atoll(preco) * 100;
    }

    euros = atoll(preco);

    char cent_str[10];
    strncpy(cent_str, dot + 1, 3); /* até 3 dígitos */
    cent_str[3] = '\0';

    int len = strlen(cent_str);

    if (len == 0) {
        cents = 0;
    } 
    else if (len == 1) {
        cents = (cent_str[0] - '0') * 10;
    } 
    else {
        cents = (cent_str[0] - '0') * 10 + (cent_str[1] - '0');

        /* 🔥 arredondamento correto */
        if (len >= 3 && cent_str[2] >= '5')
            cents++;
    }

    if (cents >= 100) {
        euros++;
        cents -= 100;
    }

    return euros * 100 + cents;
}

/* ================= COMANDOS ================= */

void cmd_p(Sistema *s) {
    char ean[MAX_LINE], iva, desc[MAX_LINE], price_str[32];
    int qty;

    if (scanf("%s %c %s %d", ean, &iva, price_str, &qty) != 4)
        return;

    if (scanf(" %[^\n]", desc) != 1)
        desc[0] = '\0';

    long long price = convert_str_centimos(price_str);

    /* validações */
    if (!is_ean_valid(ean)) {
        printf("invalid ean\n");
        return;
    }

    if (iva < 'A' || iva > 'Z' || s->taxas[iva - 'A'] == -1) {
        printf("invalid iva\n");
        return;
    }

    if (price <= 0) {
        printf("invalid price\n");
        return;
    }

    if (qty < 0) {
        printf("invalid quantity\n");
        return;
    }

    if (!valid_description(desc)) {
        printf("invalid description\n");
        return;
    }

    Product *p = find_product(s, ean);

    if (p) {
        if (find_basket_item(s, ean)) {
            printf("product in use\n");
            return;
        }

        p->stock += qty;
        p->price = (long)price;
        p->iva_code = iva;

        free(p->description);
        p->description = sstrdup(desc);
    }
    else {
        if (s->num_p >= MAX_PRODUCTS) {
            printf("invalid product\n");
            return;
        }

        p = smalloc(sizeof(Product));

        strcpy(p->ean, ean);
        p->description = sstrdup(desc);
        p->price = (long)price;
        p->iva_code = iva;
        p->stock = qty;
        p->sold_qty = 0;
        p->next = NULL;

        if (!s->head_p)
            s->head_p = p;
        else
            s->tail_p->next = p;

        s->tail_p = p;
        s->num_p++;
    }

    printf("%d\n", p->stock);
}

void cmd_a(Sistema *s) {
    char buf[MAX_LINE], ean[MAX_LINE];
    int qty = 1, c;

   while ((c = getchar()) == ' ' || c == '\t') {}

    if (c == '\n' || c == EOF) {
        list_basket(s);
        return;
    }
 
    ungetc(c, stdin);
    scanf("%s", buf);

    if (is_ean_valid(buf)) strcpy(ean, buf);
    else {
        char *end;
        long val = strtol(buf, &end, 10);
        if (*end != '\0') { printf("invalid ean\n"); return; }
        qty = (int)val;
        scanf("%s", ean);
    }

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }

    Product *p = find_product(s, ean);
    if (!p) { printf("%s: no such product\n", ean); return; }

    BasketItem *b = find_basket_item(s, ean);

    if (qty > 0) {
        if (p->stock < qty) { printf("no stock\n"); return; }
        p->stock -= qty;
        if (b) b->quantity += qty;
        else {
            b = smalloc(sizeof(BasketItem));
            strcpy(b->ean, ean);
            b->quantity = qty;
            b->next = s->head_b;
            s->head_b = b;
        }
    } else if (qty < 0) {
        if (!b || b->quantity < -qty) { printf("invalid quantity\n"); return; }
        b->quantity += qty;
        p->stock -= qty;

        if (b->quantity == 0) {
            BasketItem *prev = NULL, *cur = s->head_b;
            while (cur != b) { prev = cur; cur = cur->next; }
            if (!prev) s->head_b = b->next;
            else prev->next = b->next;
            free(b);
            b = NULL;
        }
    }

    int total = b ? b->quantity : 0;

    printf("%c %.2f %d %.2f %s\n",
        p->iva_code,
        p->price / 100.0,
        total,
        calc_total_iva(p->price, total,
            s->taxas[p->iva_code - 'A']) / 100.0,
        p->description);
}

void cmd_f(Sistema *s) {
    char nome[MAX_LINE] = "Cliente final";
    char buf[MAX_LINE];
    long nif = 999999999;
    int items = 0;
    long total = 0;
    int c, quoted;

    /* saltar espaços */
    while ((c = getchar()) == ' ' || c == '\t');

    /* ===================== */
    /* 🔹 TEM ARGUMENTOS */
    /* ===================== */
    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);

        /* ler primeiro token */
        if (!read_name_or_token(buf, &quoted)) {
            printf("invalid name\n");
            return;
        }

        /* ===================== */
        /* 🔹 CASO: STRING ENTRE ASPAS → NOME */
        /* ===================== */
        if (quoted) {
            if (!valid_name(buf)) {
                printf("invalid name\n");
                return;
            }
            strcpy(nome, buf);
        }

        /* ===================== */
        /* 🔹 CASO: NIF VÁLIDO */
        /* ===================== */
        else if (valid_nif(buf)) {
            nif = atol(buf);

            while ((c = getchar()) == ' ' || c == '\t');

            if (c != '\n' && c != EOF) {
                ungetc(c, stdin);

                if (!read_name_or_token(nome, &quoted)) {
                    printf("invalid name\n");
                    return;
                }

                if (!valid_name(nome)) {
                    printf("invalid name\n");
                    return;
                }
            }
        }

        /* ===================== */
        /* 🔹 NÚMERO PURO MAS NÃO NIF */
        /* ===================== */
        else if (strspn(buf, "0123456789") == strlen(buf)) {
            printf("%s: no such nif\n", buf);
            return;
        }

        /* ===================== */
        /* 🔹 COMEÇA POR NÚMERO → INVÁLIDO */
        /* ===================== */
        else if (isdigit(buf[0])) {
            printf("invalid name\n");
            return;
        }

        /* ===================== */
        /* 🔹 NOME NORMAL */
        /* ===================== */
        else {
            if (!valid_name(buf)) {
                printf("invalid name\n");
                return;
            }

            strcpy(nome, buf);
        }
    }

    /* ===================== */
    /* 🔹 CRIAR FATURA */
    /* ===================== */
    calculate_totals(s, &items, &total);

    Invoice *nv = smalloc(sizeof(Invoice));
    nv->id = s->next_inv_id++;
    nv->nif = nif;
    nv->name = sstrdup(nome);
    nv->items_count = items;
    nv->total_cents = total;

    Invoice *curr = s->head_i, *prev = NULL;

    while (curr && (strcmp(curr->name, nome) < 0 ||
        (strcmp(curr->name, nome) == 0 && curr->id < nv->id))) {
        prev = curr;
        curr = curr->next;
    }

    nv->next = curr;

    if (!prev) s->head_i = nv;
    else prev->next = nv;

    printf("%d %.2f %d\n", items, total / 100.0, nv->id);

    clear_basket(s);
}

void cmd_l(Sistema *s) {
    char tok[MAX_LINE];
    int c;

    /* saltar espaços */
    while ((c = getchar()) == ' ' || c == '\t') {}

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

void cmd_d(Sistema *s) {
    char arg[MAX_LINE];
    int c;

    if (scanf("%s", arg) != 1)
        return;

    while ((c = getchar()) == ' ' || c == '\t') {}

    /* === CASO PRODUTO === */
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

   while ((c = getchar()) == ' ' || c == '\t') {}

    /* 🔹 SEM ARGUMENTOS */
    if (c == '\n' || c == EOF) {
        long total_items = 0;
        long total_valor = 0;

        /* ✅ soma apenas faturas existentes */
        for (Invoice *i = s->head_i; i; i = i->next) {
            total_items += i->items_count;
            total_valor += i->total_cents;
        }

        /* ✅ número TOTAL de faturas (mesmo apagadas) */
        long total_facturas = s->next_inv_id - 1;

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
    while ((c = getchar()) == ' ' || c == '\t') {}

    if (c == '\n' || c == EOF) {
        for (Invoice *i = s->head_i; i; i = i->next)
            printf("%d %.2f %s\n", i->id, i->total_cents/100.0, i->name);
    } else {
    int quoted;
    ungetc(c, stdin);
    read_name_or_token(nome, &quoted);
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