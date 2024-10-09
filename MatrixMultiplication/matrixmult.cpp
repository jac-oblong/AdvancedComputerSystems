#include "matrixmult.h"
#include <algorithm>
#include <cstddef>
#include <immintrin.h>
#include <ostream>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>
#include <xmmintrin.h>

typedef void (*MatrixMultFunc)(DenseMatrix &, DenseMatrix &, DenseMatrix &,
                               size_t, size_t);

/******************************************************************************/
/*                                                                            */
/*                              GENERAL HELPERS                               */
/*                                                                            */
/******************************************************************************/

static std::random_device r;
static std::default_random_engine generator{r()};

float rand_f(void) {
  static std::uniform_real_distribution<float> fdistribution(0.0, 1000.0);
  return fdistribution(generator);
}

/******************************************************************************/
/*                                                                            */
/*                       MATRIX MULTIPLICATION HELPERS                        */
/*                                                                            */
/******************************************************************************/

void MatrixMultRegion(DenseMatrix &a, DenseMatrix &b, DenseMatrix &c,
                      size_t beg, size_t end) {
  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < a.num_cols(); j++) {
      float sum = 0;
      for (size_t k = 0; k < b.num_rows(); k++) {
        sum += a.get(i, k) * b.get(k, j);
      }
      c.set(i, j, sum);
    }
  }
}

void MatrixMultRegion_SIMD(DenseMatrix &a, DenseMatrix &b, DenseMatrix &c,
                           size_t beg, size_t end) {
  __m128 va, vb, vc;
  for (size_t i = 0; i < a.num_rows(); i++) {
    for (size_t j = 0; j < b.num_cols(); j += 4) {
      vc = _mm_setzero_ps();
      for (size_t k = 0; k < a.num_cols(); k++) {
        va = _mm_set1_ps(a.get(i, k));
        vb = _mm_loadu_ps(&b.get(k, j));
        vc = _mm_add_ps(vc, _mm_mul_ps(va, vb));
      }
      _mm_storeu_ps(&c.get(i, j), vc);
    }
  }
}

void MatrixMultRegion_Cache(DenseMatrix &a, DenseMatrix &b, DenseMatrix &c,
                            size_t beg, size_t end) {
  b.transpose();

  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < b.num_rows(); j++) {
      float sum = 0;
      for (size_t k = 0; k < a.num_cols(); k++) {
        sum += a.get(i, k) * b.get(j, k);
      }
      c.set(i, j, sum);
    }
  }
}

void MatrixMultRegion_SIMD_Cache(DenseMatrix &a, DenseMatrix &b, DenseMatrix &c,
                                 size_t beg, size_t end) {
  // https://blog.qiqitori.com/2018/05/matrix-multiplication-using-simd-instructions/
  __m128 va, vb, vc;

  b.transpose();

  for (size_t i = beg; i < end; i++) {
    for (size_t j = 0; j < b.num_rows(); j++) {
      float sum = 0;
      for (size_t k = 0; k < a.num_cols(); k += 4) {
        va = _mm_loadu_ps(&a.get(j, k));
        vb = _mm_loadu_ps(&b.get(j, k));

        vc = _mm_mul_ps(va, vb);

        vc = _mm_hadd_ps(vc, vc);
        vc = _mm_hadd_ps(vc, vc);

        sum += _mm_cvtss_f32(vc);
      }
      c.set(i, j, sum);
    }
  }
}

/******************************************************************************/
/*                                                                            */
/*                               DENSE MATRIX                                 */
/*                                                                            */
/******************************************************************************/

DenseMatrix::DenseMatrix() {
  this->size = 0;
  this->num_threads = 1;
}

DenseMatrix::DenseMatrix(size_t size, size_t num_threads, bool simd, bool cache,
                         float sparcity) {
  this->size = size;
  this->num_threads = num_threads;
  this->simd = simd;
  this->cache = cache;

  std::uniform_int_distribution<size_t> idistribution(0, this->size - 1);

  // fill in with random
  this->values.resize(this->size);
  for (size_t i = 0; i < this->size; i++) {
    this->values[i].resize(this->size);
    for (size_t j = 0; j < this->size; j++) {
      this->values[i][j] = rand_f();
    }
  }

  // fill in zeros
  size_t num_zero = this->size * this->size * sparcity;
  for (size_t i = 0; i < num_zero; i++) {
    size_t row, col;
    do {
      row = idistribution(generator);
      col = idistribution(generator);
    } while (this->values[row][col] == 0);
    this->values[row][col] = 0;
  }
}

DenseMatrix::DenseMatrix(SparseMatrix &other) {
  this->size = other.size;
  this->num_threads = 1;
  this->simd = false;
  this->cache = false;

  this->values.resize(this->size);
  for (size_t i = 0; i < this->size; i++) {
    this->values[i].resize(this->size);

    std::vector<size_t>::iterator other_col_beg = other.col_index.begin();
    other_col_beg += other.row_index[i];
    std::vector<size_t>::iterator other_col_end = other.col_index.begin();
    other_col_end += other.row_index[i + 1];

    for (size_t j = 0; j < this->size; j++) {
      std::vector<size_t>::iterator itr =
          std::find(other_col_beg, other_col_end, j);
      if (itr != other.col_index.end()) {
        size_t index = other.col_index.begin() - itr;
        this->values[i][j] = other.values[index];
      } else {
        this->values[i][j] = 0;
      }
    }
  }
}

size_t DenseMatrix::num_cols() const { return this->size; }
size_t DenseMatrix::num_rows() const { return this->size; }

DenseMatrix DenseMatrix::operator*(DenseMatrix &other) {
  const MatrixMultFunc funcs[] = {
      MatrixMultRegion,
      MatrixMultRegion_SIMD,
      MatrixMultRegion_Cache,
      MatrixMultRegion_SIMD_Cache,
  };
  int index = 0;
  if (this->simd) {
    index += 1;
  }
  if (this->cache) {
    index += 2;
  }
  const MatrixMultFunc func = funcs[index];

  DenseMatrix result = DenseMatrix(this->size);

  std::vector<std::thread> threads;
  for (size_t i = 0; i < this->num_threads; i++) {
    size_t beg = (i * this->num_rows()) / this->num_threads;
    size_t end = ((i + 1) * this->num_rows()) / this->num_threads;
    threads.emplace_back(func, std::ref(*this), std::ref(other),
                         std::ref(result), beg, end);
  }

  for (std::thread &thread : threads) {
    thread.join();
  }

  return result;
}

DenseMatrix DenseMatrix::operator*(SparseMatrix &other) {
  // let sparse matrix handle multiplication
  SparseMatrix result = other * (*this);
  DenseMatrix actual = DenseMatrix(result);
  return actual;
}

float &DenseMatrix::get(size_t row, size_t col) {
  if (row >= this->size || col >= this->size) {
    throw std::out_of_range("Index into matrix out of range");
  }
  return this->values[row][col];
}

void DenseMatrix::set(size_t row, size_t col, float val) {
  if (row >= this->size || col >= this->size) {
    throw std::out_of_range("Index into matrix out of range");
  }
  this->values[row][col] = val;
}

void DenseMatrix::transpose() {
  for (size_t i = 0; i < this->size; i++) {
    for (size_t j = 0; j < this->size; j++) {
      float temp = this->values[i][j];
      this->values[i][j] = this->values[j][i];
      this->values[j][i] = temp;
    }
  }
}

std::ostream &operator<<(std::ostream &os, DenseMatrix &obj) {
  for (size_t i = 0; i < obj.num_rows(); i++) {
    for (size_t j = 0; j < obj.num_cols(); j++) {
      os << obj.get(i, j) << " ";
    }
    os << std::endl;
  }
  return os;
}

/******************************************************************************/
/*                                                                            */
/*                               SPARSE MATRIX                                */
/*                                                                            */
/******************************************************************************/

SparseMatrix::SparseMatrix() { this->zero = 0; }

SparseMatrix::SparseMatrix(DenseMatrix &other) {
  this->zero = 0;
  this->size = other.size;

  // create sparce matrix     https://en.wikipedia.org/wiki/Sparse_matrix
  this->row_index.push_back(0);
  for (size_t i = 0; i < other.size; i++) {
    for (size_t j = 0; j < other.size; j++) {
      if (other.values[i][j] == 0) {
        continue;
      }

      this->values.push_back(other.values[i][j]);
      this->col_index.push_back(j);
    }
    this->row_index.push_back(this->col_index.size());
  }
}

size_t SparseMatrix::num_cols() const { return this->size; }
size_t SparseMatrix::num_rows() const { return this->size; }

SparseMatrix SparseMatrix::operator*(DenseMatrix &other) {
  SparseMatrix sp = SparseMatrix(other);
  return (*this) * sp;
}

SparseMatrix SparseMatrix::operator*(SparseMatrix &other) {
  SparseMatrix result = SparseMatrix();
  result.zero = 0;
  result.size = this->size;
  result.row_index.push_back(0);

  for (size_t i = 0; i < this->size; i++) {
    std::vector<float> values(this->size, 0.0F);

    for (size_t j = this->row_index[i]; j < this->row_index[i + 1]; j++) {
      const size_t x = this->col_index[j];
      const float this_val = this->values[j];

      for (size_t k = other.row_index[x]; k < other.row_index[x + 1]; k++) {
        const size_t y = other.col_index[k];
        const float other_val = other.values[k];
        values[y] += this_val * other_val;
      }
    }

    for (size_t j = 0; j < values.size(); j++) {
      if (values[j] != 0) {
        result.values.push_back(values[j]);
        result.col_index.push_back(j);
      }
    }

    result.row_index.push_back(result.col_index.size());
  }

  return result;
}

float &SparseMatrix::get(size_t row, size_t col) {
  if (row >= this->size || col >= this->size) {
    throw std::out_of_range("Index into matrix out of range");
  }

  const size_t beg = this->row_index[row];
  const size_t end = this->row_index[row + 1];
  for (size_t i = beg; i < end; i++) {
    if (this->col_index[i] == col) {
      return this->values[i];
    }
  }

  this->zero = 0;
  return this->zero;
}

std::ostream &operator<<(std::ostream &os, SparseMatrix &obj) {
  for (size_t i = 0; i < obj.num_rows(); i++) {
    for (size_t j = 0; j < obj.num_cols(); j++) {
      os << obj.get(i, j) << " ";
    }
    os << std::endl;
  }
  return os;
}
