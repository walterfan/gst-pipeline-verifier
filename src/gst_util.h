#pragma once
#include <ctime>
#include <iostream>
#include <vector>
#include <string>
#include <string_view>
#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

namespace wfan {


#define VIDEO_PAD_NAME "video"
#define AUDIO_PAD_NAME "audio"
#define SRC_PAD_NAME   "src"
#define SINK_PAD_NAME  "sink"

#define TIME_FMT "%Y%m%d_%H%M%S"

GstCaps* get_default_caps();

gboolean link_elements_with_filter (GstElement *element1, GstElement *element2);

gboolean print_field (GQuark field, const GValue * value, gpointer pfx);

void print_caps (const GstCaps * caps, const gchar * pfx);

void print_pad_capabilities (GstPad *pad, gchar *pad_name);

void check_pads(GstElement *element) ;

void log_handler(const gchar *log_domain,
                 GLogLevelFlags log_level,
                 const gchar *message,
                 gpointer user_data);

} //namespace wfan