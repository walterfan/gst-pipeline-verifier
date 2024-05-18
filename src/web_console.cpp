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

bool TestHandler::handleGet(CivetServer *server, struct mg_connection *conn) {
    const struct mg_request_info *req_info = mg_get_request_info(conn);
    mg_printf(conn,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "text/html\r\nConnection: close\r\n\r\n");

    mg_printf(conn, "<html><body>\n");
    mg_printf(conn, "<h2>Welcome</h2>\n");
    mg_printf(conn,
                "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
                req_info->request_method,
                req_info->request_uri,
                req_info->http_version);
    mg_printf(conn, "</body></html>\n");
    return true;
}

bool TestHandler::handlePost(CivetServer *server, struct mg_connection *conn)
{
    /* Handler may access the request info using mg_get_request_info */
    const struct mg_request_info *req_info = mg_get_request_info(conn);
    long long rlen, wlen;
    long long nlen = 0;
    long long tlen = req_info->content_length;
    char buf[1024];

    mg_printf(conn,
                "HTTP/1.1 200 OK\r\nContent-Type: "
                "text/html\r\nConnection: close\r\n\r\n");

    mg_printf(conn, "<html><body>\n");
    mg_printf(conn, "<h2>This is the Foo POST handler!!!</h2>\n");
    mg_printf(conn,
                "<p>The request was:<br><pre>%s %s HTTP/%s</pre></p>\n",
                req_info->request_method,
                req_info->request_uri,
                req_info->http_version);
    mg_printf(conn, "<p>Content Length: %li</p>\n", (long)tlen);
    mg_printf(conn, "<pre>\n");

    while (nlen < tlen) {
        rlen = tlen - nlen;
        if (rlen > sizeof(buf)) {
            rlen = sizeof(buf);
        }
        rlen = mg_read(conn, buf, (size_t)rlen);
        if (rlen <= 0) {
            break;
        }
        wlen = mg_write(conn, buf, (size_t)rlen);
        if (wlen != rlen) {
            break;
        }
        nlen += wlen;
    }

    mg_printf(conn, "\n</pre>\n");
    mg_printf(conn, "</body></html>\n");

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
    const struct mg_request_info *req_info = mg_get_request_info(conn);
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
            WLOG("token not match: '{}'!='{}'", m_app_config.get_general_config().web_token, token);
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