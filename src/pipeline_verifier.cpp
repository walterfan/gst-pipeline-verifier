#include <gst/gst.h>
#include <unistd.h>
#include "logger.h"
#include <vector>
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "pipeline_verifier.h"
#include "pipeline_builder.h"
#include "web_console.h"

using namespace std;

namespace hefei {

#define CHECK_VALUE(msg, value, expect)                      \
do {                                                         \
    if ((value) != expect) {                                 \
        ELOG(msg, value);                                    \
        return value;                                        \
    } else {                                                 \
        ILOG(msg, value);                                    \
    }                                                        \
} while (0)


constexpr auto KEY_DEFAULT_PIPELINE = "default_pipeline";
constexpr auto KEY_PIPELINES = "pipelines";
constexpr auto KEY_PROBES = "probes";
constexpr auto KEY_PROPS = "props";
constexpr auto KEY_GENERAL = "general";
constexpr auto KEY_LOG_FOLDER = "log_folder";
constexpr auto KEY_LOG_NAME = "log_name";
constexpr auto KEY_LOG_LEVEL = "log_level";
constexpr auto KEY_DEBUG_THRESHOLD = "debug_threshold";
constexpr auto KEY_HTTP_ENABLED = "http_enabled";
constexpr auto KEY_HTTP_PORT = "http_port";
constexpr auto KEY_WEB_ROOT = "web_root";
constexpr auto KEY_WEB_TOKEN = "web_token";
constexpr auto CONFIG_FOLDER = "./etc";
constexpr auto CONFIG_FILE = "config.yaml";
constexpr auto SPDLOG_FLUSH_SEC = 3;

static void gst_log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    switch (log_level)
    {
    case G_LOG_LEVEL_ERROR:
        ELOG("{} {}", log_domain, message);
        break;
    case G_LOG_LEVEL_CRITICAL:
        ELOG("{} {}", log_domain, message);
        break;
    case G_LOG_LEVEL_WARNING:
        WLOG("{} {}", log_domain, message);
        break;
    case G_LOG_LEVEL_MESSAGE:
        ILOG("{} {}", log_domain, message);
        break;
    case G_LOG_LEVEL_INFO:
        ILOG("{} {}", log_domain, message);
        break;
    case G_LOG_LEVEL_DEBUG:
        TLOG("{} {}", log_domain, message);
        break;
    default:
        break;
    }
}

PipelineVerifier::PipelineVerifier(int argc, char *argv[])
{
    gst_init(&argc, &argv);
}

PipelineVerifier::~PipelineVerifier()
{
    join_web_server();
    gst_deinit();
}

int PipelineVerifier::init(const std::string& config_file, const std::string& log_level)
{
    m_config_file = config_file;

    if (m_config_file.empty())
    {
        if (file_exists(CONFIG_FILE)) {
            m_config_file = CONFIG_FILE;
        } else {
            m_config_file = CONFIG_FOLDER;
            m_config_file.append("/");
            m_config_file.append(CONFIG_FILE);
        }
    }
    try {
        auto log_config = read_yaml_section(m_config_file, KEY_GENERAL);
        // init logger
        auto &logger = Logger::get_instance();
        logger.init(log_config[KEY_LOG_FOLDER], log_config[KEY_LOG_NAME], 20, 20);

        int log_level_num = 0;
        if (!log_level.empty())
        {
            log_level_num = str_to_int(log_config[KEY_LOG_LEVEL]);
        } else {
            log_level_num = str_to_int(log_level);
        }

        logger.reset_level(log_level_num, SPDLOG_LEVEL_ERROR, SPDLOG_FLUSH_SEC);


        int ret = read_config_file(m_config_file);
        if (ret < 0) {
            ELOG("configuration is incorrect.");
            return ret;
        }
    } catch (const std::exception &e) {
        std::cerr << "PipelineVerifier::init read config exception: " << e.what() << std::endl;
        return -1;
    }

    // set gst log handler
    guint major, minor, micro, nano = 0;
    gst_version(&major, &minor, &micro, &nano);
    gst_debug_set_active(TRUE);

    int debug_threshold = m_app_config.get_general_config().debug_threshold;
    if (debug_threshold <= 0)
    {
        debug_threshold = GST_LEVEL_FIXME;
    }
    gst_debug_set_default_threshold((GstDebugLevel)debug_threshold);

    g_log_set_handler(NULL, G_LOG_LEVEL_MASK, gst_log_handler, NULL);

    DLOG("gstreamer initialized, gst version = {}.{}.{}.{}", major, minor, micro, nano);

    return 0;
}

int PipelineVerifier::read_pipelines_config(YAML::Node& config)
{
    if (config[KEY_PIPELINES])
    {
        YAML::Node childNode = config[KEY_PIPELINES];
        YAML::const_iterator it = childNode.begin();
        for (; it != childNode.end(); ++it)
        {
            std::string key = it->first.as<std::string>();

            read_pipeline_config(key, it->second);
        }
        return 0;
    }
    return -1;
}

int PipelineVerifier::read_pipeline_config(const std::string& key, const YAML::Node &pipelineNode)
{
    if (pipelineNode.Type() == YAML::NodeType::Map)
    {
        auto pipeline_tags = pipelineNode["tags"];
        auto pipeline_desc = pipelineNode["desc"];
        auto pipeline_steps = pipelineNode["steps"];

        auto pipeline_props = pipelineNode["props"];

        auto vec = pipeline_steps.as<std::vector<std::string>>();

        auto pipeline_config_ptr = m_app_config.get_pipelines_config().emplace(key, vec,
            pipeline_desc.as<std::string>(), pipeline_tags.as<std::string>());

        if (pipeline_config_ptr && pipeline_props) {
            pipeline_config_ptr->m_props = pipeline_props.as<std::string>();
        }

    } else if (pipelineNode.Type() == YAML::NodeType::Sequence) {
        auto vec = pipelineNode.as<std::vector<std::string>>();
        m_app_config.get_pipelines_config().emplace(key, vec);
    } else {
        WLOG("unkown type of {}", key);
    }
    return 0;
}

int PipelineVerifier::read_general_config(YAML::Node &config)
{
    if (config[KEY_GENERAL])
    {
        auto config_map = config[KEY_GENERAL].as<std::map<std::string, std::string>>();

        m_app_config.get_general_config().log_name = config_map[KEY_LOG_NAME];
        m_app_config.get_general_config().log_level = str_to_int(config_map[KEY_LOG_LEVEL]);
        m_app_config.get_general_config().log_folder = config_map[KEY_LOG_FOLDER];

        m_app_config.get_general_config().debug_threshold = str_to_int(config_map[KEY_DEBUG_THRESHOLD]);
        m_app_config.get_general_config().default_pipeline = config_map[KEY_DEFAULT_PIPELINE];

        m_app_config.get_general_config().http_enabled = str_to_int(config_map[KEY_HTTP_ENABLED]);
        m_app_config.get_general_config().http_port = str_to_int(config_map[KEY_HTTP_PORT]);
        m_app_config.get_general_config().web_root = config_map[KEY_WEB_ROOT];
        m_app_config.get_general_config().web_token = config_map[KEY_WEB_TOKEN];
        return 0;
    }
    return -1;
}

int PipelineVerifier::read_probe_config(YAML::Node &config)
{
    if (config[KEY_PROBES])
    {
        YAML::Node childNode = config[KEY_PROBES];
        YAML::const_iterator it = childNode.begin();
        for (; it != childNode.end(); ++it)
        {
            std::string key = it->first.as<std::string>();
            std::map<std::string, std::string> vals = it->second.as<std::map<std::string, std::string>>();
            ProbeConfigItem probeConfigItem;
            probeConfigItem.probe_pipeline_name = vals["probe_pipeline"];
            probeConfigItem.probe_element_name = vals["probe_element"];
            probeConfigItem.probe_pad_name = vals["probe_pad"];
            probeConfigItem.probe_callback_name = vals["probe_callback"];
            probeConfigItem.probe_type = str_to_int(vals["probe_type"]);
            m_app_config.get_probe_config().add_probe_config_item(probeConfigItem);
            DLOG("add_probe_config_item for {} on {}'{} by {}",
                probeConfigItem.probe_pipeline_name, probeConfigItem.probe_element_name,
                probeConfigItem.probe_pad_name, probeConfigItem.probe_callback_name);
        }
        return 0;
    }
    return -1;
}

int PipelineVerifier::read_props_config(YAML::Node &config)
{
    if (config[KEY_PROPS])
    {
        YAML::Node childNode = config[KEY_PROPS];
        YAML::const_iterator it = childNode.begin();
        for (; it != childNode.end(); ++it)
        {
            std::string key = it->first.as<std::string>();
            std::map<std::string, std::string> vals = it->second.as<std::map<std::string, std::string>>();
            PropConfigItem propConfigItem;
            propConfigItem.prop_key = key;
            propConfigItem.pipeline_name = vals["pipeline_name"];
            propConfigItem.element_name = vals["element_name"];
            propConfigItem.prop_name = vals["prop_name"];
            propConfigItem.prop_value = vals["prop_value"];
            //std::cout << "add prop: " << propConfigItem.prop_name << "=" << propConfigItem.prop_value << std::endl;
            m_app_config.get_props_config().add_prop_config_item(propConfigItem);
        }
        return 0;
    }
    return -1;
}
// !! Do not write log in read config because logger need configuration for initiation
int PipelineVerifier::read_config_file(const std::string config_file)
{

    if (!endswith(m_config_file, "yaml") && !endswith(m_config_file, "yml")) {
        std::cerr << "only support yaml/yml format for now: " << m_config_file << std::endl;
        return -1;
    }

    YAML::Node config = YAML::LoadFile(config_file);
    
    auto ret = read_general_config(config);
    ret += read_probe_config(config);
    ret += read_pipelines_config(config);
    ret += read_props_config(config);

    if (directory_exists(CONFIG_FOLDER))
    {
        read_all_config_files(CONFIG_FOLDER);
    }

    return ret;
}

int PipelineVerifier::read_all_config_files(const char *szFolder)
{
    std::vector<std::string> config_files;
    int cnt = retrieve_files(szFolder, "yaml", config_files);
    if (cnt <= 0)
    {
        std::cerr << "no config files under " << szFolder << std::endl;
        return -1;
    }
    for (auto &file : config_files)
    {
        std::string config_file(szFolder);
        config_file = config_file + "/" + file;
        if (endswith(config_file, ".yaml") || endswith(config_file, ".yml")) {
            YAML::Node config = YAML::LoadFile(config_file);
            read_pipelines_config(config);
        }
        
    }
    return cnt;
}

void PipelineVerifier::list_pipelines(const std::string &pipeline_name)
{
    auto& pipelines_map = m_app_config.get_pipelines_config().get_pipelines();
    auto it = pipelines_map.begin();
    int i = 0;
    for (; it != pipelines_map.end(); ++it)
    {
        if (!pipeline_name.empty())
        {
            if (pipeline_name != it->first)
            {
                continue;
            }
        }
        cout << (++i) << ". " << it->first << ": " << endl;
        auto &vec = it->second->m_steps;
        int cnt = vec.size();
        for (int i = 0; i < cnt; ++i)
        {
            if (i > 0)
            {
                cout << "  ! ";
            }
            else
            {
                cout << "  gst-launch-1.0 -ev ";
            }

            cout << vec[i];
            if (i < cnt - 1)
            {
                cout << " \\" << endl;
            }
            else
            {
                cout << endl;
            }
        }
    }
}

int PipelineVerifier::start_web_server(const char *doc_root, int port)
{
    char szPort[10];
    snprintf(szPort, sizeof(szPort), "%d", port);

    const char *options[] = {"document_root", doc_root,
                             "listening_ports", szPort, NULL};
    CivetServer server(options);

    TestHandler test_handler(m_app_config);
    server.addHandler("/test", test_handler);
    test_handler.register_pipeline_setter([&](const PropConfigItem& prop_config) {
        ILOG("pipeline setter {}={}", prop_config.prop_name, prop_config.prop_value);
        auto ret = change_pipeline(prop_config);
        ILOG("pipeline setter end with {}", ret);
        return ret;
    });

    ExitHandler exit_handler;
    server.addHandler("/exit", exit_handler);

    VerifyHandler verify_handler(m_app_config);
    server.addHandler("/verify", verify_handler);

    verify_handler.register_pipeline_runner([&](const std::string& name, const std::string& args) {
        ILOG("pipeline {} start", name);
        auto ret = run_pipeline(name, args);
        ILOG("pipeline {} end with {}", name, ret);
        return ret;
    });

    ILOG("web server started on port {} for {}", port, doc_root);
    while (!exit_handler.exit_now())
    {
#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    return 0;
}

void PipelineVerifier::fork_web_server(int http_port, bool forced) {

    auto general_config = m_app_config.get_general_config();
    if (!general_config.http_enabled && !forced)
    {
        ILOG("not enable http server");
        return;
    }

    if (http_port <= 0) {
        http_port = general_config.http_port;
    }

    m_web_thread = std::make_unique<std::thread>([=] {
        ILOG("start web server on {}", http_port);
        start_web_server(general_config.web_root.c_str(), http_port);
    });
    
}

void PipelineVerifier::join_web_server()
{
    if (m_web_thread) {
        m_web_thread->join();
    }
}


int PipelineVerifier::run_pipeline(const std::string pipeline_name, const std::string variables) {
    m_pipeline_builder = std::make_shared<PipelineBuilder>(m_app_config);
    int ret = m_pipeline_builder->init(pipeline_name, variables);
    CHECK_VALUE("pipeline init, ret={}",  ret, 0);
    ret = m_pipeline_builder->build();
    CHECK_VALUE("pipeline build, ret={}", ret, 0);
    ret = m_pipeline_builder->start();
    CHECK_VALUE("pipeline start, ret={}", ret, 0);
    ret = m_pipeline_builder->stop();
    CHECK_VALUE("pipeline stop, ret={}",  ret, 0);
    ret = m_pipeline_builder->clean();
    CHECK_VALUE("pipeline clean, ret={}", ret, 0);
    return ret;
}

int PipelineVerifier::change_pipeline(const PropConfigItem& propConfigItem) {
    if (!m_pipeline_builder) {
        WLOG("pipeline {} have not been setup", propConfigItem.pipeline_name);
        return -1;
    }

    return m_pipeline_builder->set_prop(propConfigItem);
}


} // namespace hefei
