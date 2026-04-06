#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basket.h"
#include "utils.h"

BasketItem* find_basket_item(Sistema *s, const char *ean) {
    for (BasketItem *b = s->head_b; b; b = b->next)
        if (!strcmp(b->product->ean, ean)) return b;
    return NULL;
}

/** @brief Insere item ordenado no cesto */
void insert_basket_sorted(Sistema *s, BasketItem *new_item) {
    BasketItem *curr = s->head_b, *prev = NULL;
    while (curr && strcmp(curr->product->ean, new_item->product->ean) < 0) {
        prev = curr; curr = curr->next;
    }
    new_item->next = curr;
    if (!prev) s->head_b = new_item;
    else prev->next = new_item;
}

/** @brief Remove item do cesto */
void remove_basket_item(Sistema *s, BasketItem *b) {
    BasketItem *prev = NULL, *cur = s->head_b;
    while (cur && cur != b) { prev = cur; cur = cur->next; }
    if (!prev) s->head_b = b->next;
    else prev->next = b->next;
    b->product->basket_qty = 0;
    free(b);
}

/** @brief Lista cesto */
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

/** @brief Limpa cesto */
void clear_basket(Sistema *s) {
    while (s->head_b) {
        BasketItem *t = s->head_b;
        s->head_b = t->next;
        t->product->basket_qty = 0;
        free(t);
    }
}

/** @brief Calcula totais */
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