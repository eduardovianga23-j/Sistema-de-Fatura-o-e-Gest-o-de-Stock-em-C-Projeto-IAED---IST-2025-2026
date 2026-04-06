#ifndef INVOICE_H
#define INVOICE_H

#include "structs.h"

void insert_invoice_sorted(Sistema *s, Invoice *nv);
Invoice* create_invoice(Sistema *s, char *nome, long nif, int items, long total);
void print_invoice(Invoice *nv, int items, long total);

#endif