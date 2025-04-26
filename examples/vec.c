#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define T int // Improves clarity for generic vector macros
#define _lc_vec_pfx IntVec
#include "../containers/vec.h"

// Comparator for integers (used in sorting)
static int
compare_ints(const int *a, const int *b) {
  return (*a > *b) - (*a < *b); // Safer than (*a - *b) for large values
}

bool
is_even(int *a) {
  return *a % 2 == 0;
}

int
succ(int *a) {
  return *a + 1;
}

int
main(void) {
  srand((unsigned)time(NULL)); // Seed randomness

  IntVec vec = {0};
  IntVec_init(&vec, 16, NULL); // Initial capacity of 16, no cleanup func needed

  // Fill the vector with random integers
  for (size_t i = 0; i < 25; ++i) {
    IntVec_push_back(&vec, rand() % 1000);
  }

  // Display the unsorted vector
  puts("Randomly generated vector of 25 integers");
  puts("[Unsorted Vector]");
  printf("[ ");
  for (size_t i = 0; i < vec.size; ++i) {
    printf("%d ", *IntVec_at(&vec, i));
  }
  puts("]\n");

  // Modify a random element in the vector
  size_t idx = (size_t)(rand() % vec.size);
  int new_val = rand() % 1000;
  IntVec_set(&vec, idx, new_val);

  puts("Let's sort our vector in ascending order");
  // Sort the vector
  IntVec_qsort(&vec, compare_ints);

  // Display the sorted vector
  puts("[Sorted Vector]");
  printf("[ ");
  for (size_t i = 0; i < vec.size; ++i) {
    printf("%d ", *IntVec_at(&vec, i));
  }
  puts("]\n");

  puts("Let's filter out the odd numbers");
  IntVec filtered = IntVec_filter(&vec, is_even);
  puts("[Filtered Vector]");
  printf("[ ");
  for (size_t i = 0; i < filtered.size; ++i) {
    printf("%d ", *IntVec_at(&filtered, i));
  }
  puts("]\n");

  puts("Let's map each element of the vector to the successor");
  IntVec mapped = IntVec_map(&filtered, succ);
  puts("[Mapped Vector]");
  printf("[ ");
  for (size_t i = 0; i < mapped.size; ++i) {
    printf("%d ", *IntVec_at(&mapped, i));
  }
  puts("]\n");

  puts("Now let's sort the elements of the 3 vectors we created by treating "
       "them as a single vector");
  IntVec_sort_chained(compare_ints, &vec, &filtered, &mapped, NULL);
  printf("[First vector] [ ");
  for (size_t i = 0; i < vec.size; ++i) {
    printf("%d ", *IntVec_at(&vec, i));
  }
  puts("]");
  printf("[Filtered vector] [ ");
  for (size_t i = 0; i < filtered.size; ++i) {
    printf("%d ", *IntVec_at(&filtered, i));
  }
  puts("]");
  printf("[Mapped vector] [ ");
  for (size_t i = 0; i < mapped.size; ++i) {
    printf("%d ", *IntVec_at(&mapped, i));
  }
  puts("]");
  // Cleanup
  IntVec_destroy(&vec);
  IntVec_destroy(&filtered);
  IntVec_destroy(&mapped);
  return 0;
}
