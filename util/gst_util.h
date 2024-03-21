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

#define VIDEO_PAD_NAME  "video"
#define AUDIO_PAD_NAME  "audio"
#define SRC_PAD_NAME    "src"
#define SRC_PAD_NAME_U  "src_%u"
#define SINK_PAD_NAME   "sink"
#define SINK_PAD_NAME_U "sink_%u"


#define LINK_GST_PADS(src, sink)                                            \
    do {                                                                    \
        if(GST_PAD_LINK_FAILED(gst_pad_link((src), (sink)))) {              \
            ELOG("link {} and {} failed", (void*)(src), (void*)(sink));     \
            return false;                                                   \
        }                                                                   \
        DLOG("src={}, sink={}, link succeed", (void*)(src), (void*)(sink)); \
    } while (0)

#define GST_REQUEST_PAD_RELEASE(e, pad)                 \
    do {                                                \
        if ((e) != nullptr && (pad) != nullptr) {       \
            gst_element_release_request_pad((e), (pad));\
            gst_object_unref((pad));                    \
            (e) = nullptr;                              \
            (pad) = nullptr;                            \
            DLOG("gst release request pad succeed");    \
        }                                               \
    } while (0)

GstPadPresence get_one_pad_presence(const char* factory_name, GstPadDirection direction);

GstCaps* get_default_caps();

gboolean link_elements_with_filter (GstElement *element1, GstElement *element2);

gboolean print_field (GQuark field, const GValue * value, gpointer pfx);

void print_caps (const GstCaps * caps, const gchar * pfx);

void print_pad_capabilities (GstPad *pad, gchar *pad_name);

void print_pad_templates_information (GstElementFactory * factory);

void print_pad_templates_info(const char* factory_name);

void check_pads(GstElement *element) ;

void log_handler(const gchar *log_domain,
                 GLogLevelFlags log_level,
                 const gchar *message,
                 gpointer user_data);

std::string get_pad_name_from_link_tag(const std::string& link_tag);

bool is_probe_event(GstPadProbeInfo *info);
bool is_probe_query(GstPadProbeInfo *info);
bool is_probe_buffer(GstPadProbeInfo *info);

} //namespace wfan