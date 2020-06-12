/*

    ttcdt-huff - Huffman encoding library.

    ttcdt <dev@triptico.com>

    This software is released into the public domain.

*/

#define TTCDT_HUFF_VERSION "1.05"

/**
 * ttcdt_huff_compress - Compresses a block of data.
 * @ib: input buffer
 * @uz: data size in bytes
 * @ob: output buffer
 *
 * Compresses the @ib block of @uz bytes into the buffer
 * pointed by @ob. @uz must be non-zero. @ob must
 * be at least @uz size.
 *
 * Returns the pointer to the next byte in @ib.
 */
unsigned char *ttcdt_huff_compress(const unsigned char *ib, int uz,
                                 unsigned char *ob);

/**
 * ttcdt_huff_size - Gets the size of stored data.
 * @ib: input buffer
 * @uz: pointer to store the uncompressed data size.
 *
 * Gets the number of bytes @ib will expand to
 * after decompression.
 *
 * Returns the pointer to the next byte in @ib.
 */
const unsigned char *ttcdt_huff_size(const unsigned char *ib, int *uz);

/**
 * ttcdt_huff_decompress - Decompresses a block of compressed data.
 * @ib: input buffer
 * @ob: output buffer
 *
 * Decompresses the compressed data block in @ib into the
 * buffer pointed by @ob. The buffer must have enough
 * size for the uncompressed block (see ttcdt_huff_size()).
 *
 * Returns the pointer to the next byte in @ib.
 */
const unsigned char *ttcdt_huff_decompress(const unsigned char *ib,
                                         unsigned char *ob);
