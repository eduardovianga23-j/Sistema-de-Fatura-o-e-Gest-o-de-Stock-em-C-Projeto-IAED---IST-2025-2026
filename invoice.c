#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"  
#include "invoice.h"
#include "utils.h"

/** @brief Insere fatura ordenada */
void insert_invoice_sorted(Sistema *s, Invoice *nv) {
    Invoice *curr = s->head_i, *prev = NULL;
    while (curr && (strcmp(curr->name, nv->name) < 0 ||
        (strcmp(curr->name, nv->name) == 0 && curr->id < nv->id))) {
        prev = curr; curr = curr->next;
    }
    nv->next = curr;
    if (!prev) s->head_i = nv;
    else prev->next = nv;
}

Invoice* create_invoice(Sistema *s, char *nome, long nif, int items, long total) {

    Invoice *nv = smalloc(sizeof(Invoice));

    nv->id = s->next_inv_id++;
    nv->nif = nif;
    nv->name = sstrdup(nome);
    nv->items_count = items;
    nv->total_cents = total;
    nv->next = NULL;

    insert_invoice_sorted(s, nv);

    return nv;
}

void print_invoice(Invoice *nv, int items, long total) {
    printf("%d %.2f %d\n",
        items,
        total / 100.0,
        nv->id);
}
