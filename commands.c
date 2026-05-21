/**
 * @file commands.c
 * @brief Implementação dos comandos do sistema de faturação.
 * Este módulo processa todos os comandos introduzidos pelo utilizador:
 * - Gestão de produtos (p, l, d)
 * - Gestão do cesto (a, r)
 * - Faturação (f, c)
 * Inclui também funções auxiliares para parsing, validação e manipulação
 * de estruturas internas do sistema.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "structs.h"
#include "commands.h"
#include "utils.h"
#include "product.h"
#include "basket.h"
#include "invoice.h"

/**
 * @brief Faz parsing do comando 'a' (adicionar ao cesto).
 * Permite dois formatos:
 * - <ean>
 * - <qty> <ean>
 * Se não houver argumentos, ativa modo de visualização do cesto.
 * @param in Estrutura onde guardar os dados lidos
 * @return 1 em sucesso, 0 em erro
 */

 int has_invalid_desc_chars(char *desc) {
    for (int i = 0; desc[i]; i++) {
        if (desc[i] == '(' || desc[i] == ')' ||
            desc[i] == '[' || desc[i] == ']' ||
            desc[i] == '{' || desc[i] == '}') {
            return 1;
        }
    }
    return 0;
}

int parse_cmd_a(CmdAInput *in) {
    char buf[MAX_LINE];
    int c;

    in->qty = 1;
    in->show_only = 0;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '\n' || c == EOF) {
        in->show_only = 1;
        return 1;
    }

    ungetc(c, stdin);

    if (scanf("%s", buf) != 1) return 0;

    if (is_ean_valid(buf)) {
        strcpy(in->ean, buf);
    } else {
        char *end;
        long val = strtol(buf, &end, 10);

        if (*end != '\0') {
            printf("invalid ean\n");
            return 0;
        }

        in->qty = (int)val;

        if (scanf("%s", in->ean) != 1) return 0;
    }

    return 1;
}

/**
 * @brief Atualiza o cesto de compras.
 * - qty > 0 → adiciona produto ao cesto
 * - qty < 0 → remove produto do cesto
 * Garante consistência entre:
 * - stock do produto
 * - quantidade no cesto
 * @param s Sistema
 * @param p Produto
 * @param qty Quantidade a adicionar/remover
 * @return 1 em sucesso, 0 em erro
 */

int update_basket(Sistema *s, Product *p, int qty) {

    BasketItem *b = find_basket_item(s, p->ean);

    if (qty > 0) {
        if (p->stock < qty) {
            printf("no stock\n");
            return 0;
        }

        p->stock -= qty;
        p->basket_qty += qty;

        if (b) {
            b->quantity += qty;
        } else {
            b = smalloc(sizeof(BasketItem));
            b->product = p;
            b->quantity = qty;
            b->next = NULL;
            insert_basket_sorted(s, b);
        }

    } else if (qty < 0) {
        if (!b || b->quantity < -qty) {
            printf("invalid quantity\n");
            return 0;
        }

        b->quantity += qty;
        p->stock += -qty;
        p->basket_qty -= (-qty);

        if (b->quantity == 0)
            remove_basket_item(s, b);
    }

    return 1;
}

/**
 * @brief Processa token de cliente (nome ou NIF).
 * Determina se o input corresponde a:
 * - Nome (com ou sem aspas)
 * - NIF válido
 * Atualiza os valores de nome e NIF conforme necessário.
 * @param buf Token lido
 * @param quoted Indica se estava entre aspas
 * @param nome Nome do cliente (output)
 * @param nif NIF do cliente (output)
 * @return 1 em sucesso, 0 em erro
 */

int handle_client_token(char *buf, int quoted, char *nome, long *nif) {

    if (quoted) {
        strcpy(nome, buf);
        return 1;
    }

    if (valid_nif(buf)) {
        *nif = atol(buf);

        int c;
        while ((c = getchar()) == ' ' || c == '\t');

        if (c != '\n' && c != EOF) {
            ungetc(c, stdin);

            int q;
            if (!read_name_or_token(nome, &q) || !valid_name(nome)) {
                printf("invalid name\n");
                return 0;
            }
        }

        return 1;
    }

    if (strspn(buf, "0123456789") == strlen(buf)) {
        printf("%s: no such nif\n", buf);
        return 0;
    }

    if (!valid_name(buf)) {
        printf("invalid name\n");
        return 0;
    }

    strcpy(nome, buf);
    return 1;
}

/**
 * @brief Faz parsing da informação do cliente.
 * Define valores por defeito:
 * - Nome: "Cliente final"
 * - NIF: 999999999
 * Se houver input, processa nome ou NIF.
 * @param nome Nome do cliente (output)
 * @param nif NIF do cliente (output)
 * @return 1 em sucesso, 0 em erro
 */

int parse_client_info(char *nome, long *nif) {
    char buf[MAX_LINE];
    int c, quoted;

    strcpy(nome, "Cliente final");
    *nif = 999999999;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '\n' || c == EOF)
        return 1;

    ungetc(c, stdin);

    if (!read_name_or_token(buf, &quoted)) {
        printf("invalid name\n");
        return 0;
    }

    return handle_client_token(buf, quoted, nome, nif);
}

/**
 * @brief Remove quantidade de um produto do sistema.
 * - Valida EAN
 * - Verifica existência
 * - Garante que não está no cesto
 * - Remove produto se stock chegar a 0
 * @param s Sistema
 * @param ean Código EAN
 * @param qty Quantidade a remover
 */

void handle_product_removal(Sistema *s, char *ean, int qty) {

    if (!is_ean_valid(ean)) {
        printf("invalid ean\n");
        return;
    }

    Product *p = find_product(s, ean);

    if (!p) {
        printf("%s: no such product\n", ean);
        return;
    }

    if (p->basket_qty > 0) {
        printf("product in use\n");
        return;
    }

    if (qty <= 0 || qty > p->stock) {
        printf("invalid quantity\n");
        return;
    }

    p->stock -= qty;

    printf("%d %s\n", p->stock, p->description);

    // remover produto se stock = 0
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

/**
 * @brief Remove uma fatura pelo ID.
 * Imprime os dados da fatura antes de remover.
 * @param s Sistema
 * @param id Identificador da fatura
 */

void handle_invoice_removal(Sistema *s, int id) {

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

/**
 * @brief Imprime informação de um produto.
 * Inclui:
 * - EAN
 * - IVA
 * - Preço
 * - Quantidade total (vendido + cesto)
 * - Stock
 * - Descrição
 * @param p Produto
 */
void print_product_line(Product *p) {
    int qty = p->basket_qty;

    printf("%s %c %.2f %d %d %s\n",
        p->ean,
        p->iva_code,
        p->price / 100.0,
        p->sold_qty + qty,
        p->stock,
        p->description);
}

/**
 * @brief Lista todos os produtos com stock disponível.
 * Imprime todos os produtos cujo stock é maior que zero.
 * Caso não existam, imprime mensagem de erro.
 * @param s Sistema
 */

void list_all_products(Sistema *s) {
    int found_any = 0;

    for (Product *p = s->head_p; p; p = p->next) {
        if (p->stock > 0) {
            print_product_line(p);
            found_any = 1;
        }
    }

    if (!found_any)
        printf("*: no such product\n");
}

/**
 * @brief Lista produtos que correspondem a um padrão wildcard.
 * Utiliza correspondência com '*' e '?' sobre o EAN.
 * @param s Sistema
 * @param tok Padrão de pesquisa
 */

void list_products_pattern(Sistema *s, char *tok) {
    int found_any = 0;

    for (Product *p = s->head_p; p; p = p->next) {
        if (match_wild(tok, p->ean) && p->stock > 0) {
            print_product_line(p);
            found_any = 1;
        }
    }

    if (!found_any)
        printf("%s: no such product\n", tok);
}

/**
 * @brief Valida dados do comando 'a'.
 * - Verifica se o EAN é válido
 * - Verifica se o produto existe
 * @param s Sistema
 * @param in Input do comando
 * @param p Produto encontrado (output)
 * @return 1 em sucesso, 0 em erro
 */

int validate_cmd_a(Sistema *s, CmdAInput *in, Product **p) {

    if (!is_ean_valid(in->ean)) {
        printf("invalid ean\n");
        return 0;
    }

    *p = find_product(s, in->ean);

    if (!(*p)) {
        printf("%s: no such product\n", in->ean);
        return 0;
    }

    return 1;
}

/**
 * @brief Imprime resultado do comando 'a'.
 * Mostra:
 * - IVA
 * - Preço
 * - Quantidade no cesto
 * - Total com IVA
 * - Descrição
 * @param p Produto
 * @param s Sistema
 */

void print_cmd_a(Product *p, Sistema *s) {
    int total = p->basket_qty;

    printf("%c %.2f %d %.2f %s\n",
        p->iva_code,
        p->price / 100.0,
        total,
        calc_total_iva(p->price, total,
            s->taxas[p->iva_code - 'A']) / 100.0,
        p->description);
}


/* ================= COMANDOS ================= */

/**
 * @brief Comando 'a' - Adiciona ou remove produtos do cesto.
 * - Permite adicionar/remover quantidades
 * - Pode listar o cesto (sem argumentos)
 * - Atualiza stock e cesto de forma consistente
 * @param s Sistema
 */

void cmd_p(Sistema *s) {
    char ean[MAX_LINE], iva, desc[MAX_LINE], price_str[32];
    int qty;

    if (scanf("%s %c %s %d", ean, &iva, price_str, &qty) != 4) return;
    if (scanf(" %65535[^\n]", desc) != 1) desc[0] = '\0';

    long long price = parse_price_to_cents(price_str);

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }
    if (iva < 'A' || iva > 'Z' || s->taxas[iva - 'A'] == -1) { printf("invalid iva\n"); return; }
    if (price <= 0) { printf("invalid price\n"); return; }
    if (qty < 0) { printf("invalid quantity\n"); return; }
    if (has_invalid_desc_chars(desc)) {
    printf("invalid character in description\n"); //Nova alteração do teste prático
    return;
    }

    if (!valid_description(desc)) {
        printf("invalid description\n");
        return;
    }
    Product *p = find_product(s, ean);

    if (p) {
        if (find_basket_item(s, ean)) { printf("product in use\n"); return; }
        p->stock += qty;
        p->price = (long)price;
        p->iva_code = iva;
        free(p->description);
        p->description = sstrdup(desc);
    } else {
        if (s->num_p >= MAX_PRODUCTS) { printf("invalid product\n"); return; }

        p = smalloc(sizeof(Product));
        strcpy(p->ean, ean);
        p->description = sstrdup(desc);
        p->price = (long)price;
        p->iva_code = iva;
        p->stock = qty;
        p->sold_qty = 0;
        p->basket_qty = 0;
        p->next = NULL;

        if (!s->head_p) s->head_p = p;
        else s->tail_p->next = p;

        s->tail_p = p;
        s->num_p++;
    }

    printf("%d\n", p->stock);
}

/**
 * @brief Comando 'a' - Adiciona ou remove produtos do cesto.
 * - Permite adicionar/remover quantidades
 * - Pode listar o cesto (sem argumentos)
 * - Atualiza stock e cesto de forma consistente
 * @param s Sistema
 */

void cmd_a(Sistema *s) {
    CmdAInput in;
    Product *p;

    if (!parse_cmd_a(&in)) return;

    if (in.show_only) {
        list_basket(s);
        return;
    }

    if (!validate_cmd_a(s, &in, &p)) return;

    if (!update_basket(s, p, in.qty)) return;

    print_cmd_a(p, s);
}

/**
 * @brief Comando 'f' - Finaliza compra e gera fatura.
 * - Processa dados do cliente
 * - Calcula totais
 * - Cria fatura
 * - Limpa o cesto
 * @param s Sistema
 */

void cmd_f(Sistema *s) {
    char nome[MAX_LINE];
    long nif;
    int items = 0;
    long total = 0;

    if (!parse_client_info(nome, &nif))
        return;

    calculate_totals(s, &items, &total);

    Invoice *nv = create_invoice(s, nome, nif, items, total);

    print_invoice(nv, items, total);

    clear_basket(s);
}

/** @brief Comando l */
void cmd_l(Sistema *s) {
    char tok[MAX_LINE];
    int c;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '\n' || c == EOF) {
        list_all_products(s);
        return;
    }

    ungetc(c, stdin);

    while (scanf("%s", tok) == 1) {

        list_products_pattern(s, tok);

        while (isspace(c = getchar()) && c != '\n');

        if (c == '\n' || c == EOF)
            break;
        else
            ungetc(c, stdin);
    }
}

/** @brief Comando d */
void cmd_d(Sistema *s) {
    char arg[MAX_LINE];
    int c;

    if (scanf("%s", arg) != 1) return;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c != '\n' && c != EOF) {
        ungetc(c, stdin);

        int qty;
        if (scanf("%d", &qty) != 1) return;

        handle_product_removal(s, arg, qty);
    } else {
        handle_invoice_removal(s, atoi(arg));
    }
}

/**
 * @brief Comando 'r' - Consulta estatísticas ou produto.
 * - Sem argumentos: mostra resumo global do sistema
 * - Com EAN: mostra informação de um produto
 * @param s Sistema
 */
void cmd_r(Sistema *s) {
    char ean[MAX_LINE];
    int c;

    while ((c = getchar()) == ' ' || c == '\t') {}

    if (c == '\n' || c == EOF) {
        long total_items = 0;
        long total_valor = 0;

        for (Invoice *i = s->head_i; i; i = i->next) {
            total_items += i->items_count;
            total_valor += i->total_cents;
        }

        long total_facturas = s->next_inv_id - 1;

       printf("%ld %ld %.2f\n",
            total_items,
            total_facturas,
            total_valor / 100.0);

        for (int j = 0; j < 26; j++) {
            if (s->taxas[j] != -1)
                printf("%c %d%%\n", 'A' + j, s->taxas[j]);
        }

        return;
    }

    ungetc(c, stdin);

    if (scanf("%s", ean) != 1) return;

    if (!is_ean_valid(ean)) { printf("invalid ean\n"); return; }

    Product *p = find_product(s, ean);
    if (!p) { printf("%s: no such product\n", ean); return; }

    int total_vendido = p->sold_qty + p->basket_qty;

    printf("%d %d %s\n",
        p->stock,
        total_vendido,
        p->description);
}

/** @brief Comando c */
void cmd_c(Sistema *s) {
    char nome[MAX_LINE];
    int c, fnd = 0, quoted;

    while ((c = getchar()) == ' ' || c == '\t') {}

    if (c == '\n' || c == EOF) {
        for (Invoice *i = s->head_i; i; i = i->next)
            printf("%d %.2f %s\n", i->id, i->total_cents / 100.0, i->name);
        return;
    }

    ungetc(c, stdin);

    if (!read_name_or_token(nome, &quoted) || !valid_name(nome)) {
        printf("invalid name\n");
        return;
    }

    for (Invoice *i = s->head_i; i; i = i->next) {
        if (strncmp(i->name, nome, strlen(nome)) == 0) { // Nova alteração do teste prático
            printf("%d %.2f %s\n", i->id, i->total_cents / 100.0, i->name);
            fnd = 1;
        }
    }

    if (!fnd) printf("%s: no such client\n", nome);
}