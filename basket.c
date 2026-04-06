/**
 * @file basket.c
 * @brief Implementação das operações sobre o cesto de compras.
 * Este módulo contém funções para gestão do cesto:
 * - Pesquisa de itens
 * - Inserção ordenada
 * - Remoção de itens
 * - Listagem
 * - Limpeza
 * - Cálculo de totais
 * @author
 * Eduardo João Vianga
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basket.h"
#include "utils.h"



/**
 * @brief Procura um item no cesto pelo código EAN.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto.
 * @return Ponteiro para o item encontrado ou NULL se não existir.
 */
BasketItem* find_basket_item(Sistema *s, const char *ean) {
    for (BasketItem *b = s->head_b; b; b = b->next)
        if (!strcmp(b->product->ean, ean)) return b;
    return NULL;
}

/**
 * @brief Insere um item no cesto de forma ordenada pelo EAN.
 * @param s Ponteiro para o sistema.
 * @param new_item Novo item a inserir.
 */
void insert_basket_sorted(Sistema *s, BasketItem *new_item) {
    BasketItem *curr = s->head_b, *prev = NULL;
    while (curr && strcmp(curr->product->ean, new_item->product->ean) < 0) {
        prev = curr; 
        curr = curr->next;
    }
    new_item->next = curr;
    if (!prev) s->head_b = new_item;
    else prev->next = new_item;
}

/**
 * @brief Remove um item do cesto.
 * @param s Ponteiro para o sistema.
 * @param b Item a remover.
 */
void remove_basket_item(Sistema *s, BasketItem *b) {
    BasketItem *prev = NULL, *cur = s->head_b;
    while (cur && cur != b) { 
        prev = cur; 
        cur = cur->next; 
    }
    if (!prev) s->head_b = b->next;
    else prev->next = b->next;

    b->product->basket_qty = 0;
    free(b);
}

/**
 * @brief Lista todos os itens do cesto.
 * Mostra o código IVA, preço unitário, quantidade,
 * preço total com IVA e descrição do produto.
 * @param s Ponteiro para o sistema.
 */
void list_basket(Sistema *s) {
    for (BasketItem *b = s->head_b; b; b = b->next) {
        if (b->quantity <= 0) continue;

        Product *p = b->product;
        if (p) {
            printf("%c %.2f %d %.2f %s\n",
                p->iva_code,
                p->price / 100.0,
                b->quantity,
                calc_total_iva(p->price, b->quantity,
                    s->taxas[p->iva_code - 'A']) / 100.0,
                p->description);
        }
    }
}

/**
 * @brief Remove todos os itens do cesto.
 * Liberta a memória e repõe as quantidades no sistema.
 * @param s Ponteiro para o sistema.
 */
void clear_basket(Sistema *s) {
    while (s->head_b) {
        BasketItem *t = s->head_b;
        s->head_b = t->next;

        t->product->basket_qty = 0;
        free(t);
    }
}

/**
 * @brief Calcula o total de itens e o valor total com IVA.
 * Atualiza também a quantidade vendida de cada produto.
 * @param s Ponteiro para o sistema.
 * @param items Ponteiro para o total de itens.
 * @param total Ponteiro para o valor total (em cêntimos).
 */
void calculate_totals(Sistema *s, int *items, long *total) {
    for (BasketItem *b = s->head_b; b; b = b->next) {
        Product *p = b->product;
        if (p && b->quantity > 0) {
            p->sold_qty += b->quantity;
            *items += b->quantity;
            *total += calc_total_iva(p->price, b->quantity,
                s->taxas[p->iva_code - 'A']);
        }
    }
}