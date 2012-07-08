
#include "blind_trie.h"

/* maps a disk page to a blind trie struct */
void
blindtrie_from_dpage(sb_diskpage_t* page,blindtrie_t* bt)
{
    uint64_t* dptr = page->data;
    bt->g = page->data[0];
    bt->Z = &(page->data[1]);
    uint64_t zlen_bits = 2*bt->g - 2;
    uint64_t zlen_u64 = zlen_bits/64 + 1;
    bt->pos = &(page->data[1 + zlen_u64]);
}

/* given a list of suffixes, create a blind trie over them */
blindtrie_t*
blindtrie_create(sbtree_t* sbt,uint64_t* sa,uint64_t n)
{
    return NULL;
}

/* write a blind trie to the index file using at most B bytes */
void
blindtrie_write(sbtree_t* sbt,blindtrie_t* bt,FILE* sbt_fd)
{

}
