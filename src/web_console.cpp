#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <sstream>
#include "web_console.h"
#include "file_util.h"
#include "logger.h"
#include "string_util.h"
#include "http_util.h"
#include <nlohmann/json.hpp>



using json = nlohmann::json;

namespace hefei {

    static int send_json(struct mg_connection *conn, int status, const char* json_str)
{
    size_t json_str_len = strlen(json_str);

    /* Send HTTP message header (+1 for \n) */
    if (status >=200 && status < 400) {
        mg_send_http_ok(conn, "application/json; charset=utf-8", json_str_len + 1);
    } else {
        mg_send_http_error(conn, status, "%s", http_response_status(status));
    }


    /* Send HTTP message content */
    mg_write(conn, json_str, json_str_len);

    /* Add a newline. This is not required, but the result is more
        * human-readable in a debugger. */
    mg_write(conn, "\n", 1);

    return (int)json_str_len;
}

static void respond_json(int result, const std::string& desc, struct mg_connection *conn)
{
    nlohmann::json json_obj;
    json_obj["result"] = result;
    json_obj["desc"] = desc;
    send_json(conn, result,  json_obj.dump().c_str());
}


std::string TestHandler::get_prop_value(const std::string& prop_key) {
    PropsConfig& props_config = m_app_config.get_props_config();
    PropConfigItem& prop_item = props_config.get_prop_config_item(prop_key);
    return prop_item.prop_value;
}

bool TestHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
    //const struct mg_request_info *req_info = mg_get_request_info(conn);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/html\r\nConnection: close\r\n\r\n");


    std::string file_name = "./doc/test.html";
    std::string file_content;
    file2msg(file_name, file_content);

    std::string oldContent = "{{ default_prop_value }}";
    std::string newContent = get_prop_value("algo_infer_test_1");

    // Replace the old content with the new content
    size_t pos = file_content.find(oldContent);
    while (pos != std::string::npos)
    {
        file_content.replace(pos, oldContent.length(), newContent);
        pos = file_content.find(oldContent, pos + newContent.length());
    }

    mg_printf(conn, "%s", file_content.c_str());
    return true;
}

bool TestHandler::handlePost(CivetServer *server, struct mg_connection *conn)
{
    const struct mg_request_info *req_info = mg_get_request_info(conn);

    long long tlen = req_info->content_length;

    std::vector<unsigned char> buf(tlen);
    unsigned char *buffer = buf.data();

    size_t num_bytes = mg_read(conn, buffer, tlen);

    std::string post_data(reinterpret_cast<char *>(buffer), num_bytes);

    DLOG("got {}", post_data);
    try {
        nlohmann::json json_data = nlohmann::json::parse(post_data);

        std::string pipeline = json_data["pipeline"].get<std::string>();
        std::string element = json_data["element"].get<std::string>();
        std::string prop_name = json_data["prop_name"].get<std::string>();
        std::string prop_value = json_data["prop_value"].get<std::string>();
        if (pipeline.empty() || element.empty() || prop_name.empty() || prop_value.empty()) {
            respond_json(400, "required text is empty", conn);
            return true;
        }

        PropConfigItem propConfigItem;
        //propConfigItem.prop_key = ;
        propConfigItem.pipeline_name = pipeline;
        propConfigItem.element_name = element;
        propConfigItem.prop_name = prop_name;
        propConfigItem.prop_value = prop_value;

        DLOG("pipeline setter, element={}, prop_name={}", propConfigItem.element_name, propConfigItem.prop_name);
        if (m_pipeline_setter) {
            int ret = m_pipeline_setter(propConfigItem);
            if (ret == 0) {
                respond_json(200, "pipeline set succeed", conn);
            } else {
                respond_json(400, "pipeline set failed", conn);
            }
        } else {
            respond_json(500, "m_pipeline_setter have not registered", conn);
        }

    } catch (const std::exception &e) {
        ELOG("pipeline exception: {}", e.what());
        respond_json(400, "pipeline error", conn);
    }


    return true;
}


bool ExitHandler::handleGet(CivetServer *server, struct mg_connection *conn)
{
    mg_printf(conn,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "text/plain\r\nConnection: close\r\n\r\n");
    mg_printf(conn, "Good-Bye!\n");
    exitNow = true;
    return true;
}


std::string VerifyHandler::dump_pipelines() {
    std::stringstream ss;

    int i = 0;
    auto pipelines_config = m_app_config.get_pipelines_config();
    for(auto kv: pipelines_config.get_pipelines()) {
        auto& pipeline = kv.second;
        ss << "<tr><td>" << (++i) << "</td>";
        ss << "<td>" << pipeline->m_name << "</td>";
        ss << "<td>" << pipeline->m_desc << "</td>";
        ss << "<td>" << pipeline->m_tags << "</td>";
        ss << "<td>" << pipeline->dump_pipeline_line() << "</td>";
    }
    return ss.str();
}

bool VerifyHandler::handleGet(CivetServer *server, struct mg_connection *conn)
{
    //const struct mg_request_info *req_info = mg_get_request_info(conn);
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "text/html\r\nConnection: close\r\n\r\n");


    std::string file_name = "./doc/pipelines.html";
    std::string file_content;
    file2msg(file_name, file_content);

    std::string oldContent = "{{ pipeline_rows }}";
    std::string newContent = dump_pipelines();

    // Replace the old content with the new content
    size_t pos = file_content.find(oldContent);
    while (pos != std::string::npos)
    {
        file_content.replace(pos, oldContent.length(), newContent);
        pos = file_content.find(oldContent, pos + newContent.length());
    }

    mg_printf(conn, "%s", file_content.c_str());
    return true;
}

bool VerifyHandler::handlePost(CivetServer *server, struct mg_connection *conn) {

    const struct mg_request_info *req_info = mg_get_request_info(conn);

    long long tlen = req_info->content_length;

    unsigned char *buffer = new unsigned char[tlen];
    
    size_t num_bytes = mg_read(conn, buffer, tlen);

    // Convert the buffer to a std::string for easier processing
    std::string post_data(reinterpret_cast<char *>(buffer), num_bytes);

    // Free the buffer
    delete[] buffer;

    //std::string command = url_decode(post_data);
    trim(post_data);
    DLOG("verify pipeline: {}", post_data);

    try {
        json json_data = json::parse(post_data);

        if (!(json_data.contains("pipeline") && json_data.contains("token"))) {
            respond_json(400, "pipeline and token fields required", conn);
            return true;
        }

        auto pipe = json_data["pipeline"].get<std::string>();
        auto args = json_data["args"].get<std::string>();
        auto token = json_data["token"].get<std::string>();
        if (m_app_config.get_general_config().web_token.compare(trim_space(token)) != 0) {
            //WLOG("token not match: '{}'!='{}'", m_app_config.get_general_config().web_token, token);
            respond_json(401, "token verified failed", conn);
            return true;
        }
        DLOG("pipeline={}, parameters={}", pipe, args);
        if (m_pipeline_runner) {
            int ret = m_pipeline_runner(pipe, args);
            if (ret == 0) {
                respond_json(200, "pipeline verified succeed", conn);
            } else {
                respond_json(400, "pipeline verified failed", conn);
            }
        } else {
            respond_json(500, "m_pipeline_runner have not registered", conn);
        }
    }
    catch (const nlohmann::json::parse_error &e)
    {
        ELOG("JSON parse error:{} ", e.what());
        respond_json(400, "json parse error", conn);
    }
    catch (const std::exception &e)
    {
        ELOG("Standard exception: {}", e.what());
        respond_json(500, "std exception", conn);
    }
    return true;
}

}//namespace hefei