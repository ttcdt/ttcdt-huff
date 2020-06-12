/*

    ttcdt-huff - Huffman encoding library.

    ttcdt <dev@triptico.com>

    This software is released into the public domain.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ttcdt-huff.h"

void usage(void)
{
    printf("ttcdt-huff - Huffman file compressor\n");
    printf("ttcdt <dev@triptico.com>\n\n");

    printf("Usage:\n");
    printf("  ttcdt-huff -C               Compress STDIN to STDOUT\n");
    printf("  ttcdt-huff -D               Decompress STDIN to STDOUT\n");
}


#define CHUNK_SIZE 16384

int compress(FILE *i, FILE *o)
{
    int ret = 0;
    int z;
    unsigned char bi[CHUNK_SIZE];
    unsigned char bo[CHUNK_SIZE * 2];

    while ((z = fread(bi, 1, CHUNK_SIZE, i))) {
        unsigned char *ptr;
        int cz;

        ptr = ttcdt_huff_compress(bi, z, bo);
        cz = ptr - bo;

        if (cz >= z) {
            /* non-compressed block */
            z = -z;
            fwrite(&z, sizeof(z), 1, o);
            fwrite(bi, 1, -z, o);
        }
        else {
            fwrite(&cz, sizeof(cz), 1, o);
            fwrite(bo, 1, cz, o);
        }
    }

    return ret;
}


int decompress(FILE *i, FILE *o)
{
    int ret = 0;
    int z;
    unsigned char bi[CHUNK_SIZE];
    unsigned char bo[CHUNK_SIZE];

    while (fread(&z, sizeof(z), 1, i)) {
        if (z < 0) {
            /* non-compressed block */
            fread(bi, 1, -z, i);
            fwrite(bi, 1, -z, o);
        }
        else {
            int dz;

            fread(bi, 1, z, i);

            ttcdt_huff_size(bi, &dz);

            if (dz > CHUNK_SIZE) {
                fprintf(stderr, "ttcdt-huff: error: corrupted stream\n");
                ret = 4;
                break;
            }

            ttcdt_huff_decompress(bi, bo);

            fwrite(bo, 1, dz, o);
        }
    }

    return ret;
}


int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc == 1) {
        usage();
        ret = 1;
    }
    else
    if (strcmp(argv[1], "-C") == 0) {
        ret = compress(stdin, stdout);
    }
    else
    if (strcmp(argv[1], "-D") == 0) {
        ret = decompress(stdin, stdout);
    }
    else {
        usage();
        ret = 2;
    }

    return ret;
}
