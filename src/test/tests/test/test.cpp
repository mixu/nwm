
extern "C"
{
#include "example.h"
}

#include "CppUTest/TestHarness.h"

TEST_GROUP(FirstTestGroup) {

};

TEST(FirstTestGroup, FirstTest) {
  LONGS_EQUAL(3, Do_Something());
//   FAIL("Fail me!");
}
