#include <string.h>
#include "product.h"

Product* find_product(Sistema *s, const char *ean) {
    for (Product *p = s->head_p; p; p = p->next)
        if (!strcmp(p->ean, ean)) return p;
    return NULL;
}