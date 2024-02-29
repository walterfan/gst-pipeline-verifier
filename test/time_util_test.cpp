#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "time_util.h"

using namespace std;
using namespace testing;
using namespace wfan;

//using ::testing::Return;
//using ::testing::Mock;


TEST(TimeUtilTest, timepoint_to_str)
{
	cout<<"--- timepoint_to_str --"<<endl;
    const char* time_fmt = "%Y-%m-%dT%H:%M:%S%Z";
	std::string time_str = "2020-02-21T17:32:26CST";
    TimePoint tp = str_to_timepoint(time_str, time_fmt);
    auto ret = timepoint_to_str(tp, time_fmt);
    cout << time_str << "==" << ret << endl;
    ASSERT_EQ(time_str, ret);

}
