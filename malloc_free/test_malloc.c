#include <stdio.h>
#include <stdlib.h>

#define NBLOCKS 5000

int main()
{

    int chunk1 = 1024*1024; /* let's do it a megabyte at a time */
    int chunk2;
    int i; /* generic loop index */
    int n1; /* number of chunk1 blocks from first allocation */
    int f1; /* number of chunk1 blocks freed */
    int n2; /* number of chunk2 blocks from second allocation */
    int f2; /* number of chunk2 blocks freed */
    char *pointer1[NBLOCKS]; /* array of pointers */
    char *pointer2[NBLOCKS]; /* array of pointers */
    for (i = 0; i < NBLOCKS; i++) { /* make really, really sure that they are all zero */
        pointer1[i] = NULL;
        pointer2[i] = NULL;
    }
    for (n1 = 0; n1 < NBLOCKS ; n1++) {
        pointer1[n1] = malloc(chunk1);
        if (!pointer1[n1]) { /* malloc failed: returned NULL */
            break;
        }
    }
    printf("1: Allocated %4d chunks of %d bytes.\n", n1, chunk1);
    f1 = 0;
    for (i = 0; i < NBLOCKS; i += 2) { /* free every other block */
        if (pointer1[i]) {
            free(pointer1[i]);
            pointer1[i] = 0;
            f1++;
        }
    }
    printf("1: Freed %4d chunks of %d bytes.\n\n", f1, chunk1);

#if 0
    chunk2 = chunk1;
#else
    chunk2 = 2 * chunk1;
    printf("New chunk size = %d\n\n", chunk2);
#endif
    for (n2 = 0; n2 < NBLOCKS; n2++) {
        pointer2[n2] = malloc(chunk2);
        if (!pointer2[n2]) {
            break;
        }
    }
    printf("2: Allocated %4d chunks of %d bytes.\n", n2, chunk2);
    f2 = 0;
    for (i = 0; i < NBLOCKS; i++) {
        if (pointer2[i]) {
            free(pointer2[i]);
            f2++;
        }
    }
    printf("2: Freed %4d chunks of %d bytes.\n\n", f2, chunk2);
    f1 = 0;
    for (i = 0; i < NBLOCKS; i++) {
        if (pointer1[i]) {
            free(pointer1[i]);
            f1++;
        }
    }
    printf("1: Freed %4d chunks of %d bytes.\n", f1, chunk1);
    return 0;
}
