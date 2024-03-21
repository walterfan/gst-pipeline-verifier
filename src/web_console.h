#pragma once
#include <cstdio>
#include "CivetServer.h"

namespace wfan {

class TestHandler : public CivetHandler {
public:
    bool handleGet(CivetServer *server, struct mg_connection *conn);
    bool handlePost(CivetServer *server, struct mg_connection *conn);
};

class ExitHandler : public CivetHandler
{
public:
	bool handleGet(CivetServer *server, struct mg_connection *conn);
    bool exit_now() { return exitNow; }
    volatile bool exitNow = false;
};


int start_web_server(const char *doc_root, int port = 8080);


}//namespace wfan