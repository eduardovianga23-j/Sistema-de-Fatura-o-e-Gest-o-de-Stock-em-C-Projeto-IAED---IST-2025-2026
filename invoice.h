/**
 * @file invoice.h
 * @brief Funções para gestão de faturas no sistema de faturação.
 * Este módulo permite:
 * - Criar faturas
 * - Inserir faturas numa lista ordenada
 * - Imprimir informação de faturas
 */

#ifndef INVOICE_H
#define INVOICE_H

#include "structs.h"

/**
 * @brief Insere uma fatura na lista ordenada do sistema.
 * A lista de faturas é mantida ordenada (tipicamente por ID crescente).
 * @param s Sistema
 * @param nv Nova fatura a inserir
 */
void insert_invoice_sorted(Sistema *s, Invoice *nv);

/**
 * @brief Cria uma nova fatura.
 * Inicializa os campos da fatura com os dados fornecidos.
 * @param s Sistema (para obter ID da fatura)
 * @param nome Nome do cliente
 * @param nif NIF do cliente
 * @param items Número de itens
 * @param total Total em cêntimos
 * @return Ponteiro para a nova fatura
 */
Invoice* create_invoice(Sistema *s, char *nome, long nif, int items, long total);

/**
 * @brief Imprime os dados de uma fatura.
 * @param nv Fatura a imprimir
 * @param items Número de itens
 * @param total Total em cêntimos
 */
void print_invoice(Invoice *nv, int items, long total);

#endif