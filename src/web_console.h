#pragma once
#include <cstdio>
#include <functional>
#include "CivetServer.h"
#include "pipeline_config.h"

namespace hefei {

using pipeline_runner_t = std::function<int(const std::string& name, const std::string& args)>;

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

class VerifyHandler : public CivetHandler
{
public:
    VerifyHandler(AppConfig& app_config)
        :m_app_config (app_config)  {}
    bool handleGet(CivetServer *server, struct mg_connection *conn);
    bool handlePost(CivetServer *server, struct mg_connection *conn);
    std::string dump_pipelines();
    void register_pipeline_runner(const pipeline_runner_t& runner) {
        m_pipeline_runner = runner;
    }
private:
    AppConfig& m_app_config;
    pipeline_runner_t m_pipeline_runner;
};


}//namespace hefei