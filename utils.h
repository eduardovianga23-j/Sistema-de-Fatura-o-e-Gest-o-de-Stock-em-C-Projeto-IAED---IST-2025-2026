#ifndef UTILS_H
#define UTILS_H

#include "structs.h"

void* smalloc(size_t size);
char* sstrdup(const char *s);
int match_wild(const char *p, const char *s);

int read_name_or_token(char *buffer, int *is_quoted);

int valid_name(const char *name);
int valid_nif(const char *s);
int is_ean_valid(const char *e);
int valid_description(const char *d);

long calc_total_iva(long price, int qty, int tax);
long long convert_str_centimos(const char *preco);

#endif