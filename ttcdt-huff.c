/*

    ttcdt-huff - Huffman encoding library.

    ttcdt <dev@triptico.com>

    This software is released into the public domain.

    This code is my own implementation of the Huffman encoding
    algorithm. Though it has been written for teaching purposes,
    it's reasonably optimized for real use cases.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ttcdt-huff.h"

struct node {
    int f;      /* frequency */
    int n;      /* next (in sort order) */
    int b[2];   /* branches (0: left, 1: right) */
    int c;      /* char */
};

#define NUM_NODES 512


/** utility functions **/

static unsigned char *write_bits(unsigned char *ob, int *im, int bits, int v)
/* write some bits of @v into @ob, in reverse order */
{
    while (bits--) {
        if (v & 0x1)
            *ob |= *im;
        else
            *ob &= ~*im;

        v >>= 1;
        (*im) >>= 1;

        if (*im == 0) {
            *im = 0x80;
            ob++;
        }
    }

    return ob;
}


static const unsigned char *read_bits(const unsigned char *ib,
                                      int *im, int bits, int *v)
/* read some bits from @ib into @v, in reverse order */
{
    int om = 0x1;

    while (bits--) {
        if (*ib & *im)
            *v |= om;
        else
            *v &= ~om;

        (*im) >>= 1;
        om <<= 1;

        if (*im == 0) {
            *im = 0x80;
            ib++;
        }
    }

    return ib;
}


/** compression **/

static int insert_node(int i, int *z, int f, int l, int r, int c, struct node *tree)
/* inserts a node, sorted by frequency */
{
    int n, p;

    /* loop until a value with a higher frequency is found */
    for (n = i, p = -1; n != -1 && tree[n].f < f; p = n, n = tree[n].n);

    /* insert in that position */
    if (p != -1)
        tree[p].n = *z;
    else
        i = *z;

    tree[*z].n    = n;
    tree[*z].f    = f;
    tree[*z].b[0] = l;
    tree[*z].b[1] = r;
    tree[*z].c    = c;

    *z = *z + 1;

    return i;
}


int ttcdt_huff_build_tree_from_data(const unsigned char *ib, int uz, struct node *tree)
/* builds a tree from an input buffer */
{
    int n, m, r;
    int freqs[256];
    int tz = 0;

    /* reset */
    memset(tree, '\0', sizeof(struct node) * NUM_NODES);
    memset(freqs, '\0', sizeof(freqs));

    /* count frequency of symbols in data */
    for (n = 0; n < uz; n++)
        freqs[ib[n]]++;

    /* add all symbols as leaf nodes */
    m = -1;
    for (n = 0; n < 256; n++) {
        if (freqs[n])
            m = insert_node(m, &tz, freqs[n], -1, -1, n, tree);
    }

    /* build the internal nodes:
       they are created by iterating the tree from the lowest
       frequency nodes. An internal node has two leaf nodes
       and has a frequency that is the sum of both */
    for (r = m; tree[r].n != -1; r = tree[n].n) {
        n = tree[r].n;
        m = insert_node(m, &tz, tree[r].f + tree[n].f, r, n, '\0', tree);
    }

    return r;
}


void ttcdt_huff_build_symbols(const struct node *tree, int r,
                        int b, int v, int *bits, int *value)
/* builds the bits and values from a tree (recursive) */
{
    if (tree[r].b[0] == -1 && tree[r].b[1] == -1) {
        /* found leaf node */
        bits[tree[r].c] = b;
        value[tree[r].c] = v;
    }
    else {
        ttcdt_huff_build_symbols(tree, tree[r].b[0], b + 1, v,            bits, value);
        ttcdt_huff_build_symbols(tree, tree[r].b[1], b + 1, v | (1 << b), bits, value);
    }
}


unsigned char *ttcdt_huff_compress_tree(const struct node *tree, unsigned char *ob)
/* compresses a tree into @ob.
   Returns a pointer to the next byte in @ob */
{
    int n, c, xc;
    int im = 0x80;
    unsigned char *cptr;

    /* save the position where the count of leaf nodes will be stored */
    cptr = ob;
    ob++;

    n = xc = 0;
    while (tree[n].b[0] == -1 && tree[n].b[1] == -1) {
        c = 0;

        /* count how many entries in the tree are consecutive */
        while (tree[n].b[0] == -1 && tree[n].b[1] == -1 && xc == tree[n].c) {
            c++;
            xc = tree[n].c + 1;
            n++;
        }

        if (c) {
            /* store a run-length count (bit: 0) */
            ob = write_bits(ob, &im, 1, 0);
            ob = write_bits(ob, &im, 8, c);
        }

        /* store all unexpected values as verbatim symbols (bit: 1) */
        while (tree[n].b[0] == -1 && tree[n].b[1] == -1 && xc != tree[n].c) {
            ob = write_bits(ob, &im, 1, 1);
            ob = write_bits(ob, &im, 8, tree[n].c);
            xc = tree[n].c + 1;

            n++;
        }
    }

    /* store the count */
    /* note: 0 means 256 nodes (truncation helps us) */
    *cptr = n;

    /* align to byte */
    if (im != 0x80)
        ob++;

    /* save the position where count of internal nodes will be stored */
    cptr = ob;
    ob++;

    /* store only branches */
    for (c = 0; tree[n].b[0] || tree[n].b[1]; n++, c++) {
        ob = write_bits(ob, &im, 9, tree[n].b[0]);
        ob = write_bits(ob, &im, 9, tree[n].b[1]);
    }

    /* store the count */
    *cptr = c;

    if (im != 0x80)
        ob++;

    return ob;
}


unsigned char *ttcdt_huff_compress_stream(const unsigned char *ib, int uz,
                                        unsigned char *ob, int *bits, int *value)
/* compresses a stream of bytes in @ib to Huffman symbols written onto @ob.
   Returns the pointer to the next byte of @ob */
{
    int n;
    int im = 0x80;

    for (n = 0; n < uz; n++)
        ob = write_bits(ob, &im, bits[ib[n]], value[ib[n]]);

    if (im != 0x80)
        ob++;

    return ob;
}


/** decompression **/

const unsigned char *ttcdt_huff_decompress_tree(const unsigned char *ib, int *r,
                                              struct node *tree)
/* decompresses a compressed tree from inside @ib into a usable tree.
   The root node will be stored into @r.
   The tree will not have frequency information,
   but that is not necessary for decompression */
{
    int n, c, xc;
    int im = 0x80;

    /* reset the tree */
    memset(tree, '\0', sizeof(struct node) * NUM_NODES);

    /* get the count of leaf nodes */
    c = *ib;
    ib++;

    /* '0' elements means there is an entry for every char */
    if (c == 0)
        c = 256;

    n = xc = 0;
    while (n < c) {
        int p, v;

        /* read prefix and value */
        p = v = 0;
        ib = read_bits(ib, &im, 1, &p);
        ib = read_bits(ib, &im, 8, &v);

        if (p == 0) {
            /* prefix is 0: run-length sequence of consecutive values */
            if (v == 0)
                v = 256;

            while (v) {
                tree[n].b[0] = tree[n].b[1] = -1;
                tree[n].c = xc;
                xc++;
                n++;
                v--;
            }
        }
        else {
            /* prefix is 1: as-is char */
            tree[n].b[0] = tree[n].b[1] = -1;
            tree[n].c = v;
            xc = v + 1;
            n++;
        }
    }

    if (im != 0x80)
        ib++;

    /* get the count of internal nodes */
    c += *ib;
    ib++;

    for (; n < c; n++) {
        ib = read_bits(ib, &im, 9, &tree[n].b[0]);
        ib = read_bits(ib, &im, 9, &tree[n].b[1]);
    }

    if (im != 0x80)
        ib++;

    /* the root node is always the last one */
    *r = n - 1;

    return ib;
}


const unsigned char *ttcdt_huff_decompress_stream(const struct node *tree, int r,
                                                const unsigned char *ib, int uz,
                                                unsigned char *ob)
/* decompresses a compressed stream of @uz Huffman symbols.
   Returns the pointer to the next byte of @ib */
{
    int n = 0;
    int im = 0x80;

    for (n = 0; n < uz; n++) {
        int c = -1;
        int s = 0;
        int nr = r;

        do {
            if (nr < 0 || nr >= NUM_NODES)
                goto end;           /* corrupted stream */
            else
            if (tree[nr].b[0] == -1 && tree[nr].b[1] == -1)
                c = tree[nr].c;     /* symbol found */
            else {
                s <<= 1;
                ib = read_bits(ib, &im, 1, &s);
                nr = tree[nr].b[s & 0x01];
            }
        } while (c == -1);

        ob[n] = c;
    }

    if (im != 0x80)
        ib++;

end:
    return ib;
}


#ifdef TTCDT_HUFF_DEBUG

void print_tree(struct node *tree, int i)
{
    printf("\n");
    while (i != -1) {
        printf("i: %3d n: %3d f: %3d l: %3d r: %3d c: '%c' (%02x)\n",
            i, tree[i].n, tree[i].f, tree[i].b[0], tree[i].b[1],
            tree[i].c, tree[i].c);

        i = tree[i].n;
    }
}


void print_tree_raw(struct node *tree)
{
    int i = 0;

    printf("\n");
    while (tree[i].b[0] || tree[i].b[1]) {
        printf("i: %3d n: %3d f: %3d l: %3d r: %3d c: '%c' (%02x)\n",
            i, tree[i].n, tree[i].f, tree[i].b[0], tree[i].b[1],
            tree[i].c, tree[i].c);

        i++;
    }
}

#endif /* TTCDT_HUFF_DEBUG */


/** interface **/

unsigned char *ttcdt_huff_compress(const unsigned char *ib, int uz, unsigned char *ob)
/* compresses @uz bytes from @ib into @ob.
   Returns the pointer to the next byte of @ob */
{
    struct node tree[NUM_NODES];
    int r;
    int bits[256];
    int value[256];
    int im = 0x80;

    /* build a tree using this data buffer */
    r = ttcdt_huff_build_tree_from_data(ib, uz, tree);

#ifdef TTCDT_HUFF_DEBUG
    print_tree_raw(tree);
#endif

    /* build bits and values */
    ttcdt_huff_build_symbols(tree, r, 0, 0, bits, value);

    /* store the number of bytes the decompressed data contains */
    ob = write_bits(ob, &im, 24, uz);

    /* store the tree in compressed form */
    ob = ttcdt_huff_compress_tree(tree, ob);

    /* compress the data stream */
    return ttcdt_huff_compress_stream(ib, uz, ob, bits, value);
}


const unsigned char *ttcdt_huff_size(const unsigned char *ib, int *uz)
/* returns the number of bytes @ib will expand to */
{
    int im = 0x80;

    return read_bits(ib, &im, 24, uz);
}


const unsigned char *ttcdt_huff_decompress(const unsigned char *ib,
                                         unsigned char *ob)
/* decompresses @ib into @ob
   Returns the pointer to the next byte of @ib */
{
    struct node tree[NUM_NODES];
    int r, uz = 0;

    /* take the expected data size */
    ib = ttcdt_huff_size(ib, &uz);

    /* decompress the tree, getting also the root node */
    ib = ttcdt_huff_decompress_tree(ib, &r, tree);

#ifdef TTCDT_HUFF_DEBUG
    print_tree_raw(tree);
#endif

    /* decompress the stream */
    return ttcdt_huff_decompress_stream(tree, r, ib, uz, ob);
}
