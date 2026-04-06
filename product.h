/**
 * @file product.h
 * @brief Funções para gestão de produtos no sistema de faturação.
 * Este módulo fornece operações básicas sobre produtos,
 * nomeadamente a pesquisa na lista de produtos.
 */

#ifndef PRODUCT_H
#define PRODUCT_H

#include "structs.h"

/**
 * @brief Procura um produto pelo código EAN.
 * Percorre a lista ligada de produtos e devolve o produto
 * correspondente ao EAN fornecido.
 * @param s Sistema
 * @param ean Código EAN do produto
 * @return Ponteiro para o produto encontrado ou NULL se não existir
 */
Product* find_product(Sistema *s, const char *ean);

#endif