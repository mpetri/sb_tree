
#include <stdio.h>

#include "sb_tmpfile.h"
#include "sb_util.h"

/* file I/O */

sbtmpfile_t*
sbtmpfile_read_from_file(FILE* f)
{
    sbtmpfile_t* stf = (sbtmpfile_t*) sb_malloc(sizeof(sbtmpfile_t));
    stf->f = f;

    if (!stf->f) {
        fprintf(stderr, "error opening existing tmpfile.\n");
        exit(EXIT_FAILURE);
    } else {
        fseek(stf->f,0,SEEK_SET); /* seek to the start */
        stf->access_mode = SBTMPFILE_READREAD;
    }

    return stf;
}

sbtmpfile_t*
sbtmpfile_create_write()
{
    sbtmpfile_t* stf = (sbtmpfile_t*) sb_malloc(sizeof(sbtmpfile_t));

    stf->f = tmpfile();

    if (!stf->f) {
        fprintf(stderr, "error creating tmpfile.\n");
        exit(EXIT_FAILURE);
    } else {
        stf->access_mode = SBTMPFILE_WRITE;
    }
    return stf;
}

void
sbtmpfile_finish(sbtmpfile_t* stf)
{
    if (stf->access_mode == SBTMPFILE_WRITE) {
        stf->access_mode = SBTMPFILE_READREAD;
    } else {
        fprintf(stderr, "error finish writing tmpfile.\n");
        exit(EXIT_FAILURE);
    }
}

int sbtmpfile_open_read(sbtmpfile_t* stf)
{
    if (stf->access_mode == SBTMPFILE_READREAD) {
        stf->access_mode = SBTMPFILE_READ;
        fseek(stf->f,0,SEEK_SET); /* seek to the start */
    } else {
        fprintf(stderr, "error start reading tmpfile.\n");
        exit(EXIT_FAILURE);
    }
}

void sbtmpfile_delete(sbtmpfile_t* stf)
{
    if (stf) {
        fclose(stf->f);
        free(stf);
    }
}

/* block input output */
void sbtmpfile_write_block(sbtmpfile_t* stf,uint64_t* buf,uint64_t n)
{
    if (stf->access_mode == SBTMPFILE_WRITE) {
        if (fwrite(buf,sizeof(uint64_t),n,stf->f) != n) {
            fprintf(stderr, "error writing block to tmpfile.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        fprintf(stderr, "error writing block to tmpfile.\n");
        exit(EXIT_FAILURE);
    }
}

uint64_t sbtmpfile_read_block(sbtmpfile_t* stf,uint64_t* buf,uint64_t max)
{
    if (stf->access_mode == SBTMPFILE_READ) {
        return fread(buf,sizeof(uint64_t),max,stf->f);
    } else {
        fprintf(stderr, "error reading block from tmpfile.\n");
        exit(EXIT_FAILURE);
    }
}