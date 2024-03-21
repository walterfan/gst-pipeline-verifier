#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "string_util.h"

using namespace std;
using namespace testing;
using namespace hefei;


TEST(StringUtilTest, startendswith)
{
	std::string str =  "'image/jpeg,width=1920,height=1080,framerate=20/1'";
    bool ret1 = startswith(str, "'");
    bool ret2 = endswith(str, "'");
    cout << "ret1=" << ret1 << ", ret2=" << ret2 << endl;
    ASSERT_TRUE(ret1);
    ASSERT_TRUE(ret2);

}
