#ifndef BASKET_H
#define BASKET_H

#include "structs.h"

BasketItem* find_basket_item(Sistema *s, const char *ean);

void insert_basket_sorted(Sistema *s, BasketItem *new_item);
void remove_basket_item(Sistema *s, BasketItem *b);

void list_basket(Sistema *s);
void clear_basket(Sistema *s);
void calculate_totals(Sistema *s, int *items, long *total);

#endif