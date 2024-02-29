
add_executable(gst-pipeline-verify 
    gst_pipeline_verify.cpp
    pipeline_builder.cpp 
    pipeline_config.cpp
)

target_link_libraries(gst-pipeline-verify
    ${GST_LIBRARIES}
    common_util
    fmt::fmt 
    spdlog::spdlog
    yaml-cpp
)
