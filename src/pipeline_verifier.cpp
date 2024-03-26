#include <gst/gst.h>
#include <unistd.h>
#include "logger.h"
#include <vector>
#include <iostream>
#include "yaml-cpp/yaml.h"
#include "pipeline_verifier.h"
#include "web_console.h"

using namespace std;

namespace hefei {

constexpr auto KEY_DEFAULT_PIPELINE = "default_pipeline";
constexpr auto DECODE_BIN = "decodebin";
constexpr auto DEMUX = "demux";
constexpr auto KEY_PIPELINES = "pipelines";
constexpr auto KEY_PROBES = "probes";
constexpr auto KEY_GENERAL = "general";
constexpr auto KEY_LOG_FOLDER = "log_folder";
constexpr auto KEY_LOG_NAME = "log_name";
constexpr auto KEY_LOG_LEVEL = "log_level";
constexpr auto KEY_DEBUG_THRESHOLD = "debug_threshold";
constexpr auto KEY_HTTP_ENABLED = "http_enabled";
constexpr auto KEY_HTTP_PORT = "http_port";
constexpr auto KEY_WEB_ROOT = "web_root";
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

int PipelineVerifier::init(const std::string& log_level)
{
    GeneralConfig &general_config = get_app_config().get_general_config();
    
    if (!log_level.empty())
    {
        general_config.log_level = str_to_int(log_level);
    }
    // init logger
    auto &logger = Logger::get_instance();
    logger.init(general_config.log_folder, general_config.log_name, 20, 20);
    logger.reset_level(general_config.log_level, SPDLOG_LEVEL_ERROR, SPDLOG_FLUSH_SEC);

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

int PipelineVerifier::read_config_file(const std::string config_file)
{
    m_config_file = config_file;

    if (m_config_file.empty())
    {
        if (file_exists(CONFIG_FILE)) {
            m_config_file = CONFIG_FILE;
        }
        else {
            m_config_file = CONFIG_FOLDER;
            m_config_file.append("/");
            m_config_file.append(CONFIG_FILE);
        }
    }

    YAML::Node config = YAML::LoadFile(m_config_file);

    if (config[KEY_PIPELINES])
    {
        YAML::Node childNode = config[KEY_PIPELINES];
        YAML::const_iterator it = childNode.begin();
        for (; it != childNode.end(); ++it)
        {
            std::string key = it->first.as<std::string>();
            std::vector<std::string> vals = it->second.as<std::vector<std::string>>();
            m_app_config.get_pipeline_config().emplace(std::make_pair(key, vals));
        }
    }

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
    }

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
            probeConfigItem.probe_type = str_to_int(vals["probe_type"]);
            m_app_config.get_probe_config().add_probe_config_item(probeConfigItem);
        }
    }

    if (directory_exists(CONFIG_FOLDER))
    {
        read_all_config_files(CONFIG_FOLDER);
    }

    return 0;
}
// do not write log in read config because logger need configuration for initiation
int PipelineVerifier::read_all_config_files(const char *szFolder)
{
    std::vector<std::string> config_files;
    int cnt = retrieve_files(szFolder, config_files);
    if (cnt <= 0)
    {
        std::cerr << "No config files under " << szFolder << std::endl;
        return -1;
    }
    for (auto &file : config_files)
    {
        std::string config_file(szFolder);
        config_file = config_file + "/" + file;
        yaml_to_str_vec_map(config_file, KEY_PIPELINES, m_app_config.get_pipeline_config());
    }
    return cnt;
}

void PipelineVerifier::list_pipelines(const std::string &pipeline_name)
{
    auto it = m_app_config.get_pipeline_config().begin();
    int i = 0;
    for (; it != m_app_config.get_pipeline_config().end(); ++it)
    {
        if (!pipeline_name.empty())
        {
            if (pipeline_name != it->first)
            {
                continue;
            }
        }
        cout << (++i) << ". " << it->first << ": " << endl;
        auto &vec = it->second;
        auto cnt = vec.size();
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

    TestHandler handler;
    server.addHandler("/test", handler);

    ExitHandler exit_handler;
    server.addHandler("/exit", exit_handler);

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
        ILOG("Not enable http server");
        return;
    }

    if (http_port <= 0) {
        http_port = general_config.http_port;
    }

    m_thptr = std::make_unique<std::thread>([=] {
        ILOG("start web server on {}", http_port);
        start_web_server(general_config.web_root.c_str(), http_port); 
    });
    
}

void PipelineVerifier::join_web_server()
{
    if (m_thptr) {
        m_thptr->join();
    }
}

} // namespace hefei