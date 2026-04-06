/**
 * @file structs.h
 * @brief Definição das estruturas de dados do sistema de faturação.
 * Este ficheiro contém as estruturas principais utilizadas para gerir:
 * - Produtos
 * - Cesto de compras
 * - Faturas
 * - Estado global do sistema
 * @author Eduardo João Vianga
 * @date March 2025
 */

#ifndef STRUCTS_H
#define STRUCTS_H

#ifndef MAX_LINE
#define MAX_LINE 65536
#define MAX_PRODUCTS 10000
#endif

/**
 * @struct Product
 * @brief Representa um produto no sistema.
 * Cada produto contém informação identificativa, fiscal e de stock.
 */
typedef struct Product {
    char ean[14];          
    char *description;     
    long price;            
    char iva_code;         
    int stock;           
    int sold_qty;          
    int basket_qty;        
    struct Product *next;  
} Product;

/**
 * @struct BasketItem
 * @brief Representa um item no cesto de compras.
 * Cada item referencia um produto e a quantidade adicionada.
 */
typedef struct BasketItem {
    Product *product;      
    int quantity;         
    struct BasketItem *next; 
} BasketItem;

/**
 * @struct Invoice
 * @brief Representa uma fatura emitida.
 * Guarda informação do cliente e totais da compra.
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
 * @brief Estado global do sistema de faturação.
 * Centraliza todas as estruturas dinâmicas e dados do sistema.
 */
typedef struct {
    Product *head_p;       
    Product *tail_p;       
    int num_p;            
    BasketItem *head_b;   
    Invoice *head_i;       
    int next_inv_id;     
    int taxas[26];    
} Sistema;

/**
 * @struct CmdAInput
 * @brief Estrutura auxiliar para o comando 'a' (adicionar ao cesto).
 * Utilizada para armazenar os parâmetros lidos do input.
 */
typedef struct {
    char ean[MAX_LINE];    
    int qty;              
    int show_only;        
} CmdAInput;

#endif