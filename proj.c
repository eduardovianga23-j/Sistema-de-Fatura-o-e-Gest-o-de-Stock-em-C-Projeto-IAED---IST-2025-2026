/* iaed26 - ist1119719 - project */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* --- Consts --- */
#define MAX_PRODUCTS 10000
#define MAX_DESC 50
#define MAX_LINE 65535

typedef struct Product {
    char *description;
    char ean[14];
    char iva_code;
    long price; /* Armazenado em cêntimos (ex: 705 para 7.05) */
    int stock;
    int sold_qty;
    struct Product *next;
} Product;

typedef struct {
    int taxas[26];
} TabelaIVA;

/* --- Memory Management --- */

void free_all_products(Product *head) {
    Product *curr, *next;
    curr = head;
    while (curr) {
        next = curr->next;
        if (curr->description)
            free(curr->description);
        free(curr);
        curr = next;
    }
}

void fatal_no_memory(Product *head) {
    printf("No memory.\n");
    free_all_products(head);
    exit(0);
}

/* --- Input Reading --- */

int read_word(char *buffer) {
    int c, i = 0;
    while ((c = getchar()) != EOF && (c == ' ' || c == '\t'));
    if (c == EOF || c == '\n') return c;
    buffer[i++] = (char)c;
    while ((c = getchar()) != EOF && c != ' ' && c != '\t' && c != '\n') {
        if (i < MAX_LINE - 1) buffer[i++] = (char)c;
    }
    buffer[i] = '\0';
    return c;
}

/* --- Aux Functions --- */

int is_ean_valid(const char *ean) {
    int len = (int)strlen(ean);
    int soma = 0, i, check;

    if (len != 8 && len != 13) return 0;

    for (i = 0; i < len; i++) {
        if (!isdigit((unsigned char)ean[i])) return 0;
        if (i < len - 1) {
            int d = ean[i] - '0';
            if (len == 13)
                /* EAN-13: posições 0, 2, 4... mult 1; posições 1, 3, 5... mult 3 */
                soma += (i % 2 == 0) ? d : d * 3;
            else
                /* EAN-8: posições 0, 2, 4, 6 mult 3; posições 1, 3, 5 mult 1 */
                soma += (i % 2 == 0) ? d * 3 : d;
        }
    }
    check = (10 - (soma % 10)) % 10;
    return (ean[len - 1] - '0') == check;
}

int match_wildcard(const char *pat, const char *str) {
    if (*pat == '\0' && *str == '\0') return 1;
    if (*pat == '*')
        return match_wildcard(pat + 1, str) ||
               (*str != '\0' && match_wildcard(pat, str + 1));
    if (*pat == '?' && *str != '\0')
        return match_wildcard(pat + 1, str + 1);
    return (*pat == *str) && match_wildcard(pat + 1, str + 1);
}

/* --- Commands --- */

void command_q(Product *head) {
    free_all_products(head);
    exit(0);
}

void command_p(Product **head, int *total_count, TabelaIVA *iva_tab) {
    char ean[MAX_LINE], buffer[MAX_LINE], desc[MAX_LINE];
    double price_input;
    int qty, iva_idx, i = 0, c;
    char iva_code;
    Product *curr, *temp;

    read_word(ean);
    read_word(buffer);
    iva_code = buffer[0];

    /* Leitura do Preço: Lemos como double e convertemos para long (cêntimos)
       usando +0.5 para arredondamento simétrico na leitura binária */
    read_word(buffer);
    price_input = atof(buffer);
    long price_total = (long)(price_input * 100 + 0.5);

    read_word(buffer);
    qty = atoi(buffer);

    while ((c = getchar()) == ' ' || c == '\t');
    while (c != '\n' && c != EOF) {
        if (i < MAX_LINE - 1) desc[i++] = (char)c;
        c = getchar();
    }
    desc[i] = '\0';

    /* Validações conforme o enunciado */
    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }
    iva_idx = iva_code - 'A';
    if (iva_idx < 0 || iva_idx > 25 || iva_tab->taxas[iva_idx] == -1) { 
        printf("invalid iva\n"); return; 
    }
    if (price_total <= 0) { printf("invalid price\n"); return; }
    if (qty < 0) { printf("invalid quantity\n"); return; }
    if (i > MAX_DESC) { printf("invalid description\n"); return; }

    curr = *head;
    while (curr) {
        if (strcmp(curr->ean, ean) == 0) {
            curr->stock += qty;
            curr->price = price_total;
            curr->iva_code = iva_code;
            free(curr->description);
            curr->description = malloc(i + 1);
            if (!curr->description) fatal_no_memory(*head);
            strcpy(curr->description, desc);
            printf("%d\n", curr->stock);
            return;
        }
        curr = curr->next;
    }

    if (*total_count >= MAX_PRODUCTS) { printf("invalid product\n"); return; }

    curr = malloc(sizeof(Product));
    if (!curr) fatal_no_memory(*head);
    curr->description = malloc(i + 1);
    if (!curr->description) { free(curr); fatal_no_memory(*head); }

    strcpy(curr->ean, ean);
    strcpy(curr->description, desc);
    curr->iva_code = iva_code;
    curr->price = price_total;
    curr->stock = qty;
    curr->sold_qty = 0;
    curr->next = NULL;

    if (*head == NULL) *head = curr;
    else {
        temp = *head;
        while (temp->next) temp = temp->next;
        temp->next = curr;
    }
    (*total_count)++;
    printf("%d\n", curr->stock);
}

void command_l(Product *head) {
    char wildcard[MAX_LINE];
    int c, found = 0, i = 0;
    Product *p;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '\n' || c == EOF) {
        for (p = head; p; p = p->next) {
            if (p->stock > 0) {
                printf("%s %c %ld.%02ld %d %d %s\n",
                       p->ean, p->iva_code, p->price / 100, p->price % 100,
                       p->sold_qty, p->stock, p->description);
            }
        }
        return;
    }

    while (c != ' ' && c != '\t' && c != '\n' && c != EOF) {
        wildcard[i++] = (char)c;
        c = getchar();
    }
    wildcard[i] = '\0';

    for (p = head; p; p = p->next) {
        if (p->stock > 0 && (strcmp(wildcard, "*") == 0 || match_wildcard(wildcard, p->ean))) {
            printf("%s %c %ld.%02ld %d %d %s\n",
                   p->ean, p->iva_code, p->price / 100, p->price % 100,
                   p->sold_qty, p->stock, p->description);
            found = 1;
        }
    }
    if (!found && strcmp(wildcard, "*") != 0)
        printf("%s: no such product\n", wildcard);

    if (c != '\n' && c != EOF)
        while ((c = getchar()) != '\n' && c != EOF);
}

/* --- Main --- */

int main(int argc, char *argv[]) {
    TabelaIVA iva_tab;
    Product *head = NULL;
    int total = 0, i, c;

    for (i = 0; i < 26; i++) iva_tab.taxas[i] = -1;
    iva_tab.taxas['A'-'A'] = 0; iva_tab.taxas['B'-'A'] = 6;
    iva_tab.taxas['C'-'A'] = 13; iva_tab.taxas['D'-'A'] = 23;

    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (f) {
            char code; int val;
            while (fscanf(f, " %c %d", &code, &val) == 2)
                if (code >= 'A' && code <= 'Z') iva_tab.taxas[code - 'A'] = val;
            fclose(f);
        }
    }

    while ((c = getchar()) != EOF) {
        if (isspace(c)) continue;
        if (c == 'q') command_q(head);
        else if (c == 'p') command_p(&head, &total, &iva_tab);
        else if (c == 'l') command_l(head);
        else while ((c = getchar()) != '\n' && c != EOF);
    }

    free_all_products(head);
    return 0;
}