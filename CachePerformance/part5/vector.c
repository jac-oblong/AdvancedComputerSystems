/* clang-format off */

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>


#define PAGE_SIZE 4096


typedef struct Page Page;
struct Page {
    int contents[(PAGE_SIZE / sizeof(int))];
};

const int PAGE_CONTENTS_SIZE = PAGE_SIZE / sizeof(int);


Page **gen_vector(int pages) {
    Page **vector;

    vector = (Page **)malloc(pages * sizeof(Page *));

    srand((unsigned) time(NULL));

    for (int i = 0; i < pages; i++) {
        // initialize page and fill with random values
        vector[i] = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
        for (int j = 0; j < PAGE_CONTENTS_SIZE; j++) {
            vector[i]->contents[j] = rand();
        }
    }

    return vector;
}

void miss_multiplication(Page **vector, int pages) {
    volatile int product = 0;

    for (int i = 0; i < PAGE_CONTENTS_SIZE; i++) {
        for (int j = 0; j < pages; j++) {
            product *= vector[j]->contents[i];
        }
    }
}

void hit_multiplication(Page **vector, int pages) {
    volatile int product = 0;

    for (int i = 0; i < pages; i++) {
        for (int j = 0; j < PAGE_CONTENTS_SIZE; j++) {
            product *= vector[i]->contents[j];
        }
    }
}

int main(int argc, char **argv) {
    int pages = 0, miss = 0;

    // starting at 1 to avoid executable name
    for (int i = 1; i < argc;) {
        if (strcmp(argv[i], "--pages") == 0) {
            pages = atoi(argv[i + 1]);
            i += 2;
        } else if (strcmp(argv[i], "--miss") == 0) {
            miss = 1;
            i += 1;
        } else {
            exit(1);
        }
    }

    Page **vector = gen_vector(pages);
    if (miss) {
        miss_multiplication(vector, pages);
    } else {
        hit_multiplication(vector, pages);
    }
}
