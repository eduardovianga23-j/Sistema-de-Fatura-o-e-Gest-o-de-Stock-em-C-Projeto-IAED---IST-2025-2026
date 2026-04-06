/**
 * @file utils.c
 * @brief Funções utilitárias de apoio ao sistema de faturação.
 * Este módulo inclui funções auxiliares para:
 * - Gestão de memória
 * - Manipulação de strings
 * - Validação de dados (NIF, EAN, nomes, descrições)
 * - Processamento de input
 * - Cálculos de preços e IVA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

/**
 * @brief Aloca memória dinamicamente com verificação de erro.
 * Termina o programa caso a alocação falhe.
 * @param size Número de bytes a alocar
 * @return Ponteiro para a memória alocada
 */
void* smalloc(size_t size) {
    void *p = malloc(size);
    if (!p) {
        printf("No memory.\n");
        exit(0);
    }
    return p;
}

/**
 * @brief Duplica uma string.
 * Aloca memória suficiente e copia o conteúdo da string original.
 * @param s String original
 * @return Nova string duplicada
 */
char* sstrdup(const char *s) {
    char *d = smalloc(strlen(s) + 1);
    return strcpy(d, s);
}

/**
 * @brief Compara string com padrão wildcard.
 * Suporta:
 * - '?' : corresponde a um único carácter
 * - '*' : corresponde a qualquer sequência de caracteres
 * @param p Padrão (pattern)
 * @param s String a comparar
 * @return 1 se corresponder, 0 caso contrário
 */
int match_wild(const char *p, const char *s) {
    const char *star = NULL, *ss = NULL;

    while (*s) {
        if (*p == '?' || *p == *s) {
            p++; s++;
        }
        else if (*p == '*') {
            star = p++;
            ss = s;
        }
        else if (star) {
            p = star + 1;
            s = ++ss;
        }
        else return 0;
    }

    while (*p == '*') p++;
    return *p == '\0';
}

/**
 * @brief Lê um nome ou token da entrada padrão.
 * Permite:
 * - Strings entre aspas (ex: "João Silva")
 * - Tokens simples (sem espaços)
 * @param buffer Buffer onde será armazenado o texto
 * @param is_quoted Indica se o input estava entre aspas
 * @return 1 em sucesso, 0 em erro
 */
int read_name_or_token(char *buffer, int *is_quoted) {
    int c, i = 0;
    *is_quoted = 0;

    /* Ignorar espaços iniciais */
    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '"') {
        *is_quoted = 1;

        /* Ler até fechar aspas */
        while ((c = getchar()) != '"' && c != '\n' && c != EOF)
            if (i < MAX_LINE - 1)
                buffer[i++] = (char)c;

        if (c != '"') return 0;
    } else {
        /* Ler token simples */
        while (c != EOF && c != '\n' && c != ' ' && c != '\t') {
            if (i < MAX_LINE - 1)
                buffer[i++] = (char)c;
            c = getchar();
        }
        if (c != EOF) ungetc(c, stdin);
    }

    buffer[i] = '\0';
    return 1;
}

/**
 * @brief Arredonda um valor decimal para cêntimos.
 * @param x Valor em euros
 * @return Valor arredondado em cêntimos
 */
static long round_to_cents(double x) {
    double y = x * 100.0;
    return (long)(y + 0.5 + 1e-9);
}

/**
 * @brief Valida um nome de cliente.
 * Regras:
 * - Deve começar por letra
 * - Pode conter letras, dígitos e espaços
 * @param name Nome a validar
 * @return 1 se válido, 0 caso contrário
 */
int valid_name(const char *name) {
    if (!name || !isalpha((unsigned char)name[0])) return 0;

    for (int i = 0; name[i]; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalpha(c) || isdigit(c) || c == ' '))
            return 0;
    }
    return 1;
}

/**
 * @brief Valida um NIF.
 * Regras:
 * - Deve ter exatamente 9 dígitos
 * - Não pode começar por '0'
 * @param s String com o NIF
 * @return 1 se válido, 0 caso contrário
 */
int valid_nif(const char *s) {
    if (strlen(s) != 9) return 0;

    for (int i = 0; i < 9; i++)
        if (!isdigit(s[i])) return 0;

    if (s[0] == '0') return 0;

    return 1;
}

/**
 * @brief Valida um código EAN (8 ou 13 dígitos).
 * Verifica:
 * - Comprimento válido (8 ou 13)
 * - Apenas dígitos
 * - Dígito de controlo (checksum)
 * @param e Código EAN
 * @return 1 se válido, 0 caso contrário
 */
int is_ean_valid(const char *e) {
    int len = strlen(e), soma = 0;

    if (len != 8 && len != 13) return 0;

    for (int i = 0; i < len; i++)
        if (!isdigit(e[i])) return 0;

    /* Cálculo do checksum */
    for (int i = 0; i < len - 1; i++) {
        int d = e[i] - '0';
        soma += (i % 2 == 0) ? d : d * 3;
    }

    return ((10 - (soma % 10)) % 10) == (e[len - 1] - '0');
}

/**
 * @brief Valida descrição de produto.
 * Regras:
 * - Comprimento entre 1 e 50 caracteres
 * - Primeiro carácter maiúsculo ou UTF-8 válido
 * - Sem caracteres de controlo
 * @param d Descrição
 * @return 1 se válida, 0 caso contrário
 */
int valid_description(const char *d) {
    int len = strlen(d);

    if (len == 0 || len > 50) return 0;

    unsigned char c = (unsigned char)d[0];
    if (!((c >= 'A' && c <= 'Z') || c >= 192)) return 0;

    for (int i = 0; i < len; i++)
        if ((unsigned char)d[i] < 32) return 0;

    return 1;
}

/**
 * @brief Calcula o total com IVA incluído.
 * @param price Preço unitário em cêntimos
 * @param qty Quantidade
 * @param tax Taxa de IVA (%)
 * @return Total em cêntimos
 */
long calc_total_iva(long price, int qty, int tax) {
    double preco = price / 100.0;
    double total = preco * qty * (1 + tax / 100.0);
    return round_to_cents(total);
}

/**
 * @brief Converte string de preço para cêntimos.
 * Aceita formatos como:
 * - "10"
 * - "10.5"
 * - "10.57"
 * - "10.578" (arredonda)
 *
 * @param price_str String com preço
 * @return Valor em cêntimos ou -1 em erro
 */
long long parse_price_to_cents(const char *price_str) {
    if (!price_str || price_str[0] == '-') return -1;

    long long euros = 0, cents = 0;
    char *dot_pos = strchr(price_str, '.');

    if (!dot_pos)
        return atoll(price_str) * 100;

    euros = atoll(price_str);

    char cents_str[10];
    strncpy(cents_str, dot_pos + 1, 3);
    cents_str[3] = '\0';

    int len = strlen(cents_str);

    if (len == 1) {
        cents = (cents_str[0] - '0') * 10;
    }
    else if (len >= 2) {
        cents = (cents_str[0] - '0') * 10 +
                (cents_str[1] - '0');

        if (len >= 3 && cents_str[2] >= '5')
            cents++;
    }

    if (cents >= 100) {
        euros++;
        cents -= 100;
    }

    return euros * 100 + cents;
}