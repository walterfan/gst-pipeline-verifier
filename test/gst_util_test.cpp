#include <glib.h>
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

static void gst_log_handler(const gchar *log_domain, GLogLevelFlags log_level, 
const gchar *message, gpointer user_data)
{
    // Custom logging logic, for example, printing log messages to stdout
    g_print("[%s] %s\n", log_domain, message);
}

TEST(GstUtilTest, g_log_set_handler)
{
    // Set the custom log handler globally
    g_log_set_handler("gst_plugin", 
        (GLogLevelFlags)(G_LOG_LEVEL_MASK | G_LOG_FLAG_FATAL | G_LOG_FLAG_RECURSION),
        gst_log_handler, NULL);

    // Trigger some log messages
    g_message("This is a message");
    g_warning("This is a warning");
    g_critical("This is a critical error");
}
