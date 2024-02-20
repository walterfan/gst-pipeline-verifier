#include "gst_util.h"
#include "file_util.h"
#include "string_util.h"
#include "pipeline_builder.h"
#include <regex>
#include "logger.h"

using namespace std;

namespace wfan {

PipelineBuilder::PipelineBuilder(const std::string& config_file)
: m_config_file(config_file) {
    
    yaml_to_str_vec_map(std::string(m_config_file), "pipelines", m_pipeline_config);
}

int PipelineBuilder::init(int argc, char *argv[], const cmd_args_t& args) {
    gst_init(&argc, &argv);
    
    if (m_pipeline_config.empty()) {
        ELOG("PipelineBuilder init not read pipeline yaml: {} ", m_config_file);
        return -1;
    }

    m_pipeline_name = get_option(args, "-p");
    if (m_pipeline_name.empty()) {
        m_pipeline_name = "pipeline_test";
    }

    auto it = m_pipeline_config.find(std::string(m_pipeline_name));
    if (it == m_pipeline_config.end()) {
        ELOG("Do not found pipeline {}",  m_pipeline_name);
        return -1;
    }

    m_pipeline = gst_pipeline_new(m_pipeline_name.c_str());
    m_bus = gst_element_get_bus(m_pipeline);
    gst_bus_add_watch(m_bus, on_bus_msg, this);
    //use the global-default main context, and not running
    m_loop = g_main_loop_new(NULL, FALSE);

    m_pipelie_config = std::make_shared<PipelineConfig>(it->first, it->second);
    DLOG("Init success for pipeline {}",  m_pipeline_name);
    return 0;
}

void PipelineBuilder::list_pipelines(const std::string& pipeline_name) {
    auto it = m_pipeline_config.begin();
    for (;it != m_pipeline_config.end(); ++it) {
        if (!pipeline_name.empty()) {
            if (pipeline_name != it->first) {
                continue;
            }
        }
        cout << it->first << ": " << endl;
        auto& vec = it->second;
        auto cnt = vec.size();
        for(int i = 0; i < cnt; ++i) {
            if (i > 0) {
                cout << "  ! ";
            } else {
                cout << "  gst-launch-1.0 -ev ";
            } 
            
            cout << vec[i] ;
            if (i < cnt -1) {
                cout << " \\" << endl;
            } else {
                cout << endl;
            }
        }
    }
}

int PipelineBuilder::build() {
    int failed_count = 0;
    
    for(auto& ele_cfg_ptr: m_pipelie_config->m_elements_config) {        
        auto created_ele_ptr = create_element(ele_cfg_ptr->m_factory, ele_cfg_ptr->m_name);
        if (!created_ele_ptr) {
            failed_count ++;
        }
    }
    
    if (failed_count > 0) {
        ELOG("create elements, failed_count={}", failed_count);
        return -1;
    } 
    DLOG("created elements");

    for(auto& ele_cfg_ptr: m_pipelie_config->m_elements_config) {
        auto add_ret = add_element(ele_cfg_ptr->m_name);
        if (!add_ret) {
            failed_count ++;
        }
    }
        
    if (failed_count > 0) {
        ELOG("create elements, failed_count={}", failed_count);
        return -1;
    } 
    DLOG("added elements");  
    
    auto ret = link_elements();
    
    if (!ret) {
        ELOG("link elements failed"); 
        return -1;
    }
    DLOG("linked elements"); 
    return 0;
}


int PipelineBuilder::clean() {
    //don't forget gst_object_unref (GST_OBJECT (element)) if not use it
    unlink_elements();
    for(auto& ele_cfg_ptr: m_pipelie_config->m_elements_config) {
        del_element(ele_cfg_ptr->m_name);
    }
    return 0;
}

int PipelineBuilder::start() {
    std::string dot_file = "test_pipeline";
    //set environment variable, such as export GST_DEBUG_DUMP_DOT_DIR=/tmp
    GST_DEBUG_BIN_TO_DOT_FILE(GST_BIN_CAST(m_pipeline), GST_DEBUG_GRAPH_SHOW_VERBOSE, dot_file.c_str());

    DLOG("start playing...");
    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    g_main_loop_run(m_loop);
    return 0;
}

int PipelineBuilder::stop() {
    DLOG("stop playing...");
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
    return 0;
}

GstElement* PipelineBuilder::create_element(const std::string& factory, 
    const std::string& name) {
    
    GstElement* element = gst_element_factory_make (factory.c_str(), name.c_str());
    if (!element) {
        DLOG("create failed for element {} name={}",  factory, name);
        return nullptr;
    }
    DLOG("create succeed for element {}, name={}", factory, name);
    m_elements.emplace(std::make_pair(name, element));
    return element;
}

GstElement* PipelineBuilder::get_element(const std::string& name) {
    auto it = m_elements.find(name);
    if (it != m_elements.end()) {
        return it->second;
    }
    return nullptr;
}

bool PipelineBuilder::add_element(const std::string& name) {
    auto it = m_elements.find(name);
    if (it == m_elements.end()) {
        return false;
    }

    auto ret = gst_bin_add(GST_BIN(m_pipeline), it->second);
    if(ret) {     
        DLOG("add succeed for element {}",  it->first);
        //set element prop
        auto ele_cfg_ptr = m_pipelie_config->get_element_config(name);
        if (ele_cfg_ptr) {
            for(auto& kv: ele_cfg_ptr->m_props) {
                auto key = trim(kv.first, " ,\"");
                auto value = trim(kv.second, " ,\"");
                DLOG("set element prop: {}={}",  key, value);

                if(key == "caps") {
                    GstCaps* caps = gst_caps_from_string(value.c_str());
                    g_object_set(G_OBJECT(it->second), key.c_str(), caps, nullptr);
                } else if(is_number(value)) {
                    g_object_set(it->second, key.c_str(), str_to_num<int32_t>(value), NULL);    
                } else {
                    std::string lower_value = str_tolower(value);
                    if(lower_value == "true") {
                        g_object_set(it->second, key.c_str(), 1, NULL);
                    } else if(lower_value == "false") {
                        g_object_set(it->second, key.c_str(), 0, NULL);
                    } else {
                        g_object_set(it->second, key.c_str(), value.c_str(), NULL);
                    }
                    
                }
                
            }
        }
        return true;                               
    } 
    DLOG("add failed for element {}",  it->first);   
    return false;
}

bool PipelineBuilder::del_element(const std::string& name) {
    auto it = m_elements.find(name);
    if (it == m_elements.end()) {
        return false;
    }
    GstElement* e = it->second;
    GstStateChangeReturn s = gst_element_set_state(e, GST_STATE_NULL);                                   
    if(s == GST_STATE_CHANGE_SUCCESS) {                    
        if(gst_bin_remove(GST_BIN(m_pipeline), (e))) {             
            DLOG("remove/delete succeed for element {}",  name);
            m_elements.erase(it);
            //TODO: need unrefe here?
            //gst_object_unref(GST_OBJECT(e));
            //e = nullptr;
            return true;
        } else {                                           
            DLOG("remove failed for element {}",  name);           
        }                                                  
    } else {                                               
        DLOG("remove elem failed for set state null: {}",  name);  
    }                                                      

    return false;
}

bool PipelineBuilder::link_elements() {
    auto& elements_configs = m_pipelie_config->m_elements_config;
    auto it = elements_configs.begin();
    int failed_count = 0;
    while( it != elements_configs.end()) {
        GstElement* e1 = get_element((*it)->m_name);
        //for decodebin, cannot link directly
        if ((*it)->m_factory == "decodebin") {
            g_signal_connect(e1, "pad-added", G_CALLBACK(on_pad_added), this);
        }

        ++it;
        if (it == elements_configs.end()) {
            break;
        }
        
        GstElement* e2 = get_element((*it)->m_name);
        if (e1 != nullptr && e2 != nullptr) {      
            gchar* e1n = gst_element_get_name(e1);
            gchar* e2n = gst_element_get_name(e2);
            auto link_ret = gst_element_link(e1, e2);
            if(link_ret) {                                 
                DLOG("link succed for {} and {}", e1n, e2n);
            } else {
                DLOG("link failed for {} and {}", e1n, e2n);
                failed_count ++;
            }
            g_free(e1n);        
            g_free(e2n);                                         
        }
    }
    if (failed_count == 0) {
        return true;
    } else {
        return false;
    }
    
}

bool PipelineBuilder::unlink_elements() {
    auto& pipe_configs = m_pipelie_config->m_elements_config;
    auto it = pipe_configs.begin();
    
    while( it != pipe_configs.end()) {
        GstElement* e1 = get_element((*it)->m_name);
        ++it;
        if (it == pipe_configs.end()) {
            break;
        }
        GstElement* e2 = get_element((*it)->m_name);
        if (e1 != nullptr && e2 != nullptr) {      
                gst_element_unlink(e1, e2);
                                     
                gchar* e1n = gst_element_get_name((e1));
                gchar* e2n = gst_element_get_name((e2));
                DLOG("unlink successfully for {} and {}", e1n, e2n);
                g_free(e1n);                            
                g_free(e2n); 
                                                    
        }
    }

    return false;
}


gboolean PipelineBuilder::on_bus_msg(GstBus* bus, GstMessage* msg, gpointer data) {
    
    DLOG("on_bus_msg: type={}, type name={}, source={}", 
        (int)GST_MESSAGE_TYPE(msg),
        GST_MESSAGE_TYPE_NAME(msg),
        GST_OBJECT_NAME(msg->src));

    PipelineBuilder* builder = (PipelineBuilder*)data;

    switch(GST_MESSAGE_TYPE(msg)) {
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
        default:
            break;
    }
    return TRUE;
}


void PipelineBuilder::on_pad_added(GstElement* element, GstPad* pad, gpointer data) {
    DLOG("on_pad_added:");
}

void PipelineBuilder::on_bus_msg_eos() {
    ILOG("on_bus_msg_eos");
    g_main_loop_quit(m_loop);
}

void PipelineBuilder::on_bus_msg_error(GstMessage* msg) {
    ELOG("on_bus_msg_error:");
    gchar* debug = nullptr;
    GError* error = nullptr;
    gst_message_parse_error(msg, &error, &debug);
    ELOG("there is error {} for {}", error->message,  GST_OBJECT_NAME(msg->src));
    if (debug) {
        ELOG("error details: {}", debug);
    }

    g_free(debug);
    g_error_free(error);
 
    g_main_loop_quit(m_loop);
}

void PipelineBuilder::on_bus_msg_warning(GstMessage* msg) {
    DLOG("on_bus_msg_warning:");
}

void PipelineBuilder::on_state_changed(GstMessage* msg) {

    GstState old_state, new_state, pending_state;
    gst_message_parse_state_changed (msg, &old_state, &new_state, &pending_state);

    DLOG("--> state changed from {} to {} for {}" 
               , gst_element_state_get_name (old_state)
               , gst_element_state_get_name(new_state)
               , GST_OBJECT_NAME(msg->src));
 
}

void PipelineBuilder::on_stream_started(GstMessage* msg) {
    DLOG("on_stream_started:");
}

} //namespace wfan