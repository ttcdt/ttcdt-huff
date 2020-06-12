/*

    ttcdt-huff-ar - Extremely simple Huffman file archiver.

    ttcdt <dev@triptico.com>

    This software is released into the public domain.

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "ttcdt-huff.h"

void usage(void)
{
    printf("ttcdt-huff-ar - Extremely simple Huffman file archiver\n");
    printf("ttcdt <dev@triptico.com> -- public domain\n\n");

    printf("Usage:\n");
    printf("  ttcdt-huff-ar c archive.aha [file(s)...]    Create archive\n");
    printf("  ttcdt-huff-ar t archive.aha                 Test (list) archive\n");
}

int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 3) {
        usage();
        ret = 1;
    }
    else
    if (strcmp(argv[1], "c") == 0) {
        FILE *o;

        if ((o = fopen(argv[2], "wb")) != NULL) {
            int n;

            /* write signature */
            fwrite("aha", 4, 1, o);

            for (n = 3; n < argc; n++) {
                struct stat s;
                FILE *i;

                if ((i = fopen(argv[n], "rb")) != NULL && fstat(fileno(i), &s) != -1) {
                    unsigned char *ib, *ob, *ptr;
                    int z = s.st_size, nz;

                    /* alloc working size */
                    ib = malloc(z);
                    ob = malloc(z * 3);

                    /* read and compress in one chunk */
                    fread(ib, z, 1, i);
                    ptr = ttcdt_huff_compress(ib, z, ob);

                    /* compressed size */
                    nz = ptr - ob;

                    /* write file name */
                    fwrite(argv[n], strlen(argv[n]) + 1, 1, o);

                    if (nz < z) {
                        /* compressed */

                        /* write size */
                        fwrite(&nz, sizeof(nz), 1, o);

                        /* write compressed stream */
                        fwrite(ob, nz, 1, o);
                    }
                    else {
                        /* uncompressed */

                        /* write size */
                        nz = -z;
                        fwrite(&nz, sizeof(nz), 1, o);

                        /* write uncompressed stream */
                        fwrite(ib, z, 1, o);
                    }

                    free(ib);
                    free(ob);

                    fclose(i);
                }
                else
                    printf("WARN : cannot open '%s'\n", argv[n]);
            }

            fclose(o);
        }
        else {
            printf("ERROR: cannot create '%s'\n", argv[2]);
            ret = 3;
        }
    }
    else
    if (strcmp(argv[1], "t") == 0) {
        FILE *i;

        if ((i = fopen(argv[2], "rb")) != NULL) {
            char buf[5];

            buf[4] = '\0';

            /* read signature */
            fread(buf, 4, 1, i);

            if (strcmp("aha", buf) == 0) {
                /* print file name */
                int c, z;

                while (!feof(i)) {
                    while ((c = fgetc(i)) != EOF && c != '\0')
                        printf("%c", c);
    
                    if (c == EOF)
                        break;

                    /* read file size */
                    fread(&z, sizeof(z), 1, i);
    
                    if (z < 0) {
                        /* uncompressed */
                        printf(" %d (uncompressed)\n", -z);
    
                        fseek(i, -z, SEEK_CUR);
                    }
                    else {
                        /* compressed */
                        unsigned char *ib = malloc(z);
                        int uz;
    
                        /* read compressed file */
                        fread(ib, z, 1, i);
    
                        /* get uncompressed size */
                        ttcdt_huff_size(ib, &uz);
    
                        printf(" %d %d (%.2f%%)\n", z, uz,
                            100.0 * ((float)uz - (float)z) / (float)uz);
                    }
                }
            }
            else {
                printf("ERROR: '%s' not a .aha archive\n", argv[2]);
                ret = 4;
            }
        }
        else {
            printf("ERROR: cannot open '%s'\n", argv[2]);
            ret = 3;
        }
    }
    else {
        usage();
        ret = 2;
    }

    return ret;
}
