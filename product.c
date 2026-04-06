/**
 * @file product.c
 * @brief Funções relacionadas com a gestão de produtos.
 * Este módulo contém operações de pesquisa sobre os produtos
 * armazenados no sistema.
 * @author
 * Eduardo João Vianga
 */

#include <string.h>
#include "product.h"

/**
 * @brief Procura um produto pelo código EAN.
 * @param s Ponteiro para o sistema.
 * @param ean Código EAN do produto a procurar.
 * @return Ponteiro para o produto encontrado ou NULL se não existir.
 */
Product* find_product(Sistema *s, const char *ean) {
    for (Product *p = s->head_p; p; p = p->next)
        if (!strcmp(p->ean, ean)) return p;
    return NULL;
}