#include "gst_util.h"
#include "file_util.h"
#include "string_util.h"
#include "pipeline_builder.h"
#include "logger.h"
#include <vector>
#include "yaml-cpp/yaml.h"

using namespace std;

namespace hefei
{

    PipelineBuilder::PipelineBuilder(AppConfig &appConfig)
        : m_build_progress(BuildProgress::NOT_STARTED), m_app_config(appConfig)
    {
    }

    PipelineBuilder::~PipelineBuilder()
    {
    }

    int PipelineBuilder::init(const std::string &pipeline_name, const std::string &variables)
    {
        auto &pipelines_config = m_app_config.get_pipelines_config();

        m_pipeline_name = pipeline_name;

        if (pipelines_config.empty())
        {
            ELOG("not found pipeline in config file");
            return -1;
        }
        auto& pipelines_map = pipelines_config.get_pipelines();
        auto it = pipelines_map.find(std::string(m_pipeline_name));
        if (it == pipelines_map.end())
        {
            ELOG("not found pipeline {}", m_pipeline_name);
            return -1;
        }
        m_pipeline_config = it->second;
        m_pipeline_config->init(variables);
        m_pipeline_config->check_elements_name();

        m_pipeline = gst_pipeline_new(m_pipeline_name.c_str());
        m_bus = gst_element_get_bus(m_pipeline);
        gst_bus_add_watch(m_bus, on_bus_msg, this);
        // use the global-default main context, and not running
        m_loop = g_main_loop_new(NULL, FALSE);

        DLOG("Init success for pipeline {}", m_pipeline_name);
        return 0;
    }

    int PipelineBuilder::build()
    {
        int failed_count = 0;

        for (auto &ele_cfg_ptr : m_pipeline_config->m_elements_config)
        {
            auto created_ele_ptr = create_element(ele_cfg_ptr->m_factory, ele_cfg_ptr->m_name);
            if (!created_ele_ptr)
            {
                failed_count++;
            }
        }

        if (failed_count > 0)
        {
            ELOG("create elements, failed_count={}", failed_count);
            return -1;
        }
        DLOG("created elements");

        for (auto &ele_cfg_ptr : m_pipeline_config->m_elements_config)
        {
            auto add_ret = add_element(ele_cfg_ptr->m_name);
            if (!add_ret)
            {
                failed_count++;
            }
        }

        if (failed_count > 0)
        {
            ELOG("create elements, failed_count={}", failed_count);
            return -1;
        }
        DLOG("added elements");

        auto ret = link_elements();

        if (!ret)
        {
            ELOG("link elements failed");
            return -1;
        }
        DLOG("linked elements");
        return 0;
    }

    int PipelineBuilder::clean()
    {
        // don't forget gst_object_unref (GST_OBJECT (element)) if not use it
        unlink_elements();
        for (auto &ele_cfg_ptr : m_pipeline_config->m_elements_config)
        {
            del_element(ele_cfg_ptr->m_name);
        }
        m_pipeline_config->clean();
        return 0;
    }

    int PipelineBuilder::start()
    {
        if (m_app_config.has_probe_config_item(m_pipeline_name))
        {
            add_probe(m_app_config.get_probe_config_item(m_pipeline_name));
        }

        std::string dot_file = m_pipeline_name + "_graph";
        // set environment variable, such as export GST_DEBUG_DUMP_DOT_DIR=/tmp
        GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN_CAST(m_pipeline), GST_DEBUG_GRAPH_SHOW_VERBOSE, dot_file.c_str());

        DLOG("start playing...");
        gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
        g_main_loop_run(m_loop);
        return 0;
    }

    int PipelineBuilder::stop()
    {
        DLOG("stop playing...");
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        return 0;
    }

    GstElement *PipelineBuilder::create_element(const std::string &factory,
                                                const std::string &name)
    {

        GstElement *element = gst_element_factory_make(factory.c_str(), name.c_str());
        if (!element)
        {
            DLOG("create failed for element {} name={}", factory, name);
            return nullptr;
        }
        update_build_progress(BuildProgress::MADE_ELEMENTS);
        DLOG("create succeed for element {}, name={}", factory, name);
        m_elements.emplace(std::make_pair(name, element));
        return element;
    }

    GstElement *PipelineBuilder::get_element(const std::string &name)
    {
        auto it = m_elements.find(name);
        if (it != m_elements.end())
        {
            return it->second;
        }
        return nullptr;
    }

    GstPad *PipelineBuilder::get_static_pad(const std::string &ele_name, const std::string &pad_name)
    {
        GstElement *element = get_element(ele_name);
        if (!element)
        {
            return nullptr;
        }

        return gst_element_get_static_pad(element, pad_name.c_str());
    }

    bool PipelineBuilder::add_element(const std::string &name)
    {
        auto it = m_elements.find(name);
        if (it == m_elements.end())
        {
            return false;
        }

        auto ret = gst_bin_add(GST_BIN(m_pipeline), it->second);
        if (ret)
        {
            DLOG("add succeed for element {}", it->first);
            // set element prop
            auto ele_cfg_ptr = m_pipeline_config->get_element_config(name);
            if (ele_cfg_ptr)
            {
                for (auto &kv : ele_cfg_ptr->m_props)
                {
                    auto key = trim(kv.first, " ,\"");
                    auto value = trim(kv.second, " ,\"");
                    DLOG("set element prop: {}={}", key, value);

                    if (key == "caps")
                    {
                        GstCaps *caps = gst_caps_from_string(value.c_str());
                        g_object_set(G_OBJECT(it->second), key.c_str(), caps, nullptr);
                    }
                    else if (is_number(value))
                    {
                        if (value.find(".") == std::string::npos)
                        {
                            g_object_set(it->second, key.c_str(), str_to_num<int32_t>(value), NULL);
                        }
                        else
                        {
                            g_object_set(it->second, key.c_str(), str_to_num<float>(value), NULL);
                        }
                    }
                    else
                    {
                        std::string lower_value = str_tolower(value);
                        if (lower_value == "true")
                        {
                            g_object_set(it->second, key.c_str(), 1, NULL);
                        }
                        else if (lower_value == "false")
                        {
                            g_object_set(it->second, key.c_str(), 0, NULL);
                        }
                        else
                        {
                            g_object_set(it->second, key.c_str(), value.c_str(), NULL);
                        }
                    }
                }
            }
            return true;
        }
        update_build_progress(BuildProgress::ADDED_ELEMENTS);
        DLOG("add failed for element {}", it->first);
        return false;
    }

    bool PipelineBuilder::del_element(const std::string &name)
    {
        auto it = m_elements.find(name);
        if (it == m_elements.end())
        {
            return false;
        }
        GstElement *e = it->second;
        GstStateChangeReturn s = gst_element_set_state(e, GST_STATE_NULL);
        if (s == GST_STATE_CHANGE_SUCCESS)
        {
            if (gst_bin_remove(GST_BIN(m_pipeline), (e)))
            {
                DLOG("remove/delete succeed for element {}", name);
                m_elements.erase(it);
                // need not unref element here becase gst_bin_remove already unref the elements
                // gst_object_unref(GST_OBJECT(e)); e = nullptr;
                return true;
            }
            else
            {
                ELOG("remove failed for element {}", name);
            }
        }
        else
        {
            ELOG("remove elem failed for set state null: {}", name);
        }

        return false;
    }

    bool PipelineBuilder::link_elements()
    {
        auto &elements_configs = m_pipeline_config->m_elements_config;
        auto it = elements_configs.begin();
        int failed_count = 0;
        while (it != elements_configs.end() && failed_count == 0)
        {
            auto &leftEleCfg = *it;
            GstElement *e1 = get_element(leftEleCfg->m_name);
            // for fork tag, use fork tag's element to link to right element instead of this element
            if (!(leftEleCfg->m_fork_tag.empty()))
            {
                DLOG("should fork another branch from {} to {}", leftEleCfg->m_name, leftEleCfg->m_fork_tag);
                e1 = get_element(leftEleCfg->m_fork_tag);
                leftEleCfg = m_pipeline_config->get_element_config(leftEleCfg->m_fork_tag);
            }

            ++it;
            if (it == elements_configs.end())
            {
                break;
            }
            auto &rightEleCfg = *it;
            GstElement *e2 = get_element(rightEleCfg->m_name);

            if (e1 != nullptr && e2 != nullptr)
            {
                // print_pad_templates_info(leftEleCfg->m_factory.c_str());
                GstPadPresence leftPadPresenceType = get_one_pad_presence(leftEleCfg->m_factory.c_str(), GST_PAD_SRC);
                GstPadPresence rightPadPresenceType = get_one_pad_presence(leftEleCfg->m_factory.c_str(), GST_PAD_SINK);

                DLOG("left {}'s src pad presence type: {}, right {}'s sink pad presence type: {}",
                     leftEleCfg->m_name, (int)leftPadPresenceType,
                     rightEleCfg->m_name, (int)rightPadPresenceType);

                if (leftPadPresenceType == GST_PAD_ALWAYS && rightPadPresenceType == GST_PAD_ALWAYS)
                {
                    bool link_ret = link_two_elements(e1, e2);
                    if (!link_ret)
                        failed_count++;
                    continue;
                }

                if (leftPadPresenceType == GST_PAD_SOMETIMES)
                {
                    g_signal_connect(e1, "pad-added", G_CALLBACK(on_src_pad_added), e2);
                    DLOG("sometime pad does not link directly for element {} and {}",
                         leftEleCfg->m_name, rightEleCfg->m_name);
                    continue;
                }
                // for request pad case, static_pad -> request_pad
                if (!(rightEleCfg->m_link_tag.empty()))
                {
                    // get left element's src pad
                    GstPad *src_pad = gst_element_get_static_pad(e1, SRC_PAD_NAME);
                    if (!src_pad)
                    {
                        src_pad = gst_element_get_static_pad(e1, SRC_PAD_NAME_U);
                    }
                    // get right element's sink pad by link tag
                    auto right_pad_name = get_pad_name_from_link_tag(rightEleCfg->m_link_tag);
                    DLOG("right element's link tag: {}, right_pad_name={}", rightEleCfg->m_link_tag, right_pad_name);
                    GstPad *sink_pad = gst_element_get_request_pad(e2, right_pad_name.c_str());
                    if (!sink_pad)
                    {
                        sink_pad = gst_element_get_static_pad(e2, right_pad_name.c_str());
                    }
                    LINK_GST_PADS(src_pad, sink_pad);
                    m_linked_pads.push_back({src_pad, sink_pad});
                    continue;
                }
                // link two elements
                bool link_ret = link_two_elements(e1, e2);
                if (!link_ret)
                    failed_count++;
            }
        }

        return failed_count == 0 ? true : false;
    }

    bool PipelineBuilder::link_two_elements(GstElement *e1, GstElement *e2)
    {
        bool ret = false;
        gchar *e1n = gst_element_get_name(e1);
        gchar *e2n = gst_element_get_name(e2);
        auto link_ret = gst_element_link(e1, e2);
        if (link_ret)
        {
            DLOG("link succed for {} and {}", e1n, e2n);
            m_linked_elements.push_back({e1, e2});
            ret = true;
        }
        else
        {
            ELOG("link failed for {} and {}", e1n, e2n);
        }
        g_free(e1n);
        g_free(e2n);
        return ret;
    }

    bool PipelineBuilder::unlink_elements()
    {
        bool ret = true;
        for (auto it = m_linked_pads.begin(); it != m_linked_pads.end(); ++it)
        {
            GstPad *p1 = it->first;
            GstPad *p2 = it->second;
            ret = gst_pad_unlink(p1, p2);
            gchar *p1n = gst_pad_get_name(p1);
            gchar *p2n = gst_pad_get_name(p2);
            DLOG("unlink result={} for {} and {}", ret, p1n, p2n);
            g_free(p1n);
            g_free(p2n);
        }

        for (auto it = m_linked_elements.begin(); it != m_linked_elements.end(); ++it)
        {
            GstElement *e1 = it->first;
            GstElement *e2 = it->second;
            if (e1 != nullptr && e2 != nullptr)
            {
                gst_element_unlink(e1, e2);
                gchar *e1n = gst_element_get_name(e1);
                gchar *e2n = gst_element_get_name(e2);
                DLOG("unlink result={} for {} and {}", ret, e1n, e2n);
                g_free(e1n);
                g_free(e2n);
            }
        }

        return ret;
    }

    gboolean PipelineBuilder::on_bus_msg(GstBus *bus, GstMessage *msg, gpointer data)
    {

        TLOG("on_bus_msg: type={}, type name={}, source={}",
             (int)GST_MESSAGE_TYPE(msg),
             GST_MESSAGE_TYPE_NAME(msg),
             GST_OBJECT_NAME(msg->src));

        PipelineBuilder *builder = (PipelineBuilder *)data;

        switch (GST_MESSAGE_TYPE(msg))
        {
        case GST_MESSAGE_EOS:
            builder->on_bus_msg_eos();
            break;
        case GST_MESSAGE_ERROR:
            builder->on_bus_msg_error(msg);
            break;
        case GST_MESSAGE_WARNING:
            builder->on_bus_msg_warning(msg);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            builder->on_state_changed(msg);
            break;
        case GST_MESSAGE_STREAM_START:
            builder->on_stream_started(msg);
            break;
        case GST_MESSAGE_STREAM_STATUS:
            builder->on_stream_status(msg);
            break;
        case GST_MESSAGE_QOS:
            builder->on_bus_msg_qos(msg);
            break;
        case GST_MESSAGE_BUFFERING:
            builder->on_bus_msg_buffering_stats(msg);
            break;
        default:
            break;
        }
        return TRUE;
    }

    void PipelineBuilder::on_src_pad_added(GstElement *element, GstPad *pad, gpointer data)
    {
        gchar *src_pad_name = gst_pad_get_name(pad);
        gchar *left_ele_name = gst_element_get_name(element);
        GstElement *other = (GstElement *)data;
        gchar *right_ele_name = gst_element_get_name(other);
        // TODO: refactoring to link to
        auto ret = gst_element_link(element, other);
        DLOG("A new src pad {} was created for {} link to {}, ret={}",
             src_pad_name, left_ele_name, right_ele_name, ret);

        g_free(src_pad_name);
        g_free(left_ele_name);
        g_free(right_ele_name);
    }

    void PipelineBuilder::on_sink_pad_added(GstElement *element, GstPad *pad, gpointer data)
    {
        gchar *sink_pad_name = gst_pad_get_name(pad);
        gchar *right_ele_name = gst_element_get_name(element);
        GstElement *other = (GstElement *)data;
        gchar *left_ele_name = gst_element_get_name(other);
    
        auto ret = gst_element_link(other, element);
        DLOG("A new sink pad {} was created for {} link to {}, ret={}",
             sink_pad_name, left_ele_name, right_ele_name, ret);

        g_free(sink_pad_name);
        g_free(left_ele_name);
        g_free(right_ele_name);
    }

    void PipelineBuilder::on_bus_msg_eos()
    {
        ILOG("on_bus_msg_eos");
        g_main_loop_quit(m_loop);
    }

    void PipelineBuilder::on_bus_msg_error(GstMessage *msg)
    {
        ELOG("on_bus_msg_error:");
        gchar *debug = nullptr;
        GError *error = nullptr;
        gst_message_parse_error(msg, &error, &debug);
        ELOG("there is error {} for {}", error->message, GST_OBJECT_NAME(msg->src));
        if (debug)
        {
            ELOG("error details: {}", debug);
        }

        g_free(debug);
        g_error_free(error);

        g_main_loop_quit(m_loop);
    }

    void PipelineBuilder::on_bus_msg_warning(GstMessage *msg)
    {
        DLOG("on_bus_msg_warning: {}", GST_OBJECT_NAME(msg->src));
    }

    void PipelineBuilder::on_state_changed(GstMessage *msg)
    {

        GstState old_state, new_state, pending_state;
        gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);

        TLOG("state changed from {} to {} for {}", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state), GST_OBJECT_NAME(msg->src));
    }

    void PipelineBuilder::on_stream_started(GstMessage *msg)
    {
        DLOG("[stats] on_stream_started of {}", GST_OBJECT_NAME(msg->src));
    }
    /*
    GST_STREAM_STATUS_TYPE_CREATE (0) – A new thread need to be created.
    GST_STREAM_STATUS_TYPE_ENTER (1) – a thread entered its loop function
    GST_STREAM_STATUS_TYPE_LEAVE (2) – a thread left its loop function
    GST_STREAM_STATUS_TYPE_DESTROY (3) – a thread is destroyed
    GST_STREAM_STATUS_TYPE_START (8) – a thread is started
    GST_STREAM_STATUS_TYPE_PAUSE (9) – a thread is paused
    GST_STREAM_STATUS_TYPE_STOP (10) – a thread is stopped

    gst_message_parse_stream_status (GstMessage * message,
                                     GstStreamStatusType * type,
                                     GstElement ** owner)
    */
    void PipelineBuilder::on_stream_status(GstMessage *msg)
    {
        GstStreamStatusType statusType;
        GstElement *owner;
        gst_message_parse_stream_status(msg, &statusType, &owner);
        DLOG("[stats] on_stream_status: {} of {}", (int)statusType, GST_OBJECT_NAME(owner));
    }

    /*
    gst_message_parse_qos_stats (GstMessage * message,
                                 GstFormat * format,
                                 guint64 * processed,
                                 guint64 * dropped)

    gst_message_parse_qos_values (GstMessage * message,
                                  gint64 * jitter,
                                  gdouble * proportion,
                                  gint * quality)
    */
    void PipelineBuilder::on_bus_msg_qos(GstMessage *msg)
    {
        gboolean live = false;
        guint64 running_time = 0;
        guint64 stream_time = 0;
        guint64 timestamp = 0;
        guint64 duration = 0;
        gst_message_parse_qos(msg, &live, &running_time, &stream_time, &timestamp, &duration);
        DLOG("[stats] element {}' qos: live={}, running_time={}, stream_time={}, timestamp={}, duration={}",
             GST_OBJECT_NAME(msg->src), live, running_time, stream_time, timestamp, duration);

        // gst_message_parse_qos_stats
        GstFormat format;
        guint64 processed = 0;
        guint64 dropped = 0;
        gst_message_parse_qos_stats(msg, &format, &processed, &dropped);
        DLOG("[stats] element {}' qos stats: format={}, oricessed={}, dropped={}",
             GST_OBJECT_NAME(msg->src), (int)format, processed, dropped);

        // gst_message_parse_qos_values
        gint64 jitter = 0;
        gdouble proportion = 0;
        gint quality = 0;

        gst_message_parse_qos_values(msg, &jitter, &proportion, &quality);
        DLOG("[stats] element {}' qos values: jitter={}, proportion={}, quality={}",
             GST_OBJECT_NAME(msg->src), jitter, proportion, quality);
    }

    /*
    gst_message_parse_buffering_stats (GstMessage * message,
                                       GstBufferingMode * mode,
                                       gint * avg_in,
                                       gint * avg_out,
                                       gint64 * buffering_left)
    */
    void PipelineBuilder::on_bus_msg_buffering_stats(GstMessage *msg)
    {
        GstBufferingMode mode;
        gint avg_in = 0;
        gint avg_out = 0;
        gint64 buffering_left = 0;

        gst_message_parse_buffering_stats(msg, &mode, &avg_in,
                                          &avg_out, &buffering_left);
        DLOG("[stats] element {}' buffering stats: mode={}, avg_in={}, avg_out={}, buffering_left={}",
             GST_OBJECT_NAME(msg->src), (int)mode, avg_in, avg_out, buffering_left);
    }

    GstPadProbeReturn PipelineBuilder::probe_pad_callback(GstPad *pad, GstPadProbeInfo *info, gpointer user_data)
    {

        PipelineBuilder *thiz = (PipelineBuilder *)user_data;
        if (thiz)
        {
            thiz->m_probe_count++;
            if (thiz->m_probe_count)
            {
                if (is_probe_event(info))
                {
                    GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
                    if (event != NULL)
                    {
                        GstEventType type = GST_EVENT_TYPE(event);
                        DLOG("probe callback: {}. received event type={}",
                             thiz->m_probe_count.load(),
                             gst_event_type_get_name(type));
                    }
                }
                else if (is_probe_query(info))
                {
                    GstQuery *query = GST_PAD_PROBE_INFO_QUERY(info);
                    if (query != NULL)
                    {
                        GstQueryType type = GST_QUERY_TYPE(query);
                        DLOG("probe callback: {}. received query type={}",
                             thiz->m_probe_count.load(),
                             gst_query_type_get_name(type));
                    }
                }
                else if (is_probe_buffer(info))
                {
                    GstBuffer *buf = (GstBuffer *)info->data;
                    GstMapInfo in_map_info;
                    memset(&in_map_info, 0, sizeof(in_map_info));

                    if (gst_buffer_map(buf, &in_map_info, GST_MAP_READ))
                    {
                        std::string hexstr = bytesToHex(in_map_info.data, in_map_info.size);
                        DLOG("probe callback: {}. received type=buffer size={}, content={}",
                             thiz->m_probe_count.load(), 
                             format_large_number(in_map_info.size),
                             get_str_summary(hexstr, 128, 128));

                        return GST_PAD_PROBE_REMOVE;
                    }
                }
                else
                {
                    DLOG("probe callback: {}. pad name={}, received type={}({})",
                         thiz->m_probe_count.load(), GST_PAD_NAME(pad),
                         (int)info->type, int_to_binary(info->type));
                    return GST_PAD_PROBE_REMOVE;
                }
            }
        }

        return GST_PAD_PROBE_OK;
    }

    int PipelineBuilder::add_probe(const ProbeConfigItem &probe_config_item)
    {
        DLOG("Add probe for {}'{}, pad name={}",
             probe_config_item.probe_pipeline_name,
             probe_config_item.probe_element_name,
             probe_config_item.probe_pad_name);
        GstElement *element = gst_bin_get_by_name((GstBin *)m_pipeline, probe_config_item.probe_element_name.c_str());
        if (!element)
        {
            ELOG("Add probe but not found element {}", probe_config_item.probe_element_name);
            return -1;
        }
        GstPad *probe_pad = gst_element_get_static_pad(element, probe_config_item.probe_pad_name.c_str());
        if (!probe_pad)
        {
            ELOG("Add probe but not found pad {}", probe_config_item.probe_pad_name);
            return -2;
        }

        gulong probe_id = gst_pad_add_probe(probe_pad,
                                            (GstPadProbeType)probe_config_item.probe_type,
                                            probe_pad_callback, this, NULL);
        gst_object_unref(probe_pad);

        DLOG("Add probe probe_id={}", probe_id);
        return 0;
    }

    void PipelineBuilder::update_build_progress(BuildProgress progress)
    {
        m_build_progress = progress;
    }

} // namespace hefei