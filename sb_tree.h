
#ifndef SBTREE_H
#define SBTREE_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define SBT_ROOT_OFFSET		4096

#include "sb_tmpfile.h"

/* node in the SB-tree. size = B bytes */
typedef struct {
    uint64_t data[0];
} sb_diskpage_t;

/* the main sbtree struct */
typedef struct {
    uint64_t B;                 /* page size */
    uint64_t b;                 /* each node in the tree contains b <= x <= 2b suffixes. */
    uint64_t n;                 /* # of suffixes or size of the input text */
    uint64_t height;            /* height of the SB-tree */
    uint64_t bits_per_suffix;   /* bits used per suffix = log2(n) */
    uint64_t bits_per_pos;      /* max lcp -> determines the max size of the pos array entries in the blind trie */
    int fd;                     /* open file descriptor of the index */
    int textfd;                 /* open file descriptor to the text */
    sb_diskpage_t* root;        /* root node stays in main memory. */
} sbtree_t;

/* disk layout description of the index file:

	0-4095         : [B][b][n][empty space]
	4096-B+4096    : root disk page (B bytes)
	followed by    : [rest of the SB-tree internal pages]
	followed by    : [suffix array pages]

	therefore: root page always at file offset 4096.
*/

/* load/save/create functions */
sbtree_t* sbtree_create(const char* text_file,const char* outfile,uint64_t B);
sbtree_t* sbtree_build(const char* sa_file,const char* text_file,const char* outfile,uint64_t maxlcp,uint64_t B);
sbtree_t* sbtree_load(const char* sb_file,const char* text_file);
void      sbtree_printstats(const sbtree_t* sbt);
void      sbtree_free(sbtree_t* sbt);
void      sbtree_createtree(sbtree_t* sbt,sbtmpfile_t* suffixes,const uint8_t* T,uint64_t n,FILE* sbt_fd);

/* query functions */
uint64_t*   sbtree_search(const sbtree_t* sbt,const uint8_t* P,uint64_t m,uint64_t* nres);

/* helper functions */
uint64_t        sbtree_calc_height(const sbtree_t* sbt);
uint64_t	    sbtree_calc_branch_factor(sbtree_t* sbt);
sb_diskpage_t*  sbtree_load_diskpage(const sbtree_t* sbt,uint64_t offset);
void            sbtree_free_diskpage(const sbtree_t* sbt,sb_diskpage_t* sbd);
void            sbtree_writeheader(sbtree_t* sbt,FILE* out);
void            sbtree_readheader(sbtree_t* sbt,FILE* in);
void            sbtree_addpadding(FILE* out,uint64_t bytes);

#endif