#pragma once

#include <cstddef>
#include <ostream>
#include <vector>

class DenseMatrix;
class SparseMatrix;

/******************************************************************************/
/*                                                                            */
/*                               DENSE MATRIX                                 */
/*                                                                            */
/******************************************************************************/

class DenseMatrix {
public:
  DenseMatrix();
  DenseMatrix(size_t size, size_t num_threads = 1, bool simd = false,
              bool cache = false, float sparcity = 0);
  DenseMatrix(SparseMatrix &other);

  size_t num_rows() const;
  size_t num_cols() const;

  DenseMatrix operator*(DenseMatrix &other);
  SparseMatrix operator*(SparseMatrix &other);

  float &get(size_t row, size_t col);
  void set(size_t row, size_t col, float val);

  void transpose();

  friend class SparseMatrix;

private:
  size_t size;
  size_t num_threads;
  bool simd;
  bool cache;
  std::vector<std::vector<float>> values;
};

std::ostream &operator<<(std::ostream &os, DenseMatrix &obj);

/******************************************************************************/
/*                                                                            */
/*                               SPARSE MATRIX                                */
/*                                                                            */
/******************************************************************************/

class SparseMatrix {
public:
  SparseMatrix();
  SparseMatrix(DenseMatrix &other);

  size_t num_rows() const;
  size_t num_cols() const;

  SparseMatrix operator*(DenseMatrix &other);
  SparseMatrix operator*(SparseMatrix &other);

  float &get(size_t row, size_t col);

  friend class DenseMatrix;

private:
  float zero;
  size_t size;
  std::vector<float> values;
  std::vector<size_t> col_index;
  std::vector<size_t> row_index;
};

std::ostream &operator<<(std::ostream &os, SparseMatrix &obj);
