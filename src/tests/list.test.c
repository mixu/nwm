#include <stdio.h>
#include "list.h"
#include "minunit.h"

int tests_run = 0;

int foo = 7;

static char * test_foo() {
  mu_assert("error, foo != 7", foo == 7);
  return 0;
}

static char * test_bar() {
  const char *a = "A";

  List* items = List_list((void*) a);

  mu_assert("Length is 1", List_length(items) == 1);

  List *second = List_push(items, (void*) "B");

  mu_assert("Length is 2", List_length(items) == 3);

  List_remove(items, second);

  mu_assert("Length is 1", List_length(items) == 1);


  List_free(items);

  return 0;
}


static char * all_tests() {
  mu_run_test(test_foo);
  mu_run_test(test_bar);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    /* printf("\033[31mFAIL:\033[m %s\n", result); */
    printf("\033[41m\t\tFAIL:\033[m %s\n", result);
  } else {
    /* printf("\033[32mPASS\033[m\n"); */
    printf("\033[42m\t\tPASS\t\t\033[m\n");
  }
  printf("%d tests\n", tests_run);

  return result != 0;
}
