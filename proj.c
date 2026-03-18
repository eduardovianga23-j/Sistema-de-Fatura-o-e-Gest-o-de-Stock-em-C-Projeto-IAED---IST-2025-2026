/**
 * @file proj.c
 * @brief 
 * Billing system in C for managing products, carts, and invoices.
 * Create/update products with valid EAN, price, VAT, and stock levels.
 * Validates EAN codes and applies VAT rates (external file or fallback values). 
 * Supports a shopping cart with item addition/removal and stock control.
 * Issues invoices with TIN, name, total including VAT, and cent rounding.
 * List and remove products or invoices while maintaining data integrity.
 * Command-based input system using dynamic memory without global variables.
 * @author Eduardo João Vianga,IST 1111179
 * @date March 18, 2025
 */

/* iaed26 - ist1119719 - project */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_PRODUCTS 10000
#define MAX_LINE 65536
 
/* --- Estruturas de Dados --- */

typedef struct Product {
    char ean[14];
    char *description;
    long price;
    char iva_code;
    int stock;
    int sold_qty;
    struct Product *next;
} Product;

typedef struct BasketItem {
    char ean[14];
    int quantity;
    struct BasketItem *next;
} BasketItem;

typedef struct Invoice {
    int id;
    long nif;
    char *name;
    int items_count;
    long total_cents;
    struct Invoice *next;
} Invoice;

typedef struct {
    Product *head_p, *tail_p;
    int num_p;
    BasketItem *head_b;
    Invoice *head_i;
    int next_inv_id;
    int taxas[26];
} Sistema;

/* --- Utilitários de Memória e Strings --- */

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

/* --- Validações e Cálculos --- */

int is_ean_valid(const char *e) {
    int len = (int)strlen(e);
    int soma = 0;

    if (len != 8 && len != 13) return 0;

    // Verifica que todos os dígitos são números
    for (int i = 0; i < len; i++) {
        if (!isdigit(e[i])) return 0;
    }

    // Soma dos dígitos com os pesos corretos
    for (int i = 0; i < len - 1; i++) {
        int d = e[i] - '0';
        if (i % 2 == 0)
            soma += d;      // posição par -> peso 1
        else
            soma += d * 3;  // posição ímpar -> peso 3
    }

    int check = (10 - (soma % 10)) % 10;

    return (e[len - 1] - '0') == check;
}

long calc_total_iva(long price, int qty, int tax_percent) {
    double total = (price * (double)qty) * (1.0 + tax_percent / 100.0);
    return (long)(total + 0.5); /* Arredondamento simétrico */
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
    Product *p;

    /* 1. Leitura dos campos numéricos e EAN */
    /* O espaço antes de %c ajuda a ignorar brancos pendentes */
    if (scanf("%s %c %lf %d", ean, &iva_c, &pr, &qty) != 4) return;

    /* 2. Leitura da descrição (o resto da linha) */
    /* " %[^\n]" ignora o espaço após a quantidade e lê até ao fim da linha */
    if (scanf(" %99[^\n]", desc) != 1) { 
     desc[0] = '\0';
    }

    /* --- Validações --- */

    if (!is_ean_valid(ean)) {
        printf("invalid ean\n");
        return;
    }

    if (iva_c < 'A' || iva_c > 'Z' || s->taxas[iva_c - 'A'] == -1) {
        printf("invalid iva\n");
        return;
    }

    if (pr <= 0) {
        printf("invalid price\n");
        return;
    }

    if (qty < 0) {
        printf("invalid quantity\n");
        return;
    }

    /* Descrição: máx 50 caracteres e deve começar por letra maiúscula */
    if (strlen(desc) > 50 || !isupper((unsigned char)desc[0])) {
        printf("invalid description\n");
        return;
    }

    /* 3. Verificação de Preço: Produto em uso no cesto */
    long novo_preco_cents = (long)(pr * 100 + 0.5);
    
    for (BasketItem *b = s->head_b; b; b = b->next) {
        if (strcmp(b->ean, ean) == 0 && b->quantity > 0) {
            /* Procurar o produto atual para comparar o preço */
            p = s->head_p;
            while (p && strcmp(p->ean, ean) != 0) p = p->next;
            
            if (p && novo_preco_cents != p->price) {
                printf("product in use\n");
                return;
            }
        }
    }

    /* 4. Procurar produto no sistema */
    p = s->head_p;
    while (p && strcmp(p->ean, ean) != 0) p = p->next;

    if (p) {
        /* Produto existe: atualizar stock e campos permitidos */
        p->stock += qty;
        p->price = novo_preco_cents;
        p->iva_code = iva_c;
        free(p->description);
        p->description = sstrdup(desc);
    } else {
        /* Novo produto: verificar limite */
        if (s->num_p >= MAX_PRODUCTS) {
            printf("invalid product\n");
            return;
        }

        p = smalloc(sizeof(Product));
        strcpy(p->ean, ean);
        p->description = sstrdup(desc);
        p->price = novo_preco_cents;
        p->iva_code = iva_c;
        p->stock = qty;
        p->sold_qty = 0;
        p->next = NULL;

        if (!s->head_p) {
            s->head_p = p;
        } else {
            s->tail_p->next = p;
        }
        s->tail_p = p;
        s->num_p++;
    }

    /* 5. Output do stock final */
    printf("%d\n", p->stock);
}

void cmd_l(Sistema *s) {
    char tok[MAX_LINE];
    int c, found_any = 0;
    while (isspace(c = getchar()) && c != '\n');
    
    if (c == '\n' || c == EOF) {
        /* l sem argumentos: lista todos com stock > 0. */
        for (Product *p = s->head_p; p; p = p->next) {
            if (p->stock > 0) {
                // --- INÍCIO DA LÓGICA DA SOMA ---
                int qty_no_cesto = 0;
                for (BasketItem *b = s->head_b; b; b = b->next) {
                    if (strcmp(b->ean, p->ean) == 0) {
                        qty_no_cesto = b->quantity;
                        break;
                    }
                }
                // Imprimimos p->sold_qty + qty_no_cesto
                printf("%s %c %.2f %d %d %s\n", p->ean, p->iva_code, 
                       p->price/100.0, p->sold_qty + qty_no_cesto, p->stock, p->description);
                // --- FIM DA LÓGICA DA SOMA ---
                found_any = 1;
            }
        }
        if (!found_any) {
            printf("*: no such product\n");
        }
    } else {
        /* l com argumentos (ex: l * ou l 560*): */
        ungetc(c, stdin);
        while (scanf("%s", tok) == 1) {
            found_any = 0;
            for (Product *p = s->head_p; p; p = p->next) {
                if (match_wild(tok, p->ean) && p->stock > 0) {
                    // --- INÍCIO DA LÓGICA DA SOMA ---
                    int qty_no_cesto = 0;
                    for (BasketItem *b = s->head_b; b; b = b->next) {
                        if (strcmp(b->ean, p->ean) == 0) {
                            qty_no_cesto = b->quantity;
                            break;
                        }
                    }
                    // Imprimimos a soma aqui também
                    printf("%s %c %.2f %d %d %s\n", p->ean, p->iva_code, 
                           p->price/100.0, p->sold_qty + qty_no_cesto, p->stock, p->description);
                    // --- FIM DA LÓGICA DA SOMA ---
                    found_any = 1;
                }
            }
            if (!found_any) {
                printf("%s: no such product\n", tok);
            }
            
            while (isspace(c = getchar()) && c != '\n');
            if (c == '\n' || c == EOF) break; else ungetc(c, stdin);
        }
    }
}

void cmd_a(Sistema *s) {
    char buf[MAX_LINE], ean[MAX_LINE]; 
    int qty = 1, c;
    
    /* Pula espaços iniciais */
    while (isspace(c = getchar()) && c != '\n');
    
    /* Caso 1: Comando 'a' sem argumentos - Listar cesto */
    if (c == '\n' || c == EOF) {
        for (BasketItem *b = s->head_b; b; b = b->next) {
            if (b->quantity <= 0) continue;
            Product *p = s->head_p; 
            while (p && strcmp(p->ean, b->ean)) p = p->next;
            if (p) {
                printf("%c %.2f %d %.2f %s\n", p->iva_code, p->price/100.0, 
                       b->quantity, 
                       calc_total_iva(p->price, b->quantity, s->taxas[p->iva_code-'A'])/100.0, 
                       p->description);
            }
        } 
        return;
    }

    /* Caso 2: Comando 'a' com argumentos - Identificar o que é qty e o que é EAN */
    ungetc(c, stdin); 
    if (scanf("%s", buf) != 1) return;

    /* AQUI ESTAVA O ERRO DO TESTE 20: 
       Se buf for um EAN válido, a qty é 1. Caso contrário, buf é a qty. */
    if (is_ean_valid(buf)) {
        qty = 1;
        strcpy(ean, buf);
    } else {
        qty = atoi(buf);
        if (scanf("%s", ean) != 1) return;
    }

    /* Validações */
    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }
    
    Product *p = s->head_p; 
    while (p && strcmp(p->ean, ean)) p = p->next;
    if (!p) { printf("%s: no such product\n", ean); return; }

    /* Procura no cesto (lista ordenada por EAN) */
    BasketItem *curr = s->head_b, *prev = NULL;
    while (curr && strcmp(curr->ean, ean) < 0) { 
        prev = curr; 
        curr = curr->next; 
    }

    /* Verificação de erros de stock e quantidade */
    if (qty < 0 && (!curr || strcmp(curr->ean, ean) || curr->quantity < -qty)) { 
        printf("invalid quantity\n"); 
        return; 
    }
    if (qty > 0 && p->stock < qty) { 
        printf("no stock\n"); 
        return; 
    }

    /* Atualização */
    p->stock -= qty;
    if (curr && !strcmp(curr->ean, ean)) {
        curr->quantity += qty;
        /* Se a quantidade chegar a zero, removemos o nó para manter a lista limpa */
        if (curr->quantity == 0) {
            if (!prev) s->head_b = curr->next;
            else prev->next = curr->next;
            
            // Guardamos a info para o printf antes de libertar
            printf("%c %.2f %d %.2f %s\n", p->iva_code, p->price/100.0, 0, 0.0, p->description);
            free(curr);
            return;
        }
    } else {
        /* Novo item no cesto */
        BasketItem *new_b = smalloc(sizeof(BasketItem));
        strcpy(new_b->ean, ean); 
        new_b->quantity = qty; 
        new_b->next = curr;
        if (!prev) s->head_b = new_b; 
        else prev->next = new_b;
        curr = new_b;
    }

    /* Output final */
    printf("%c %.2f %d %.2f %s\n", p->iva_code, p->price/100.0, curr->quantity, 
           calc_total_iva(p->price, curr->quantity, s->taxas[p->iva_code-'A'])/100.0, 
           p->description);
}

void cmd_f(Sistema *s) {
    char line[MAX_LINE], nome[MAX_LINE] = "Cliente final";
    long nif = 999999999, total = 0;
    int items = 0, c;

    // Ignorar espaços iniciais
    while (isspace(c = getchar()) && c != '\n');

    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);
        read_name_or_token(line);

        // Validação robusta do NIF: deve ter 9 dígitos
        int valid_nif = 1;
        if (strlen(line) == 9) {
            for (int i = 0; i < 9; i++)
                if (!isdigit(line[i])) valid_nif = 0;

            if (valid_nif) {
                nif = atol(line);       // Converte para long
                read_name_or_token(nome); // Lê nome do cliente
            } else {
                strcpy(nome, line);     // Não é NIF, é nome
            }
        } else {
            strcpy(nome, line);         // Não tem 9 caracteres, é nome
        }
    }

    // Se nome for "error", limpar cesto e sair
    if (!strcmp(nome, "error")) {
        while (s->head_b) {
            BasketItem *t = s->head_b;
            Product *p = s->head_p;
            while (p && strcmp(p->ean, t->ean)) p = p->next;
            if (p) p->stock += t->quantity;
            s->head_b = t->next;
            free(t);
        }
        return;
    }

    // Calcular total e atualizar stock vendido
    for (BasketItem *b = s->head_b; b; b = b->next) {
        Product *p = s->head_p;
        while (p && strcmp(p->ean, b->ean)) p = p->next;
        if (p && b->quantity > 0) {
            p->sold_qty += b->quantity;
            items += b->quantity;
            total += calc_total_iva(p->price, b->quantity, s->taxas[p->iva_code-'A']);
        }
    }

    // Criar nova fatura
    Invoice *nv = smalloc(sizeof(Invoice));
    Invoice *curr = s->head_i, *prev = NULL;
    nv->id = s->next_inv_id++;
    nv->nif = nif;
    nv->name = sstrdup(nome);
    nv->items_count = items;
    nv->total_cents = total;

    // Inserir fatura ordenadamente
    while (curr && (strcmp(curr->name, nome) < 0 || 
           (strcmp(curr->name, nome) == 0 && curr->id < nv->id))) {
        prev = curr;
        curr = curr->next;
    }
    nv->next = curr;
    if (!prev) s->head_i = nv;
    else prev->next = nv;

    printf("%d %.2f %d\n", items, total / 100.0, nv->id);

    // Limpar cesto
    while (s->head_b) {
        BasketItem *t = s->head_b;
        s->head_b = t->next;
        free(t);
    }
}

void cmd_d(Sistema *s) {
    char arg[MAX_LINE]; int c;
    scanf("%s", arg);
    while ((c = getchar()) == ' ' || c == '\t');
    if (c != '\n' && c != EOF) {
        int qty; scanf("%d", &qty);
        if (!is_ean_valid(arg)) { printf("invalid ean\n"); return; }
        Product *p = s->head_p, *prev = NULL;
        while (p && strcmp(p->ean, arg)) { prev = p; p = p->next; }
        if (!p) { printf("%s: no such product\n", arg); return; }
        for (BasketItem *b = s->head_b; b; b = b->next) if (!strcmp(b->ean, arg) && b->quantity > 0) { printf("product in use\n"); return; }
        if (qty <= 0 || qty > p->stock) { printf("invalid quantity\n"); return; }
        p->stock -= qty; printf("%d %s\n", p->stock, p->description);
        if (p->stock == 0) {
            if (!prev) s->head_p = p->next; else prev->next = p->next;
            if (s->tail_p == p) s->tail_p = prev;
            free(p->description); free(p); s->num_p--;
        }
    } else {
        int id = atoi(arg); Invoice *i = s->head_i, *p_i = NULL;
        while (i && i->id != id) { p_i = i; i = i->next; }
        if (!i) { printf("%d: no such invoice\n", id); return; }
        printf("%.2f %ld %s\n", i->total_cents/100.0, i->nif, i->name);
        if (!p_i) s->head_i = i->next; else p_i->next = i->next;
        free(i->name); free(i);
    }
}

void cmd_r(Sistema *s) {
    char ean[MAX_LINE]; 
    int c;

    // Pula espaços iniciais
    while (isspace(c = getchar()) && c != '\n');

    if (c == '\n' || c == EOF) {
        /* Caso r sem argumentos: Relatório Geral */
        long it = 0, fcs = 0, tot = 0;
        for (Invoice *i = s->head_i; i; i = i->next) { 
            it += i->items_count; 
            fcs++; 
            tot += i->total_cents; 
        }
        printf("%ld %ld %.2f\n", it, fcs, tot/100.0);
        for (int j = 0; j < 26; j++) {
            if (s->taxas[j] != -1) 
                printf("%c %d%%\n", 'A' + j, s->taxas[j]);
        }
    } else {
        /* Caso r com EAN: Relatório de Produto Específico */
        ungetc(c, stdin);
        if (scanf("%s", ean) != 1) return;

        if (!is_ean_valid(ean)) { 
            printf("invalid ean\n"); 
            return; 
        }

        Product *p = s->head_p; 
        while (p && strcmp(p->ean, ean)) p = p->next;

        if (!p) { 
            printf("%s: no such product\n", ean); 
            return; 
        }

        // --- LÓGICA DE VENDAS (Faturado + No Cesto) ---
        int total_vendas = p->sold_qty;
        for (BasketItem *b = s->head_b; b; b = b->next) {
            if (strcmp(b->ean, p->ean) == 0) {
                total_vendas += b->quantity;
            }
        }
        
        printf("%d %d %s\n", p->stock, total_vendas, p->description);
    }
}

void cmd_c(Sistema *s) {
    char nome[MAX_LINE]; int c, fnd = 0;
    while (isspace(c = getchar()) && c != '\n');
    if (c == '\n' || c == EOF) {
        for (Invoice *i = s->head_i; i; i = i->next) printf("%d %.2f %s\n", i->id, i->total_cents/100.0, i->name);
    } else {
        ungetc(c, stdin); read_name_or_token(nome);
        for (Invoice *i = s->head_i; i; i = i->next)
            if (!strcmp(i->name, nome)) { printf("%d %.2f %s\n", i->id, i->total_cents/100.0, i->name); fnd = 1; }
        if (!fnd) printf("%s: no such client\n", nome);
    }
}

int main(int argc, char *argv[]) {
    Sistema s = {NULL, NULL, 0, NULL, NULL, 1, {-1}};
    for (int i=0; i<26; i++) s.taxas[i] = -1;
    s.taxas[0]=0; s.taxas[1]=6; s.taxas[2]=13; s.taxas[3]=23;
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (f) { char ch; int v; while (fscanf(f, " %c %d", &ch, &v) == 2) s.taxas[ch-'A'] = v; fclose(f); }
    }
    char cmd;
    while (scanf(" %c", &cmd) == 1 && cmd != 'q') {
        if (cmd == 'p') cmd_p(&s); else if (cmd == 'l') cmd_l(&s);
        else if (cmd == 'a') cmd_a(&s); else if (cmd == 'r') cmd_r(&s);
        else if (cmd == 'f') cmd_f(&s); else if (cmd == 'c') cmd_c(&s);
        else if (cmd == 'd') cmd_d(&s);
    }
    free_all(&s); return 0;
}