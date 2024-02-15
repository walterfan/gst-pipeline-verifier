
add_executable(gst-pipeline-verify 
    gst_pipeline_verify.cpp
    pipeline_builder.cpp 
    pipeline_config.cpp
    gst_util.cpp 
    string_util.cpp
    file_util.cpp
    logger.cpp
)

target_link_libraries(gst-pipeline-verify
    ${GST_LIBRARIES} 
    fmt::fmt 
    spdlog::spdlog
    yaml-cpp
)
