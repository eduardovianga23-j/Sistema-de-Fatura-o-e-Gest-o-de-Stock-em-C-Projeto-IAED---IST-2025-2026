/**
 * @file commands.h
 * @brief Declaração dos comandos do sistema de faturação.
 * Este ficheiro contém os protótipos das funções responsáveis
 * por processar os comandos introduzidos pelo utilizador.
 * @author
 * Eduardo João Vianga
 */

#ifndef COMMANDS_H
#define COMMANDS_H

#include "structs.h"

/**
 * @brief Comando para adicionar um novo produto ao sistema.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_p(Sistema *s);

/**
 * @brief Comando para adicionar produtos ao cesto.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_a(Sistema *s);

/**
 * @brief Comando para finalizar a compra e gerar fatura.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_f(Sistema *s);

/**
 * @brief Comando para listar produtos existentes no sistema.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_l(Sistema *s);

/**
 * @brief Comando para remover um produto do sistema.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_d(Sistema *s);

/**
 * @brief Comando para remover produtos do cesto.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_r(Sistema *s);

/**
 * @brief Comando para limpar o cesto de compras.
 * @param s Ponteiro para a estrutura do sistema.
 */
void cmd_c(Sistema *s);

#endif