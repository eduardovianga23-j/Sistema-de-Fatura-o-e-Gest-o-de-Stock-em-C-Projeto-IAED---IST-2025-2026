/* iaed26 - ist1119719 - project */

/**
 * @file proj.c
 * @brief Sistema de faturação com gestão de produtos, cesto e faturas.
 * Implementa o ciclo principal do programa, leitura de comandos e
 * inicialização do sistema, incluindo taxas de IVA.
 * @author Eduardo João Vianga
 * @date Março 2025
 */
cd 
#include <stdio.h>
#include <locale.h>
#include <stdlib.h>

#include "structs.h"
#include "commands.h"

/**
 * @brief Liberta toda a memória alocada pelo sistema.
 * Percorre e liberta:
 * - Lista de produtos
 * - Lista de itens no cesto
 * - Lista de faturas
 * @param s Ponteiro para o sistema
 */
void free_all(Sistema *s) {
    /* Libertar lista de produtos */
    Product *p = s->head_p;
    while (p) {
        Product *t = p->next;
        free(p->description);
        free(p);
        p = t;
    }

    /* Libertar lista do cesto */
    BasketItem *b = s->head_b;
    while (b) {
        BasketItem *t = b->next;
        free(b);
        b = t;
    }

    /* Libertar lista de faturas */
    Invoice *i = s->head_i;
    while (i) {
        Invoice *t = i->next;
        free(i->name);
        free(i);
        i = t;
    }
}

/**
 * @brief Função principal do programa.
 * Inicializa o sistema, carrega taxas de IVA (de ficheiro ou default),
 * e processa comandos introduzidos pelo utilizador.
 * @param argc Número de argumentos da linha de comandos
 * @param argv Argumentos da linha de comandos
 * @return int Código de saída (0 em sucesso)
 */
int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    /**
     * @brief Estrutura principal do sistema
     *
     * Campos:
     * - head_p: lista de produtos
     * - head_b: lista do cesto
     * - count_p: número de produtos
     * - head_i: lista de faturas
     * - tail_i: última fatura
     * - next_invoice_id: próximo ID de fatura
     * - taxas: vetor de taxas de IVA (A-Z)
     */
    Sistema s = {NULL, NULL, 0, NULL, NULL, 1, {-1}};

    /* Inicializar todas as taxas como inválidas */
    for (int i = 0; i < 26; i++)
        s.taxas[i] = -1;

    /* Ler taxas de IVA a partir de ficheiro (se fornecido) */
    if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (f) {
            char ch;
            int v;

            /* Formato esperado: <letra> <valor> */
            while (fscanf(f, " %c %d", &ch, &v) == 2)
                s.taxas[ch - 'A'] = v;

            fclose(f);
        } 
    } else {
        /* Taxas por defeito */
        s.taxas[0] = 0;    /* A */
        s.taxas[1] = 6;    /* B */
        s.taxas[2] = 13;   /* C */
        s.taxas[3] = 23;   /* D */
    }

    /**
     * @brief Ciclo principal de leitura de comandos
     */
    char cmd;
    while (scanf(" %c", &cmd) == 1 && cmd != 'q') {
        if (cmd == 'p') cmd_p(&s);
        else if (cmd == 'l') cmd_l(&s);
        else if (cmd == 'a') cmd_a(&s);
        else if (cmd == 'r') cmd_r(&s);
        else if (cmd == 'f') cmd_f(&s);
        else if (cmd == 'c') cmd_c(&s);
        else if (cmd == 'd') cmd_d(&s);
    }

    /* Libertar memória antes de terminar */
    free_all(&s);

    return 0;
}