
add_executable(gst-pipeline-verify 
    main.cpp
    web_console.cpp
    pipeline_builder.cpp 
    pipeline_verifier.cpp 
    pipeline_config.cpp
)

target_link_libraries(gst-pipeline-verify
    ${GST_LIBRARIES}
    common_util
    fmt::fmt 
    spdlog::spdlog
    yaml-cpp::yaml-cpp
    civetweb::civetweb-cpp
    nlohmann_json::nlohmann_json
)
