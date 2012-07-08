
#ifndef CRITBIT_TREE_H
#define CRITBIT_TREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CRITBIT_LEFTCHILD	       0
#define CRITBIT_RIGHTCHILD	       1
#define CRITBIT_ISLEAF(x)          ((((uint64_t)x)&(0x8000000000000000))>>63)
#define CRITBIT_SETSUFFIX(x)       ((critbit_node_t*)(((uint64_t)x)|0x8000000000000000))
#define CRITBIT_GETSUFFIX(x)       (((uint64_t)x)&~0x8000000000000000)
#define CRITBIT_GETBYTEPOS(x)      (x>>3)
#define CRITBIT_GETBITPOS(x)       ((x&7))
#define CRITBIT_GETDIRECTION(x,y)  ((x&(1<<(7-y)))>>(7-y))
#define CRITBIT_GETCRITBITPOS(x,y) (__builtin_clz(x^y) - ((sizeof(unsigned int) - sizeof(uint8_t))<<3))

typedef struct critbit_node {
    uint64_t crit_bit_pos;
    struct critbit_node* child[2];
} critbit_node_t;

typedef struct {
    critbit_node_t* root;
} critbit_tree_t;

critbit_tree_t* critbit_create();
void            critbit_free(critbit_tree_t* cbt);
void			critbit_clear(critbit_tree_t* cbt);
void            critbit_insert_suffix(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,uint64_t suffixpos);
uint64_t        critbit_delete_suffix(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,uint64_t suffixpos);
uint64_t        critbit_contains(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,const uint8_t* P,uint64_t m);
uint64_t		critbit_suffixes(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,const uint8_t* P,uint64_t m,uint64_t** results);
void			critbit_print(critbit_tree_t* cbt);
void			critbit_print_tex(critbit_tree_t* cbt);

/* helper functions */
void 			critbit_delete_nodes(critbit_node_t* node);
void			critbit_print_node(critbit_node_t* node);
void			critbit_print_tex_node(critbit_node_t* node);
void            critbit_collectsuffixes(critbit_node_t* node,uint64_t** results,uint64_t* nresults,uint64_t* res_size);
void            critbit_addresult(uint64_t** results,uint64_t* nresults,uint64_t* res_size,uint64_t suffix);
int             critbit_intcmp(const void* a,const void* b);

#endif