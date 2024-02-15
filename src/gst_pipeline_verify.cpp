#include <gst/gst.h>

#ifdef __APPLE__
#include <TargetConditionals.h>
#endif

#include "pipeline_builder.h"
#include "logger.h"

constexpr auto log_pattern = "[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] (%!) %v";

int verify_pipeline(int argc, char *argv[], void* param=nullptr) {

    auto& logger = Logger::get_instance();
    logger.init("./log", "gst_pipeline_verify", 20, 20);
    logger.reset_level(SPDLOG_LEVEL_DEBUG, 4, 3);
    
    int ret = 0;
    auto verifier = std::make_unique<PipelineBuilder>();
    ret = verifier->init(argc, argv);
    if (ret < 0) {
        fprintf(stderr, "initialize failed: %d \nexample: %s", ret,
            " ./bin/gst-pipeline-verify -f ./etc/pipeline.yaml -p pipeline_test" );
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
    
    return 0;
}
  
int main (int argc, char *argv[])
{
    #if defined(__APPLE__) && TARGET_OS_MAC && !TARGET_OS_IPHONE
    return gst_macos_main(verify_pipeline, argc, argv, NULL);
    #else
    return verify_pipeline(argc, argv);
    #endif
}
