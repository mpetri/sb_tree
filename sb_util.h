
#ifndef SBUTIL_H
#define SBUTIL_H

static inline void* sb_malloc(size_t bytes)
{
    void* mem = calloc(bytes,1);
    if (!mem) {
        fprintf(stderr, "error allocating %zu bytes\n",bytes);
        exit(EXIT_FAILURE);
    }
    return mem;
}

static inline uint64_t sb_getfilesize(const char* file_name)
{
    FILE* f = fopen(file_name,"r");
    if (f) {
        fseek(f,0,SEEK_END);
        long fs = ftell(f);
        fclose(f);
        return (uint64_t)fs;
    } else {
        fprintf(stderr, "could not open file '%s'\n",file_name);
        exit(EXIT_FAILURE);
    }
}

static inline uint64_t sb_getmaxlcp(const uint8_t* T,const char* sa_file)
{

}

#endif