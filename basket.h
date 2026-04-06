/**
 * @file basket.h
 * @brief Funções para gestão do cesto de compras.
 * Este módulo permite:
 * - Procurar itens no cesto
 * - Inserir e remover itens
 * - Listar conteúdo do cesto
 * - Limpar o cesto
 * - Calcular totais da compra
 */

#ifndef BASKET_H
#define BASKET_H

#include "structs.h"

/**
 * @brief Procura um item no cesto pelo EAN.
 * @param s Sistema
 * @param ean Código EAN do produto
 * @return Ponteiro para o item ou NULL se não existir
 */
BasketItem* find_basket_item(Sistema *s, const char *ean);

/**
 * @brief Insere um item no cesto de forma ordenada.
 * A ordenação pode ser feita por EAN ou outro critério definido
 * na implementação.
 * @param s Sistema
 * @param new_item Novo item a inserir
 */
void insert_basket_sorted(Sistema *s, BasketItem *new_item);

/**
 * @brief Remove um item do cesto.
 * Liberta a memória associada ao item.
 * @param s Sistema
 * @param b Item a remover
 */
void remove_basket_item(Sistema *s, BasketItem *b);

/**
 * @brief Lista todos os itens do cesto.
 * Mostra produtos, quantidades e totais.
 * @param s Sistema
 */
void list_basket(Sistema *s);

/**
 * @brief Limpa completamente o cesto.
 * Remove todos os itens e liberta memória.
 * @param s Sistema
 */
void clear_basket(Sistema *s);

/**
 * @brief Calcula totais do cesto.
 * Calcula:
 * - Número total de itens
 * - Valor total com IVA
 * @param s Sistema
 * @param items Número total de itens (output)
 * @param total Valor total em cêntimos (output)
 */
void calculate_totals(Sistema *s, int *items, long *total);

#endif