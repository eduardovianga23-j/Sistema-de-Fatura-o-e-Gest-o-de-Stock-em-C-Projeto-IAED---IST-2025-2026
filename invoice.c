/**
 * @file invoice.c
 * @brief Implementação das funções de gestão de faturas.
 * Este módulo trata da criação, inserção ordenada e impressão
 * de faturas no sistema de faturação.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structs.h"
#include "invoice.h"
#include "utils.h"

/**
 * @brief Insere uma fatura na lista ligada de forma ordenada.
 * A ordenação é feita por:
 * 1. Nome do cliente (ordem lexicográfica)
 * 2. ID da fatura (em caso de empate no nome)
 * @param s Sistema
 * @param nv Nova fatura a inserir
 */

void insert_invoice_sorted(Sistema *s, Invoice *nv) {
    Invoice *curr = s->head_i, *prev = NULL;

    while (curr &&
        (strcmp(curr->name, nv->name) < 0 ||
        (strcmp(curr->name, nv->name) == 0 && curr->id < nv->id))) {

        prev = curr;
        curr = curr->next;
    }

    nv->next = curr;

    if (!prev)
        s->head_i = nv;
    else
        prev->next = nv;
}

/**
 * @brief Cria e inicializa uma nova fatura.
 * - Atribui um ID único
 * - Copia o nome do cliente
 * - Define totais e número de itens
 * - Insere automaticamente na lista ordenada
 * @param s Sistema (usado para gerar ID e inserir)
 * @param nome Nome do cliente
 * @param nif NIF do cliente
 * @param items Número de itens
 * @param total Total em cêntimos
 * @return Ponteiro para a fatura criada
 */

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

/**
 * @brief Imprime um resumo de uma fatura.
 * Formato de saída:
 * <items> <total_em_euros> <id>
 * @param nv Fatura
 * @param items Número de itens
 * @param total Total em cêntimos
 */
void print_invoice(Invoice *nv, int items, long total) {
    printf("%d %.2f %d\n",
        items,
        total / 100.0,
        nv->id);
}