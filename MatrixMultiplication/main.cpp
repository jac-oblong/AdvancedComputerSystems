#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdexcept>

#include "matrixmult.h"

/******************************************************************************/
/*                                                                            */
/*                                   TYPES                                    */
/*                                                                            */
/******************************************************************************/

typedef struct {
  size_t size;

  size_t threads;
  bool simd;
  bool cache;

  bool sparse;
  float sparcity1;
  float sparcity2;
} CLI;

/******************************************************************************/
/*                                                                            */
/*                              HELPER FUNCTIONS                              */
/*                                                                            */
/******************************************************************************/

void print_help(char *exe_name) {
  std::cout << "USAGE: " << exe_name
            << "[-t <n>] [-sc] [--sparse] [-s1 <%>] [-s2 <%>] <size>"
            << std::endl;
  std::cout << "       -t <n>    enable 'n' threads" << std::endl;
  std::cout << "       -s        enable simd" << std::endl;
  std::cout << "       -c        enable cache optimization" << std::endl;
  std::cout << "       --sparse  enable sparse matrix compression" << std::endl;
  std::cout << "       -s1 <%>   sparcity percentage for matrix 1" << std::endl;
  std::cout << "       -s1 <%>   sparcity percentage for matrix 2" << std::endl;
  std::cout << "       <size>    size of square matrix" << std::endl;
}

void parse_cli(int argc, char **argv, CLI &cli) {
  if (argc < 2) {
    throw std::invalid_argument("Wrong number of arguments");
  }

  for (int i = 1; i < argc; i++) {
    if (std::strncmp("-t", argv[i], 2) == 0) {
      char *ret;
      cli.threads = strtoul(argv[++i], &ret, 10);
      if (cli.threads == 0 || *ret != '\0') {
        throw std::invalid_argument("INVALID THREAD COUNT");
      }

    } else if (strncmp("-s1", argv[i], 3) == 0) {
      char *ret;
      cli.sparcity1 = strtof(argv[++i], &ret) / 100.0F;
      if (*ret != '\0') {
        throw std::invalid_argument("INVALID SPARCITY PERCENTAGE");
      }

    } else if (strncmp("-s2", argv[i], 3) == 0) {
      char *ret;
      cli.sparcity2 = strtof(argv[++i], &ret) / 100.0F;
      if (*ret != '\0') {
        throw std::invalid_argument("INVALID SPARCITY PERCENTAGE");
      }

    } else if (strncmp("--sparse", argv[i], 8) == 0) {
      cli.sparse = true;

    } else if (strncmp("-", argv[i], 1) == 0) {
      cli.simd = (strchr(argv[i], 's') != NULL);
      cli.cache = (strchr(argv[i], 'c') != NULL);

      for (size_t j = 1; j < strlen(argv[i]); j++) {
        switch (argv[i][j]) {
        case 's':
        case 'c':
          break;
        default:
          throw std::invalid_argument("UNRECOGNIZED OPTION");
        }
      }

    } else {
      char *ret;
      cli.size = strtoul(argv[i], &ret, 10);
      if (cli.size == 0 || *ret != '\0') {
        throw std::invalid_argument("INVALID MATRIX SIZE");
      }
    }
  }
}

/******************************************************************************/
/*                                                                            */
/*                                    MAIN                                    */
/*                                                                            */
/******************************************************************************/

int main(int argc, char **argv) {
  CLI cli = {.threads = 1};
  try {
    parse_cli(argc, argv, cli);
  } catch (const std::invalid_argument &e) {
    print_help(argv[0]);
    throw e;
  }

  DenseMatrix a =
      DenseMatrix(cli.size, cli.threads, cli.simd, cli.cache, cli.sparcity1);
  DenseMatrix b =
      DenseMatrix(cli.size, cli.threads, cli.simd, cli.cache, cli.sparcity2);

#ifdef DEBUG
  std::cout << a << std::endl << std::endl << std::endl;
  std::cout << b << std::endl << std::endl << std::endl;
#endif

  if (cli.sparse && (cli.sparcity1 != 0 || cli.sparcity2 != 0)) {
    SparseMatrix sa, sb, sc;

    if (cli.sparcity1 != 0) {
      sa = SparseMatrix(a);
    }

    if (cli.sparcity2 != 0) {
      sb = SparseMatrix(b);
    }

    if (cli.sparcity1 != 0 && cli.sparcity2 == 0) {
      sc = sa * b;
    } else if (cli.sparcity1 == 0 && cli.sparcity2 != 0) {
      sc = a * sb;
    } else {
      sc = sa * sb;
    }

#ifdef DEBUG
    std::cout << sc << std::endl;
#endif

  } else {
    DenseMatrix c = a * b;

#ifdef DEBUG
    std::cout << c << std::endl;
#endif
  }
}
