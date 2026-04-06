/**
 * @file utils.h
 * @brief Declaração de funções utilitárias do sistema de faturação.
 * Este módulo fornece funções auxiliares para:
 * - Gestão de memória
 * - Manipulação de strings
 * - Validação de dados (NIF, EAN, nomes, descrições)
 * - Processamento de input
 * - Cálculo de preços e IVA
 */

#ifndef UTILS_H
#define UTILS_H

#include "structs.h"

/**
 * @brief Aloca memória com verificação de erro.
 * @param size Número de bytes a alocar
 * @return Ponteiro para a memória alocada
 */
void* smalloc(size_t size);

/**
 * @brief Duplica uma string.
 * @param s String original
 * @return Nova string alocada dinamicamente
 */
char* sstrdup(const char *s);

/**
 * @brief Verifica correspondência com wildcard.
 * Suporta '?' (um carácter) e '*' (qualquer sequência).
 * @param p Padrão
 * @param s String a verificar
 * @return 1 se corresponder, 0 caso contrário
 */
int match_wild(const char *p, const char *s);

/**
 * @brief Lê um nome ou token da entrada padrão.
 * Permite leitura de strings entre aspas ou tokens simples.
 * @param buffer Buffer de destino
 * @param is_quoted Indica se o input estava entre aspas
 * @return 1 em sucesso, 0 em erro
 */
int read_name_or_token(char *buffer, int *is_quoted);

/**
 * @brief Valida nome de cliente.
 * @param name Nome
 * @return 1 se válido, 0 caso contrário
 */
int valid_name(const char *name);

/**
 * @brief Valida NIF.
 * @param s String com NIF
 * @return 1 se válido, 0 caso contrário
 */
int valid_nif(const char *s);

/**
 * @brief Valida código EAN.
 * @param e Código EAN
 * @return 1 se válido, 0 caso contrário
 */
int is_ean_valid(const char *e);

/**
 * @brief Valida descrição de produto.
 * @param d Descrição
 * @return 1 se válida, 0 caso contrário
 */
int valid_description(const char *d);

/**
 * @brief Calcula total com IVA incluído.
 * @param price Preço unitário (cêntimos)
 * @param qty Quantidade
 * @param tax Taxa de IVA (%)
 * @return Total em cêntimos
 */
long calc_total_iva(long price, int qty, int tax);

/**
 * @brief Converte string de preço para cêntimos.
 * @param preco String com preço (ex: "10.57")
 * @return Valor em cêntimos ou -1 em erro
 */
long long parse_price_to_cents(const char *preco);

#endif