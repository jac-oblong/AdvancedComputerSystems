/* clang-format off */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>


uint64_t **gen_matrix(int size) {
    uint64_t **matrix = (uint64_t**)malloc(size * sizeof(uint64_t *));

    srand((unsigned) time(NULL));

    for (int i = 0; i < size; i++) {
        matrix[i] = (uint64_t*)malloc(size * sizeof(uint64_t));
        for (int j = 0; j < size; j++) {
            matrix[i][j] = rand();
        }
    }

    return matrix;
}

void miss_multiplication(uint64_t **matrix, int size) {
    volatile uint64_t product;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            product *= matrix[j][i];
        }
    }
}

void hit_multiplication(uint64_t **matrix, int size) {
    volatile uint64_t product;

    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            product *= matrix[i][j];
        }
    }
}

int main(int argc, char **argv) {
    int size = 0, miss = 0;

    // starting at 1 to avoid executable name
    for (int i = 1; i < argc;) {
        if (strcmp(argv[i], "--size") == 0) {
            size = atoi(argv[i + 1]);
            i += 2;
        } else if (strcmp(argv[i], "--miss") == 0) {
            miss = 1;
            i += 1;
        } else {
            exit(1);
        }
    }

    uint64_t **matrix = gen_matrix(size);
    if (miss) {
        miss_multiplication(matrix, size);
    } else {
        hit_multiplication(matrix, size);
    }
    free(matrix);
}
