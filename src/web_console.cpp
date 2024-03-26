#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "web_console.h"

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

}//namespace hefei