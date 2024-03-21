#include <gtest/gtest.h>
#include <string>
#include <iostream>
#include <stdint.h>
#include "gst_util.h"

using namespace std;
using namespace testing;
using namespace hefei;


TEST(GstUtilTest, get_pad_name_from_link_tag)
{

    gst_init (NULL, NULL);

    GstElement *element = gst_element_factory_make ("fakesrc", "source");

    ASSERT_TRUE(element);

    gst_object_unref (GST_OBJECT (element));

}
