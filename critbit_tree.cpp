
#include "critbit_tree.h"

/* create a new critbit tree */
critbit_tree_t*
critbit_create()
{
    critbit_tree_t* cbt = (critbit_tree_t*) malloc(sizeof(critbit_tree_t));
    if (!cbt) {
        fprintf(stderr, "error mallocing critbit tree memory.\n");
        exit(EXIT_FAILURE);
    }
    cbt->root = NULL;
    cbt->g = 0;
    return cbt;
}

/* clears all data from the critbit tree */
void
critbit_clear(critbit_tree_t* cbt)
{
    /* post order traverse */
    if (cbt->root) critbit_delete_nodes(cbt->root);
    cbt->root = NULL;
}

/* clears all data from the critbit tree and deletes the tree */
void
critbit_free(critbit_tree_t* cbt)
{
    /* post order traverse */
    if (cbt->root) critbit_delete_nodes(cbt->root);
    free(cbt);
}

void
critbit_delete_nodes(critbit_node_t* node)
{
    if (! CRITBIT_ISLEAF(node)) {
        critbit_delete_nodes(node->child[CRITBIT_LEFTCHILD]);
        critbit_delete_nodes(node->child[CRITBIT_RIGHTCHILD]);
        free(node);
    }
}

uint64_t
critbit_getsize_in_bytes(critbit_tree_t* cbt)
{

}

/*
   insert the suffix at position (suffixpos) into the critbit tree.
   we store the data directly in the pointers to the leaf nodes.
 */
void
critbit_insert_suffix(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,uint64_t suffixpos)
{
    if (!cbt->root) {
        /* store suffix in the root ptr */
        cbt->root =  CRITBIT_SETSUFFIX(suffixpos);
        cbt->g++; /* one more node */
        return;
    }

    critbit_node_t* cur_node = cbt->root;
    uint8_t direction;
    while (! CRITBIT_ISLEAF(cur_node)) {
        /* traverse till we find a leaf */
        uint64_t byte_pos = CRITBIT_GETBYTEPOS(cur_node->crit_bit_pos);
        uint8_t bit_pos_in_byte = CRITBIT_GETBITPOS(cur_node->crit_bit_pos);
        uint8_t sym = 0;
        /* use the crit bit pos to decide where to go */
        if (suffixpos + byte_pos < n) sym = T[suffixpos+byte_pos];
        /* the bit at the crit bit position decides where we go */
        direction = CRITBIT_GETDIRECTION(sym,bit_pos_in_byte);
        cur_node = cur_node->child[direction];
    }

    /* we are at a leaf. compare the suffixes and find the cirt bit */
    uint64_t critbit_pos = 0;
    uint8_t newdirection = 0;
    uint64_t i = 0;
    uint64_t k = suffixpos;
    uint64_t j = CRITBIT_GETSUFFIX(cur_node);

    /* check if the new suffix is already contained */
    if (j == suffixpos) {
        return;
    }

    /* compare suffixes till we find the crit bit pos */
    while (k+i < n && j+i < n) {
        if (T[k+i] != T[j+i]) {
            critbit_pos = CRITBIT_GETCRITBITPOS(T[k+i],T[j+i]);
            /* remember if the bit is 0 or 1 for the direction later */
            newdirection = CRITBIT_GETDIRECTION(T[k+i],critbit_pos);
            break;
        }
        i++;
    }

    /* create the new node */
    critbit_node_t* cbn = (critbit_node_t*) malloc(sizeof(critbit_node_t));
    if (!cbn) {
        fprintf(stderr, "error mallocing critbit node memory.\n");
        exit(EXIT_FAILURE);
    }
    /* bit pos = bytepos*8 + bit pos in the byte */
    cbn->crit_bit_pos = (i<<3) + critbit_pos;
    /* store the data again in the ptr */
    cbn->child[newdirection] = CRITBIT_SETSUFFIX(suffixpos);

    /* now go through the tree again to find the correct position to insert.
       we have to do this to make sure the lexicographical ordering is correct */
    cur_node = cbt->root;
    critbit_node_t* parent = NULL;
    while (! CRITBIT_ISLEAF(cur_node)) {
        /* traverse till we find a leaf */
        uint64_t byte_pos = CRITBIT_GETBYTEPOS(cur_node->crit_bit_pos);
        uint8_t bit_pos_in_byte = CRITBIT_GETBITPOS(cur_node->crit_bit_pos);
        uint8_t sym = 0;
        /* do we stop and insert because the current node is too far down? */
        if (cur_node->crit_bit_pos > cbn->crit_bit_pos) break;
        /* use the crit bit pos to decide where to go */
        if (suffixpos + byte_pos < n) sym = T[suffixpos+byte_pos];
        /* the bit at the crit bit position decides where we go */
        direction = CRITBIT_GETDIRECTION(sym,bit_pos_in_byte);
        parent = cur_node;
        cur_node = cur_node->child[direction];
    }

    /* insert the new node into the tree */
    if (parent) {
        parent->child[direction] = cbn;
    } else {
        cbt->root = cbn;
    }
    cbn->child[1 - newdirection] = cur_node;
    cbt->g++; /* one more node */
}

/* delete suffix corresponding to position (suffixpos) from the critbit tree

   returns 0 on success and 1 on error.
*/
uint64_t
critbit_delete_suffix(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,uint64_t suffixpos)
{
    if (!cbt->root) return 1;

    critbit_node_t* cur_node = cbt->root;
    critbit_node_t* parent = NULL;
    critbit_node_t* grandparent = NULL;
    uint8_t direction; /* direction parent -> current node */
    uint8_t gp_direction; /* direction grand parent -> parent */
    while (! CRITBIT_ISLEAF(cur_node)) {
        /* traverse till we find a leaf */
        uint64_t byte_pos = CRITBIT_GETBYTEPOS(cur_node->crit_bit_pos);
        uint8_t bit_pos_in_byte = CRITBIT_GETBITPOS(cur_node->crit_bit_pos);
        uint8_t sym = 0;
        /* use the crit bit pos to decide where to go */
        if (suffixpos + byte_pos < n) sym = T[suffixpos+byte_pos];
        /* the bit at the crit bit position decides where we go */
        gp_direction = direction;
        direction = CRITBIT_GETDIRECTION(sym,bit_pos_in_byte);
        grandparent = parent;
        parent = cur_node;
        cur_node = cur_node->child[direction];
    }

    /* we are at a leaf. check that the suffix position matches */
    if (CRITBIT_GETSUFFIX(cur_node) != suffixpos) return 1;

    if (!parent) {
        /* we are deleting the root */
        cbt->root = NULL;
    } else {
        if (!grandparent) {
            /* we are a direct child of the root. our sibling becomes the new root */
            cbt->root = parent->child[1 - direction];
        } else {
            /* our siblings replaces our parent in the grandparent */
            grandparent->child[gp_direction] = parent->child[1 - direction];
        }
        free(parent);
    }

    /* one less node */
    cbt->g--;

    return 0;
}

/* returns 1 if P is a prefix of a suffix stored in the critbit tree. 0 otherwise */
uint64_t
critbit_contains(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,const uint8_t* P,uint64_t m)
{
    if (!cbt->root) return 0;

    critbit_node_t* cur_node = cbt->root;
    uint8_t direction; /* direction parent -> current node */
    while (! CRITBIT_ISLEAF(cur_node)) {
        /* traverse till we find a leaf */
        uint64_t byte_pos = CRITBIT_GETBYTEPOS(cur_node->crit_bit_pos);
        uint8_t bit_pos_in_byte = CRITBIT_GETBITPOS(cur_node->crit_bit_pos);

        uint8_t sym = 0;
        /* use the crit bit pos to decide where to go */
        if (byte_pos < m) {
            /* if we are at the end already, we just go to the left-most child -> sym = 0 direction = 0 */
            sym = P[byte_pos];
        }
        /* the bit at the crit bit position decides where we go */
        direction = CRITBIT_GETDIRECTION(sym,bit_pos_in_byte);
        cur_node = cur_node->child[direction];
    }

    /* we are at a leaf. check if the prefix matches P up to m symbols. */
    uint64_t suffixpos = CRITBIT_GETSUFFIX(cur_node);
    uint64_t i;
    for (i = 0; i < m && suffixpos+i < n; i++) {
        if (T[suffixpos+i] != P[i]) return 0;
    }
    if (i==m) return 1;
    return 0;
}

/* sorts suffix integers */
int
critbit_intcmp(const void* a,const void* b)
{
    int64_t ua = *((int64_t*)a);
    int64_t ub = *((int64_t*)b);
    return ua - ub;
}

/* returns all suffixes matching the prefix P of length m. */
uint64_t
critbit_suffixes(critbit_tree_t* cbt,const uint8_t* T,uint64_t n,const uint8_t* P,uint64_t m,uint64_t** results)
{
    if (!cbt->root) return 0;

    critbit_node_t* cur_node = cbt->root;
    critbit_node_t* locus = NULL;
    uint8_t direction; /* direction parent -> current node */
    while (! CRITBIT_ISLEAF(cur_node)) {
        /* traverse till we find a leaf */
        uint64_t byte_pos = CRITBIT_GETBYTEPOS(cur_node->crit_bit_pos);
        uint8_t bit_pos_in_byte = CRITBIT_GETBITPOS(cur_node->crit_bit_pos);

        uint8_t sym = 0;
        /* use the crit bit pos to decide where to go */
        if (byte_pos < m) {
            /* if we are at the end already, we just go to the left-most child -> sym = 0 direction = 0 */
            sym = P[byte_pos];
        }
        /* the bit at the crit bit position decides where we go */
        direction = CRITBIT_GETDIRECTION(sym,bit_pos_in_byte);
        cur_node = cur_node->child[direction];

        if (byte_pos < m) {
            /* as long as we are within the prefix len we keep track of the node.
               if we later match the prefix we go back to the locus
               (the highest node in the tree that matches the prefix) and traverse all the children */
            locus = cur_node;
        }

    }

    /* we are at a leaf. check if the prefix matches P up to m symbols. */
    uint64_t suffixpos = CRITBIT_GETSUFFIX(cur_node);
    uint64_t i;
    for (i = 0; i < m && suffixpos+i < n; i++) {
        if (T[suffixpos+i] != P[i]) return 0;
    }
    if (i==m) {
        /* the prefix matched. now traverse all the children of the locus */
        uint64_t nresults = 0;
        uint64_t res_size = 512;
        *results = (uint64_t*) malloc(res_size*sizeof(uint64_t));
        if (*results == NULL) {
            fprintf(stderr, "error mallocing critbit result set memory.\n");
            exit(EXIT_FAILURE);
        }

        /* make sure the locus is not a leaf first */
        if (CRITBIT_ISLEAF(locus)) {
            critbit_addresult(results,&nresults,&res_size,CRITBIT_GETSUFFIX(locus));
            return nresults;
        } else {
            /* traverse the leafs of the locus sub tree */
            critbit_collectsuffixes(locus,results,&nresults,&res_size);
        }
        *results = (uint64_t*) realloc(*results,nresults*sizeof(uint64_t));
        if (*results == NULL) {
            fprintf(stderr, "error reallocing critbit result set memory.\n");
            exit(EXIT_FAILURE);
        }
        /* sort results as they are stored in arbitrary order */
        qsort(*results,nresults,sizeof(uint64_t),critbit_intcmp);
        return nresults;
    }
    return 0;
}

void
critbit_collectsuffixes(critbit_node_t* node,uint64_t** results,uint64_t* nresults,uint64_t* res_size)
{
    if (CRITBIT_ISLEAF(node->child[CRITBIT_LEFTCHILD]))
        critbit_addresult(results,nresults,res_size,CRITBIT_GETSUFFIX(node->child[CRITBIT_LEFTCHILD]));
    else
        critbit_collectsuffixes(node->child[CRITBIT_LEFTCHILD],results,nresults,res_size);

    if (CRITBIT_ISLEAF(node->child[CRITBIT_RIGHTCHILD]))
        critbit_addresult(results,nresults,res_size,CRITBIT_GETSUFFIX(node->child[CRITBIT_RIGHTCHILD]));
    else
        critbit_collectsuffixes(node->child[CRITBIT_RIGHTCHILD],results,nresults,res_size);
}

void
critbit_addresult(uint64_t** results,uint64_t* nresults,uint64_t* res_size,uint64_t suffix)
{
    if (*nresults == *res_size) {
        *res_size = *res_size * 2;
        *results = (uint64_t*) realloc(*results,*res_size);
        if (*results == NULL) {
            fprintf(stderr, "error reallocing critbit result set memory.\n");
            exit(EXIT_FAILURE);
        }
    }
    (*results)[*nresults] = suffix;
    *nresults = *nresults + 1;
}

/* print the bp representation of the critbit tree */
void
critbit_print(critbit_tree_t* cbt)
{
    critbit_print_node(cbt->root);
}

void
critbit_print_node(critbit_node_t* node)
{
    fprintf(stdout, "[");
    if (! CRITBIT_ISLEAF(node)) {
        critbit_print_node(node->child[CRITBIT_LEFTCHILD]);
        critbit_print_node(node->child[CRITBIT_RIGHTCHILD]);
    }
    fprintf(stdout, "]");
}

/* print a pretty latex version of the critbit tree */
void
critbit_print_tex(critbit_tree_t* cbt)
{
    fprintf(stdout, "\\documentclass{article}\n");
    fprintf(stdout, "\\usepackage{tikz}\n");
    fprintf(stdout, "\\usepackage{tikz-qtree}\n");
    fprintf(stdout, "\\begin{document}\n");
    fprintf(stdout, "\\begin{tikzpicture}\n");
    fprintf(stdout, "\\Tree ");
    critbit_print_tex_node(cbt->root);
    fprintf(stdout, "\n\\end{tikzpicture}\n");
    fprintf(stdout, "\\end{document}\n");
}

void
critbit_print_tex_node(critbit_node_t* node)
{
    if (! CRITBIT_ISLEAF(node)) {
        fprintf(stdout, "[");
        fprintf(stdout, ". (%lu,%lu) ",CRITBIT_GETBYTEPOS(node->crit_bit_pos),CRITBIT_GETBITPOS(node->crit_bit_pos));
        critbit_print_tex_node(node->child[CRITBIT_LEFTCHILD]);
        critbit_print_tex_node(node->child[CRITBIT_RIGHTCHILD]);
        fprintf(stdout, "]");
    } else {
        fprintf(stdout, "[.suffix %lu ]",CRITBIT_GETSUFFIX(node));
    }
}


