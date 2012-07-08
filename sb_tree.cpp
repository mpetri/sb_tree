
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

#include "divsufsort64.h"

#include "sb_tree.h"
#include "sb_util.h"
#include "blind_trie.h"

#include <sdsl/bitmagic.hpp>

using namespace sdsl;

/* disk layout description of the index file:

	0-4095         : [B][b][n][empty space]
	4096-B+4096    : root disk page (B bytes)
	followed by    : [ rest of the SB-tree internal pages]
	followed by    : [ suffix array leaf pages ]

	therefore: root page always at file offset 4096.
*/

/* creates the suffix array for a given text and creates the SB-tree ontop of that */
sbtree_t*
sbtree_create(const char* text_file,const char* outfile,uint64_t B)
{
    char sa_file[256];
    /* load text file */
    fprintf(stderr, "LOADING TEXT\n");
    uint64_t n = sb_getfilesize(text_file);;
    uint8_t* T = (uint8_t*) sb_malloc(n);
    FILE* t_in = fopen(text_file,"r");
    if (fread(T,1,n,t_in) != n) {
        fprintf(stderr, "error reading input file '%s'\n",text_file);
        exit(EXIT_FAILURE);
    }
    fclose(t_in);

    /* create sa */
    fprintf(stderr, "CREATING SA\n");
    uint64_t* SA = (uint64_t*) sb_malloc(n*sizeof(uint64_t));
    if (divsufsort64(T,(saidx64_t*)SA,n) != 0) {
        fprintf(stderr, "error creating suffix array\n");
        exit(EXIT_FAILURE);
    }
    free(T);

    /* store sa to disk */
    strcpy(sa_file,outfile);
    strcat(sa_file,".saraw");
    FILE* sa_out = fopen(sa_file,"w");
    if (fwrite(T,sizeof(uint64_t),n,sa_out) != n) {
        fprintf(stderr, "error writing sa file '%s'\n",sa_file);
        exit(EXIT_FAILURE);
    }
    fclose(sa_out);

    return sbtree_build(sa_file,text_file,outfile,25,B);
}

/* given a sa and text on disk create a SB-tree with disk page size B */
sbtree_t*
sbtree_build(const char* sa_file,const char* text_file,const char* outfile,uint64_t maxlcp,uint64_t B)
{
    fprintf(stderr, "BUILT SBT\n");
    sbtree_t* sbt = (sbtree_t*) sb_malloc(sizeof(sbtree_t));

    sbt->n = sb_getfilesize(text_file);
    sbt->bits_per_suffix = bit_magic::l1BP(sbt->n)+1;
    sbt->bits_per_pos = bit_magic::l1BP(maxlcp)+1;;
    sbt->B = B;
    sbt->b = sbtree_calc_branch_factor(sbt);
    sbt->height = sbtree_calc_height(sbt);

    fprintf(stderr, "n = %zu\n",sbt->n);
    fprintf(stderr, "bits_per_suffix = %zu\n",sbt->bits_per_suffix);
    fprintf(stderr, "bits_per_pos = %zu\n",sbt->bits_per_pos);
    fprintf(stderr, "b = %zu\n",sbt->b);
    fprintf(stderr, "B = %zu\n",sbt->B);
    fprintf(stderr, "height = %zu\n",sbt->height);

    /* open output file */
    FILE* out = fopen(outfile,"w");
    if (!out) {
        fprintf(stderr, "cannot open output file '%s'\n",outfile);
        exit(EXIT_FAILURE);
    }

    /* write the index header first */
    sbtree_writeheader(sbt,out);

    /* now write a dummy root page that are going to overwrite later.
       we do this so the root file is always at the same offset in the index file */
    sbtree_addpadding(out,B);

    /* construct the whole sbt tree */
    FILE* sa_fd = fopen(sa_file,"r");
    /* we wrap the suffix array in the tmpfile to keep the createtree function simple */
    sbtmpfile_t* sbtf = sbtmpfile_read_from_file(sa_fd);
    sbtree_createtree(sbt,sbtf,out);
    sbtmpfile_delete(sbtf);

    /* close the index file */
    fclose(out);

    /* open the file so we can use the sbt right away */
    sbt->fd = open(outfile,O_RDONLY);
    sbt->textfd = open(text_file,O_RDONLY);

    return sbt;
}

/* stream sa from disk and construct the sb-tree */
void
sbtree_createtree(sbtree_t* sbt,sbtmpfile_t* suffixes,FILE* sbt_fd)
{
    /* tmp file we store the next level in */
    sbtmpfile_t* next_level = sbtmpfile_create_write();

    /* read b suffixes and process */
    sbtmpfile_open_read(suffixes);
    uint64_t* suf = (uint64_t*) sb_malloc(sbt->b*sizeof(uint64_t));
    uint64_t* next_suf = (uint64_t*) sb_malloc(sbt->b*sizeof(uint64_t));
    uint64_t j,n; j = 0;
    while ((n=sbtmpfile_read_block(suffixes,suf,sbt->b)) > 0) {
        blindtrie_t* bt = blindtrie_create(sbt,suf,n);

        /* write node to the index file */
        blindtrie_write(sbt,bt,sbt_fd);

        /* add the first suffix in block to next lvl file */
        next_suf[j] = suf[0]; j++;
        if (j==sbt->b) {
            sbtmpfile_write_block(next_level,next_suf,sbt->b);
            j = 0;
        }
    }
    /* write last block */
    if (j>0) sbtmpfile_write_block(next_level,next_suf,j);

    free(suf);
    free(next_suf);
    sbtmpfile_finish(next_level);

    /* recurse to the next level */
    sbtree_createtree(sbt,next_level,sbt_fd);
}

/* load a SB-tree from disk */
sbtree_t*
sbtree_load(const char* sb_file,const char* text_file)
{
    sbtree_t* sbt = (sbtree_t*) sb_malloc(sizeof(sbtree_t));

    /* read index file */
    FILE* in = fopen(sb_file,"r");
    if (!in) {
        fprintf(stderr, "cannot open output file '%s'\n",sb_file);
        exit(EXIT_FAILURE);
    }

    /* read the header first */
    sbtree_readheader(sbt,in);

    /* open the file so we can use the sbt right away */
    sbt->fd = open(sb_file,O_RDONLY);
    sbt->textfd = open(text_file,O_RDONLY);

    /* read/map the root page */
    sbt->root = sbtree_load_diskpage(sbt,SBT_ROOT_OFFSET);

    return sbt;
}

/* print storage statistics for the SB-tree to stdout */
void
sbtree_printstats(const sbtree_t* sbt)
{

}

/* free the sb tree data structure */
void
sbtree_free(sbtree_t* sbt)
{
    if (sbt) {
        sbtree_free_diskpage(sbt,sbt->root);
        close(sbt->fd);
        close(sbt->textfd);
        free(sbt);
    }
}

/* for a given disk page size B in bytes and the size of the text n,
   calculate the branching factor b.

	calculation as follows.

	  - each node in the SB-T contains b <= |n| <= 2b suffixes
	  - each node must fit into B bytes
	  - each node contains:
	    o |n|+1 child pointers (offset into the sb index to the
	    						disk page containing the child)
	       note: as the sb-tree index is organized via disk pages,
	             we can use log(n/(2b)) bits per child pointer.
		o a blind trie over all |n| suffixes
 */
uint64_t
sbtree_calc_branch_factor(sbtree_t* sbt)
{
    return (uint64_t)(sbt->B/(0.25 + ((sbt->bits_per_pos + sbt->bits_per_suffix)/8.0f)));
}

sb_diskpage_t*
sbtree_load_diskpage(const sbtree_t* sbt,uint64_t offset)
{
    return (sb_diskpage_t*) mmap(NULL,sbt->B,PROT_READ,
                                 MAP_PRIVATE|MAP_POPULATE,sbt->fd,offset);
}

void
sbtree_free_diskpage(const sbtree_t* sbt,sb_diskpage_t* sbd)
{
    munmap((void*)sbd,sbt->B);
}

/* query functions */
uint64_t*
sbtree_search(const sbtree_t* sbt,const uint8_t* P,uint64_t m,uint64_t* nres)
{
    uint64_t* results = NULL;
    uint64_t  cur_height = 0;
    uint64_t  child_offset = 0;
    uint64_t  l = 0;

    /* start with the root */
    sb_diskpage_t* cur_node = sbt->root;

    /*while( cur_height < sbt->height &&
    		(child_offset=blindtrie_search(sbt,cur_node,P,m,&l))!= 0 ) {*/
    {

        /* go to the next level + get new page */
        sbtree_free_diskpage(sbt,cur_node); /* free old page */
        cur_node = sbtree_load_diskpage(sbt,child_offset);
        cur_height++;
    }

    /* we are either at the correct leaf or not found */
    if (child_offset) {
        /* traverse that part of the SA to find the matches */
    } else {
        /* not found */
        *nres = 0;
    }

    return results;
}


/* calculate the SB-Tree height */
uint64_t
sbtree_calc_height(const sbtree_t* sbt)
{
    return (uint64_t) log(sbt->n)/log(sbt->b) + 1;
}

/* write the index header + padding */
void
sbtree_writeheader(sbtree_t* sbt,FILE* out)
{
    uint64_t written = 0;
    written += fwrite(&sbt->n,sizeof(uint64_t),1,out);
    written += fwrite(&sbt->bits_per_suffix,sizeof(uint64_t),1,out);
    written += fwrite(&sbt->bits_per_pos,sizeof(uint64_t),1,out);
    written += fwrite(&sbt->b,sizeof(uint64_t),1,out);
    written += fwrite(&sbt->B,sizeof(uint64_t),1,out);
    written += fwrite(&sbt->height,sizeof(uint64_t),1,out);

    /* pad up to SBT_ROOT_OFFSET bytes so we have nice alignment */
    sbtree_addpadding(out,SBT_ROOT_OFFSET-(written*sizeof(uint64_t)));
}

/* add padding to the index file to get nice alignment */
void
sbtree_addpadding(FILE* out,uint64_t bytes)
{
    uint8_t* dummy_root = (uint8_t*) sb_malloc(bytes);
    memset(dummy_root,rand()%50,bytes);
    fwrite(dummy_root,1,bytes,out);
    free(dummy_root);
}

/* read the index header */
void
sbtree_readheader(sbtree_t* sbt,FILE* in)
{
    uint64_t read = 0;
    read += fread(&sbt->n,sizeof(uint64_t),1,in);
    read += fread(&sbt->bits_per_suffix,sizeof(uint64_t),1,in);
    read += fread(&sbt->bits_per_pos,sizeof(uint64_t),1,in);
    read += fread(&sbt->b,sizeof(uint64_t),1,in);
    read += fread(&sbt->B,sizeof(uint64_t),1,in);
    read += fread(&sbt->height,sizeof(uint64_t),1,in);
    fclose(in);

    if (read != 6) {
        fprintf(stderr, "error reading index file.\n");
        exit(EXIT_FAILURE);
    }
}