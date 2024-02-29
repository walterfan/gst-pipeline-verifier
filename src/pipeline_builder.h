#pragma once

#include <gst/gst.h>
#include <glib.h>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include "string_util.h"
#include "file_util.h"
#include "pipeline_config.h"

namespace wfan {

class PipelineBuilder {
public:
    PipelineBuilder();
    void list_pipelines(const std::string& pipeline_name);

    int init(int argc, char *argv[], const cmd_args_t& args);
    int clean();
    int build();
    int start();
    int stop();

    int read_config_file(const char* szFile);
    int read_all_config_files(const char* szFolder);
private:
    
    GstElement* create_element(const std::string& factory, const std::string& name);
    GstElement* get_element(const std::string& name);
    
    

    bool add_element(const std::string& name);
    bool del_element(const std::string& name);

    bool link_elements();
    bool unlink_elements();

    static gboolean on_bus_msg(GstBus* bus, GstMessage* msg, gpointer data);
    static void on_src_pad_added(GstElement* element, GstPad* pad, gpointer data);

    void on_bus_msg_eos();
    void on_bus_msg_error(GstMessage* msg);
    void on_bus_msg_warning(GstMessage* msg);
    void on_state_changed(GstMessage* msg);
    void on_stream_started(GstMessage* msg);
    void on_stream_status(GstMessage* msg);
    void on_bus_msg_qos(GstMessage* msg);
    void on_bus_msg_buffering_stats(GstMessage* msg);

    std::string m_config_file;
    config_map_t m_general_config;
    pipeline_config_t m_pipeline_config;
    std::map<std::string, GstElement*> m_elements;
    GstBus* m_bus;
    GMainLoop* m_loop;
    GstElement* m_pipeline;
    std::string m_pipeline_name;
    std::shared_ptr<PipelineConfig> m_pipelie_config;

    std::vector<std::pair<GstElement*, GstElement*>> m_linked_elements;
    std::vector<std::pair<GstPad*, GstPad*>> m_linked_pads;
};

} //namespace wfan