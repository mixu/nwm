
extern "C"
{
#include "list.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(FirstTestGroup) {

};


TEST(FirstTestGroup, CanCreateList) {

  const char *a = "A";

  List* items = List_list((void*) a);

  LONGS_EQUAL(1, List_length(items));
  /*

  List *second = List_push(items, (void*) "B");

  LONGS_EQUAL(2, List_length(items));

  List_remove(items, second);

  LONGS_EQUAL(1, List_length(items));

  */

  List_free(items);
}
