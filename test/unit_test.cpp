//#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;
//using ::testing::AtLeast;

int main(int argc, char** argv) {
  testing::GTEST_FLAG(output)="xml:UnitTestResult.xml";
  printf("--- unit test ---\n");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
