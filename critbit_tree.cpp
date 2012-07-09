
#include "critbit_tree.h"

#include <sdsl/int_vector.hpp>
#include <stack>

using namespace sdsl;

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

critbit_tree_t*
critbit_create_from_suffixes(const uint8_t* T,uint64_t n,uint64_t* suffixes,uint64_t nsuffixes)
{
    critbit_tree_t* cbt = critbit_create();

    /* insert the suffixes */
    for (uint64_t i=0; i<nsuffixes; i++) critbit_insert_suffix(cbt,T,n,suffixes[i]);

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

/* returns the number of bytes used by the critbit tree.

    there are g leaf nodes in the tree. therefore, there are g-1 internal nodes.
    the leaf nodes are stored in the pointers of the g-1 internal nodes.

    the number of bytes used by the critbit tree therefore is g-1 * sizeof(node size)

*/
uint64_t
critbit_getsize_in_bytes(critbit_tree_t* cbt)
{
    uint64_t bytes = sizeof(critbit_tree_t);
    bytes += ((cbt->g-1)*sizeof(critbit_node_t));
    return bytes;
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

/* returns all suffix positions matching the prefix P of length m. */
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
    fprintf(stdout, "\\Tree "); critbit_print_tex_node(cbt->root);
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

void
critbit_create_bp(critbit_node_t* node,bit_vector& bp,uint64_t* pos)
{
    bp[*pos] = 1; (*pos)++;
    if (! CRITBIT_ISLEAF(node)) {
        critbit_create_bp(node->child[CRITBIT_LEFTCHILD],bp,pos);
        critbit_create_bp(node->child[CRITBIT_RIGHTCHILD],bp,pos);
    }
    bp[*pos] = 0; (*pos)++;
}

void
critbit_create_posarray(critbit_node_t* node,int_vector<>& pos,uint64_t* p)
{
    if (! CRITBIT_ISLEAF(node)) {
        pos[*p] = node->crit_bit_pos; (*p)++;
        critbit_create_posarray(node->child[CRITBIT_LEFTCHILD],pos,p);
        critbit_create_posarray(node->child[CRITBIT_RIGHTCHILD],pos,p);
    }
}

void
critbit_create_suffixarray(critbit_node_t* node,int_vector<>& suffixes,uint64_t* p)
{
    if (! CRITBIT_ISLEAF(node)) {
        critbit_create_suffixarray(node->child[CRITBIT_LEFTCHILD],suffixes,p);
        critbit_create_suffixarray(node->child[CRITBIT_RIGHTCHILD],suffixes,p);
    } else {
        suffixes[*p] = CRITBIT_GETSUFFIX(node); (*p)++;
    }
}


uint64_t
critbit_write(critbit_tree_t* cbt,FILE* out)
{
    /* create the bp sequence of 2g bits */
    bit_vector bp((cbt->g+cbt->g-1)*2);
    uint64_t p = 0;
    critbit_create_bp(cbt->root,bp,&p);
    if (p != (cbt->g+cbt->g-1)*2) {
        fprintf(stderr, "ERROR creating bp sequence (%lu,%lu)\n",p,cbt->g*2);
    }

    /* create pos array */
    int_vector<> pos(cbt->g-1);
    p = 0;
    critbit_create_posarray(cbt->root,pos,&p);
    if (p != cbt->g-1) {
        fprintf(stderr, "ERROR creating pos array sequence (%lu,%lu)\n",p,cbt->g-1);
    }

    /* create suffixes array */
    int_vector<> suffixes(cbt->g);
    p = 0;
    critbit_create_suffixarray(cbt->root,suffixes,&p);
    if (p != cbt->g) {
        fprintf(stderr, "ERROR creating suffix array sequence (%lu,%lu)\n",p,cbt->g);
    }

    uint64_t written = 0;
    /* write g */
    written += fwrite(&cbt->g,1,sizeof(uint64_t),out);


    /* compress */
    util::bit_compress(pos);
    util::bit_compress(suffixes);

    /* write len of data */
    uint64_t pos_width = pos.get_int_width();
    uint64_t suffix_width = suffixes.get_int_width();
    written += fwrite(&pos_width,1,sizeof(uint64_t),out);
    written += fwrite(&suffix_width,1,sizeof(uint64_t),out);

    /* write bp */
    const uint64_t* bp_data = bp.data();
    uint64_t data_len = bp.capacity()>>3; /* convert bits to byte */
    written += fwrite(bp_data,1,data_len,out);

    /* write pos array */
    const uint64_t* pos_data = pos.data();
    data_len = pos.capacity()>>3; /* convert bits to byte */
    written += fwrite(pos_data,1,data_len,out);

    /* write suffixes */
    const uint64_t* suffix_data = suffixes.data();
    data_len = suffixes.capacity()>>3; /* convert bits to byte */
    written += fwrite(suffix_data,1,data_len,out);

    return written;
}

uint64_t
critbit_getelem(const uint64_t* mem,uint64_t idx,uint64_t width)
{
    uint64_t i = idx * width;
    return bit_magic::read_int(mem+(i>>6), i&0x3F, width);
}


/* reconstructs the tree from memory */
critbit_tree_t*
critbit_load_from_mem(uint64_t* mem,uint64_t size)
{
    critbit_tree_t* cbt = critbit_create();

    /* calc starting positions of all data elements */
    cbt->g = mem[0];
    uint64_t pos_width = mem[1];
    uint64_t suffix_width = mem[2];
    uint64_t* bp = &mem[3];
    uint64_t pos_offset = 3 + ((((cbt->g+cbt->g-1)*2)+63)>>6);
    uint64_t* pos = &mem[pos_offset];
    uint64_t pos_len_in_u64 = ((cbt->g * pos_width)+63)>>6;
    uint64_t suffix_offset = pos_offset + pos_len_in_u64;
    uint64_t* suffixes = &mem[suffix_offset];

    /* reconstruct the tree */
    std::stack<critbit_node_t*> stack;

    /* add root to the tree */
    critbit_node_t* cbn = (critbit_node_t*) malloc(sizeof(critbit_node_t));
    if (!cbn) {
        fprintf(stderr, "error mallocing critbit node memory.\n");
        exit(EXIT_FAILURE);
    }
    cbn->crit_bit_pos = critbit_getelem(pos,0,pos_width);
    cbn->child[CRITBIT_LEFTCHILD] = NULL;
    cbn->child[CRITBIT_RIGHTCHILD] = NULL;
    stack.push(cbn);
    cbt->root = cbn;

    /* process the bp */
    critbit_node_t* parent = NULL;
    uint64_t curpos = 1;
    uint64_t cursuffix = 0;
    uint64_t prev1pos = 0;
    uint64_t i = 1;
    while (!stack.empty()) {

        if (critbit_getelem(bp,i,1) == 1 && critbit_getelem(bp,i+1,1) == 0) {
            /* add suffix leaf to current top node of stack */
            cbn = stack.top();
            if (cbn->child[CRITBIT_LEFTCHILD] == NULL)
                cbn->child[CRITBIT_LEFTCHILD] = CRITBIT_SETSUFFIX(critbit_getelem(suffixes,cursuffix,suffix_width));
            else
                cbn->child[CRITBIT_RIGHTCHILD] = CRITBIT_SETSUFFIX(critbit_getelem(suffixes,cursuffix,suffix_width));
            cursuffix++;
            i+=2;
            continue;
        }
        if (critbit_getelem(bp,i,1) == 1 && critbit_getelem(bp,i+1,1) == 1) {
            /* add new node on the stack */
            cbn = (critbit_node_t*) malloc(sizeof(critbit_node_t));
            if (!cbn) {
                fprintf(stderr, "error mallocing critbit node memory.\n");
                exit(EXIT_FAILURE);
            }
            /* link to the parent */
            parent = stack.top();
            if (parent->child[CRITBIT_LEFTCHILD] == NULL) parent->child[CRITBIT_LEFTCHILD] = cbn;
            else parent->child[CRITBIT_RIGHTCHILD] = cbn;

            /* get data */
            cbn->crit_bit_pos = critbit_getelem(pos,curpos,pos_width);
            cbn->child[CRITBIT_LEFTCHILD] = NULL;
            cbn->child[CRITBIT_RIGHTCHILD] = NULL;
            stack.push(cbn);
            curpos++;
            i++;
            continue;
        }
        if (critbit_getelem(bp,i,1) == 0) {
            /* we are done with the top node of the stack */
            parent = stack.top();
            stack.pop();
            i++;
        }
    }
    return cbt;
}
