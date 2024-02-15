#include <chrono>
#include <thread>
#include "gst_util.h"
#include "pipeline_builder.h"
#include "logger.h"

constexpr auto log_pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] (%!) %v";

int main(int argc, char *argv[]) {

    auto& logger = Logger::get_instance();
    logger.init("./log", "gst_pipeline_verify", 20, 20);
    logger.reset_level(SPDLOG_LEVEL_DEBUG, 4, 3);
    
    int ret = 0;
    auto verifier = std::make_unique<PipelineBuilder>();
    ret = verifier->init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "initialize failed: %d \nexample: %s", ret,
            " ./bin/gst-pipeline-verify -f ./example/etc/pipeline.yaml -p pipeline_test_rtmp" );
        return ret;
    } else if (ret > 0) {
        DLOG("bye");
        return ret;
    } 

    DLOG("initialize logger, ret={}", ret);
    verifier->build();
    verifier->start();
    verifier->stop();
    verifier->clean();
    

}

