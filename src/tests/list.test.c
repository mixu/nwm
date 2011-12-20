#include <stdio.h>
#include "list.h"
#include "minunit.h"

int tests_run = 0;
List* items;
const char *a = "A";

/*
  There are two types of items we want to store:
    - integers (Windows)
    - structs (key codes)

  We also want to:
    - add items
    - find items (windows by value)
    - remove items (windows by value)
    - iterate through the items (keycodes)
 */

 List* windows;
 List* keys;

static char * test_list_create() {
  items = List_list((void*) a);

  mu_assert("Length is 1", List_length(items) == 1);

  List_free(items);
  return 0;
}

static char * test_list_push_remove() {
  items = List_list((void*) a);

  List *second = List_push(items, (void*) "B");
  mu_assert("Length is 2", List_length(items) == 2);
  List_remove(items, second);
  mu_assert("Length is 1", List_length(items) == 1);

  List_free(items);
  return 0;
}

static char * all_tests() {
  mu_run_test(test_list_create);
  mu_run_test(test_list_push_remove);
  return 0;
}

int main(int argc, char **argv) {
  char *result = all_tests();
  if (result != 0) {
    printf("\033[41m\t\tFAIL:\033[m %s\n", result);
  } else {
    printf("\033[42m\t\tPASS\t\t\033[m\n");
  }
  printf("%d tests\n", tests_run);

  return result != 0;
}
