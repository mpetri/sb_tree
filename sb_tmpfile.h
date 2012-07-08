
#ifndef SB_TMPFILE_H
#define SB_TMPFILE_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#define SBTMPFILE_ERROR		0
#define SBTMPFILE_WRITE     1
#define SBTMPFILE_READREAD	2
#define SBTMPFILE_READ      3

typedef struct {
    FILE* f;
    uint64_t block_size;
    int access_mode;
} sbtmpfile_t;

/* file I/O */
sbtmpfile_t* sbtmpfile_read_from_file(FILE* f);
sbtmpfile_t* sbtmpfile_create_write();
void sbtmpfile_finish(sbtmpfile_t* stf);
int sbtmpfile_open_read(sbtmpfile_t* stf);
void sbtmpfile_delete(sbtmpfile_t* stf);

/* block input output */
void sbtmpfile_write_block(sbtmpfile_t* stf,uint64_t* buf,uint64_t n);
uint64_t sbtmpfile_read_block(sbtmpfile_t* tf,uint64_t* buf,uint64_t max);

#endif