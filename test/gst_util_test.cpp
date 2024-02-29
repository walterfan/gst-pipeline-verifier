#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "gst_util.h"

using namespace std;
using namespace testing;
using namespace wfan;


TEST(GstUtilTest, get_pad_name_from_link_tag)
{
	std::string link_tag = "nvstreammux0.sink_0";
    auto pad_name = get_pad_name_from_link_tag(link_tag);
    cout << pad_name << "==" << pad_name << endl;
    ASSERT_EQ(pad_name, "sink_0");

}
