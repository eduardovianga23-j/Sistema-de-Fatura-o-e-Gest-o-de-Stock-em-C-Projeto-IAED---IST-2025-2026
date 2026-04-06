#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "utils.h"

/** @brief Aloca memória com verificação */
void* smalloc(size_t size) {
    void *p = malloc(size);
    if (!p) { printf("No memory.\n"); exit(0); }
    return p;
}

/** @brief Duplica string */
char* sstrdup(const char *s) {
    char *d = smalloc(strlen(s) + 1);
    return strcpy(d, s);
}

/** @brief Wildcard match */
int match_wild(const char *p, const char *s) {
    const char *star = NULL, *ss = NULL;

    while (*s) {
        if (*p == '?' || *p == *s) { p++; s++; }
        else if (*p == '*') { star = p++; ss = s; }
        else if (star) { p = star + 1; s = ++ss; }
        else return 0;
    }

    while (*p == '*') p++;
    return *p == '\0';
}

/** @brief Lê nome ou token */
int read_name_or_token(char *buffer, int *is_quoted) {
    int c, i = 0;
    *is_quoted = 0;

    while ((c = getchar()) == ' ' || c == '\t');

    if (c == '"') {
        *is_quoted = 1;
        while ((c = getchar()) != '"' && c != '\n' && c != EOF)
            if (i < MAX_LINE - 1) buffer[i++] = (char)c;
        if (c != '"') return 0;
    } else {
        while (c != EOF && c != '\n' && c != ' ' && c != '\t') {
            if (i < MAX_LINE - 1) buffer[i++] = (char)c;
            c = getchar();
        }
        if (c != EOF) ungetc(c, stdin);
    }

    buffer[i] = '\0';
    return 1;
}

/** @brief Arredonda para cêntimos */
static long round_to_cents(double x) {
    double y = x * 100.0;
    return (long)(y + 0.5 + 1e-9);
}

/** @brief Valida nome */
int valid_name(const char *name) {
    if (!name || !isalpha((unsigned char)name[0])) return 0;
    for (int i = 0; name[i]; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalpha(c) || isdigit(c) || c == ' ')) return 0;
    }
    return 1;
}

/** @brief Valida NIF */
int valid_nif(const char *s) {
    if (strlen(s) != 9) return 0;
    for (int i = 0; i < 9; i++) if (!isdigit(s[i])) return 0;
    if (s[0] == '0') return 0;
    return 1;
}

/** @brief Valida EAN */
int is_ean_valid(const char *e) {
    int len = strlen(e), soma = 0;
    if (len != 8 && len != 13) return 0;
    for (int i = 0; i < len; i++) if (!isdigit(e[i])) return 0;
    for (int i = 0; i < len - 1; i++) {
        int d = e[i] - '0';
        soma += (i % 2 == 0) ? d : d * 3;
    }
    return ((10 - (soma % 10)) % 10) == (e[len - 1] - '0');
}

/** @brief Valida descrição */
int valid_description(const char *d) {
    int len = strlen(d);
    if (len == 0 || len > 50) return 0;
    unsigned char c = (unsigned char)d[0];
    if (!((c >= 'A' && c <= 'Z') || c >= 192)) return 0;
    for (int i = 0; i < len; i++) if ((unsigned char)d[i] < 32) return 0;
    return 1;
}

/** @brief Calcula total com IVA */
long calc_total_iva(long price, int qty, int tax) {
    double preco = price / 100.0;
    double total = preco * qty * (1 + tax / 100.0);
    return round_to_cents(total);
}

/** @brief Converte preço string para cêntimos */
long long convert_str_centimos(const char *preco) {
    if (!preco || preco[0] == '-') return -1;

    long long euros = 0, cents = 0;
    char *dot = strchr(preco, '.');

    if (!dot) return atoll(preco) * 100;

    euros = atoll(preco);

    char cent_str[10];
    strncpy(cent_str, dot + 1, 3);
    cent_str[3] = '\0';

    int len = strlen(cent_str);

    if (len == 1) cents = (cent_str[0] - '0') * 10;
    else if (len >= 2) {
        cents = (cent_str[0] - '0') * 10 + (cent_str[1] - '0');
        if (len >= 3 && cent_str[2] >= '5') cents++;
    }

    if (cents >= 100) { euros++; cents -= 100; }

    return euros * 100 + cents;
}