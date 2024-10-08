#include <immintrin.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define RAND_F_MAX 100.0F

/******************************************************************************/
/*                                                                            */
/*                                   TYPES                                    */
/*                                                                            */
/******************************************************************************/

typedef float Value;

typedef struct {
  size_t size;
  Value **values;
} Matrix;

typedef struct {
  size_t size;
  size_t num_non_zero;
  Value *values;
  size_t *col_index;
  size_t *row_index;
} SparseMatrix;

/******************************************************************************/
/*                                                                            */
/*                              MATRIX FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

Matrix new_matrix(size_t size) {
  Matrix matrix = {0};

  // allocate space for matrix
  matrix.size = size;
  matrix.values = (Value **)malloc(size * sizeof(Value *));
  for (size_t i = 0; i < size; i++) {
    matrix.values[i] = (Value *)malloc(size * sizeof(Value));
  }

  return matrix;
}

void del_matrix(Matrix *matrix) {
  for (size_t i = 0; i < matrix->size; i++) {
    free(matrix->values[i]);
  }
  free(matrix->values);
  matrix->values = NULL;
}

void del_smatrix(SparseMatrix *smatrix) {
  free(smatrix->values);
  free(smatrix->col_index);
  free(smatrix->row_index);
  smatrix = NULL;
}

Matrix new_rand_matrix(size_t size) {
  Matrix matrix = {0};

  // allocate space
  matrix = new_matrix(size);

  // fill in matrix with random values
  for (size_t i = 0; i < size; i++) {
    for (size_t j = 0; j < size; j++) {
      matrix.values[i][j] = ((float)rand() / (float)RAND_MAX) * RAND_F_MAX;
    }
  }

  return matrix;
}

void make_sparse_matrix(Matrix matrix, float sparsity) {
  const size_t num_zero = sparsity * matrix.size * matrix.size;

  for (size_t i = 0; i < num_zero; i++) {
    size_t r, c;
    do {
      r = rand() % matrix.size;
      c = rand() % matrix.size;
    } while (matrix.values[r][c] == 0);
    matrix.values[r][c] = 0;
  }
}

SparseMatrix compress_matrix(Matrix matrix) {
  SparseMatrix smatrix = {0};

  smatrix.size = matrix.size;

  // count zeros
  for (size_t i = 0; i < matrix.size; i++) {
    for (size_t j = 0; j < matrix.size; j++) {
      if (matrix.values[i][j] == 0) {
        smatrix.num_non_zero++;
      }
    }
  }

  // allocate memory
  smatrix.row_index = (size_t *)malloc((smatrix.size + 1) * sizeof(size_t));
  smatrix.col_index = (size_t *)malloc(smatrix.num_non_zero * sizeof(size_t));
  smatrix.values = (Value *)malloc(smatrix.num_non_zero * sizeof(Value));

  // create sparce matrix     https://en.wikipedia.org/wiki/Sparse_matrix
  size_t row = 0, col = 0;
  smatrix.row_index[row] = 0;
  for (size_t i = 0; i < matrix.size; i++) {
    for (size_t j = 0; j < matrix.size; j++) {
      if (matrix.values[i][j] == 0) {
        continue;
      }

      smatrix.values[col] = matrix.values[i][j];
      smatrix.col_index[col] = j;
      col++;
    }
    smatrix.row_index[row] = col;
    row++;
  }

  return smatrix;
}

Matrix transpose_matrix(Matrix m) {
  Matrix t = new_matrix(m.size);

  for (size_t i = 0; i < m.size; i++) {
    for (size_t j = 0; j < m.size; j++) {
      t.values[i][j] = m.values[j][i];
    }
  }

  return t;
}

/******************************************************************************/
/*                                                                            */
/*                           MATRIX MULTIPLICATION                            */
/*                                                                            */
/******************************************************************************/

void matrix_mult_region(Matrix a, Matrix b, Matrix c, size_t beg, size_t end) {
  const size_t size = a.size;

  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < size; j++) {
      c.values[i][j] = 0;
      for (size_t k = 0; k < size; k++) {
        c.values[i][j] += a.values[i][k] * b.values[k][j];
      }
    }
  }
}

void matrix_mult_region_c(Matrix a, Matrix b, Matrix c, size_t beg,
                          size_t end) {
  const size_t size = a.size;

  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < size; j++) {
      c.values[i][j] = 0;
      for (size_t k = 0; k < size; k++) {
        c.values[i][j] += a.values[i][k] * b.values[j][k];
      }
    }
  }
}

void matrix_mult_region_d(Matrix a, Matrix b, Matrix c, size_t beg,
                          size_t end) {
  // TODO
}

void matrix_mult_region_dc(Matrix a, Matrix b, Matrix c, size_t beg,
                           size_t end) {
  // https://blog.qiqitori.com/2018/05/matrix-multiplication-using-simd-instructions/
  const size_t size = a.size;
  __m128 va, vb, vc;

  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < size; j++) {
      c.values[i][j] = 0;
      for (size_t k = 0; k < size; k += 4) {
        va = _mm_loadu_ps(&a.values[i][k]);
        vb = _mm_loadu_ps(&b.values[j][k]);

        vc = _mm_mul_ps(va, vb);

        vc = _mm_hadd_ps(vc, vc);
        vc = _mm_hadd_ps(vc, vc);

        c.values[i][j] += _mm_cvtss_f32(vc);
      }
    }
  }
}

Matrix matrix_mult(Matrix *a, Matrix *b) {
  Matrix c = new_matrix(a->size);
  matrix_mult_region(*a, *b, c, 0, c.size);
  return c;
}

Matrix matrix_mult_t(Matrix *a, Matrix *b, size_t threads) {
  // TODO
}

Matrix matrix_mult_d(Matrix *a, Matrix *b) {
  Matrix c = new_matrix(a->size);
  matrix_mult_region_d(*a, *b, c, 0, c.size);
  return c;
}

Matrix matrix_mult_c(Matrix *a, Matrix *b) {
  Matrix c = new_matrix(a->size);
  Matrix t = transpose_matrix(*b);
  matrix_mult_region_c(*a, t, c, 0, c.size);
  del_matrix(b);
  return c;
}

Matrix matrix_mult_td(Matrix *a, Matrix *b) {
  // TODO
}

Matrix matrix_mult_tc(Matrix *a, Matrix *b) {
  // TODO
}

Matrix matrix_mult_dc(Matrix *a, Matrix *b) {
  Matrix c = new_matrix(a->size);
  Matrix t = transpose_matrix(*b);
  matrix_mult_region_dc(*a, t, c, 0, c.size);
  del_matrix(b);
  return c;
}

Matrix matrix_mult_tdc(Matrix *a, Matrix *b) {
  // TODO
}

Matrix matrix_mult_dd(Matrix a, Matrix b) { return matrix_mult(&a, &b); }

SparseMatrix matrix_mult_ds(Matrix a, SparseMatrix b) {
  // TODO
}

SparseMatrix matrix_mult_ss(SparseMatrix a, SparseMatrix b) {
  // TODO
}

/******************************************************************************/
/*                                                                            */
/*                                    MAIN                                    */
/*                                                                            */
/******************************************************************************/

void print_help(char *exe_name) {
  printf("USAGE: %s [-t <n>] [-dcs] [-s1 <%%>] [-s2 <%%>] <size>\n", exe_name);
  printf("       -t <n>    enable multi-threading with 'n' threads\n");
  printf("       -d        enable simd\n");
  printf("       -c        enable cache optimization\n");
  printf("       -s        enable sparse matrix compression\n");
  printf("       -s1 <%%>  sparcity percentage for matrix 1\n");
  printf("       -s2 <%%>  sparcity percentage for matrix 2\n");
  printf("       <size>    size of square matrix\n");

  exit(1);
}

int main(int argc, char **argv) {
  srand(time(NULL));

  if (argc < 2) {
    print_help(argv[0]);
  }

  size_t threads = 0, size = 0;
  float sparcity1 = 0, sparcity2 = 0;
  bool simd = false, cache = false, sparse = false;

  for (int i = 1; i < argc; i++) {
    if (strncmp("-t", argv[i], 2) == 0) {
      char *ret;
      threads = strtoul(argv[++i], &ret, 10);
      if (*ret != '\0') {
        printf("INVALID THREAD COUNT: %s\n\n", argv[i]);
        print_help(argv[0]);
      }

    } else if (strncmp("-s1", argv[i], 3) == 0) {
      char *ret;
      sparcity1 = strtof(argv[++i], &ret);
      if (*ret != '\0') {
        printf("INVALID SPARCITY PERCENTAGE: %s\n\n", argv[i]);
        print_help(argv[0]);
      }

    } else if (strncmp("-s2", argv[i], 3) == 0) {
      char *ret;
      sparcity2 = strtof(argv[++i], &ret);
      if (*ret != '\0') {
        printf("INVALID SPARCITY PERCENTAGE: %s\n\n", argv[i]);
        print_help(argv[0]);
      }

    } else if (strncmp("-", argv[i], 1) == 0) {
      simd = (strchr(argv[i], 'd') != NULL);
      cache = (strchr(argv[i], 'c') != NULL);
      sparse = (strchr(argv[i], 's') != NULL);

      for (int j = 1; j < strlen(argv[i]); j++) {
        switch (argv[i][j]) {
        case 'd':
        case 'c':
        case 's':
          break;
        default:
          printf("UNRECOGNIZED OPTION: %c\n\n", argv[i][j]);
          print_help(argv[0]);
        }
      }

    } else {
      char *ret;
      size = strtoul(argv[i], &ret, 10);
      if (size == 0 || *ret != '\0') {
        printf("INVALID MATRIX SIZE: %s\n\n", argv[i]);
        print_help(argv[0]);
      }
    }
  }

  if (size == 0) {
    printf("MATRIX SIZE NOT SPECIFIED\n\n");
    print_help(argv[0]);
  }

  // TODO
}
