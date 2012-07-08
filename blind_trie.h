
#ifndef BLIND_TRIE_H
#define BLIND_TRIE_H

#include "sb_tree.h"

typedef struct {
    uint16_t g;
    uint64_t* Z;
    uint64_t* pos;
    uint64_t* suffixes;
    uint64_t* child_ptrs;
} blindtrie_compressed_t;

typedef struct {
    blindtrie_node_t* child[2];
} blindtrie_node_t;

typedef struct {
    blindtrie_node_t* root;
} blindtrie_uncompressed_t;

void                       blindtrie_from_dpage(sb_diskpage_t* page,blindtrie_compressed_t* bt);
blindtrie_uncompressed_t*  blindtrie_create(sbtree_t* sbt,uint64_t* sa,uint64_t n);
blindtrie_compressed_t*    blindtrie_compress(sbtree_t* sbt,blindtrie_uncompressed_t* btu);
void                       blindtrie_write(sbtree_t* sbt,blindtrie_compressed_t* bt,FILE* sbt_fd);

#endif