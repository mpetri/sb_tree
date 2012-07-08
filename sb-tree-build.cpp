
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "sb_tree.h"

typedef struct {
    uint64_t B;
    const char* sa;
    const char* input;
    const char* output;
} cmd_args_t;

void
print_usage(const char* program)
{
    printf("USAGE: %s -i <input> -s <sa> -o <output.sbti> -B <disk page size>\n",program);
    printf("WHERE:\n");
    printf("        -i <input>          : input file\n");
    printf("        -s <sa>             : already constructed suffix array (optional)\n");
    printf("        -o <output>         : output index file\n");
    printf("        -B <disk page size> : disk page size in bytes\n\n");
}

cmd_args_t
parse_args(int argc,char** argv)
{
    int op;
    cmd_args_t args;

    args.sa = args.input = args.output = NULL;
    args.B = 0;

    while ((op=getopt(argc,argv,"i:s:o:B:")) != -1) {
        switch (op) {
            case 'i':
                args.input = optarg;
                break;
            case 's':
                args.sa = optarg;
                break;
            case 'o':
                args.output = optarg;
                break;
            case 'B':
                args.B = atoll(optarg);
                break;
            case '?':
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (args.input == NULL || args.output == NULL || args.B == 0) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    return args;
}

int
main(int argc,char** argv)
{
    sbtree_t* sbt;
    cmd_args_t cargs = parse_args(argc,argv);

    /* build */
    if (cargs.sa != NULL) {
        sbt = sbtree_build(cargs.sa,cargs.input,cargs.output,20,cargs.B);
    } else {
        sbt = sbtree_create(cargs.input,cargs.output,cargs.B);
    }

    /* free the index */
    sbtree_free(sbt);

    return EXIT_SUCCESS;
}