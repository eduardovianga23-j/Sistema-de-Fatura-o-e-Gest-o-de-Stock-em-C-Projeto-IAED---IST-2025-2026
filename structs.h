#ifndef STRUCTS_H
#define STRUCTS_H

/**
 * @file structs.h
 * @brief Header file containing the data structures used in the billing system.
 * This module defines the core structures for managing products, the shopping
 * basket, invoices, and the overall system state. It is included by other
 * source files to share common type definitions.
 * @author Eduardo João Vianga, ist1119719
 * @date March 20, 2025
 */

/**
 * @struct Product
 * @brief Represents a product in the billing system.
 */

typedef struct Product {
    char ean[14];
    char *description;
    long price;
    char iva_code;
    int stock;
    int sold_qty;
    int in_basket;  
    struct Product *next;
} Product;

/**
 * @struct BasketItem
 * @brief Represents an item in the shopping basket.
 */

typedef struct BasketItem {
    char ean[14];
    int quantity;
    struct BasketItem *next;
} BasketItem;

/**
 * @struct Invoice
 * @brief Represents an invoice in the billing system.
 */

typedef struct Invoice {
    int id;
    long nif;
    char *name;
    int items_count;
    long total_cents;
    struct Invoice *next;
} Invoice;

/**
 * @struct Sistema
 * @brief Represents the complete state of the billing system.
 *
 * Contains:
 * - A linked list of products (with head and tail pointers)
 * - The total number of products
 * - A linked list representing the current shopping basket
 * - A linked list of issued invoices
 * - The next available invoice ID
 * - An array of VAT rates indexed by letters (A–Z)
 */

typedef struct {
    Product *head_p, *tail_p;
    int num_p;
    BasketItem *head_b;
    Invoice *head_i;
    int next_inv_id;
    int taxas[26];
} Sistema;

#endif 
