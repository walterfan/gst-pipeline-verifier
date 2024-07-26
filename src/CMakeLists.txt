list (APPEND GST_PIPELINE_UTIL_FILES
    web_console.cpp
    pipeline_builder.cpp
    pipeline_verifier.cpp
    pipeline_config.cpp
    media_player.cpp
)

list (APPEND GST_PIPELINE_SOURCES
	${GST_PIPELINE_UTIL_FILES}
    main.cpp
)

add_library(gst-pipeline-util ${GST_PIPELINE_UTIL_FILES})

target_link_libraries(gst-pipeline-util
    ${GST_LIBRARIES}
    fmt::fmt
    spdlog::spdlog
    yaml-cpp::yaml-cpp
    civetweb::civetweb-cpp
    nlohmann_json::nlohmann_json
    CURL::libcurl)

add_executable(gst-pipeline-verify
    ${GST_PIPELINE_SOURCES}
)

target_link_libraries(gst-pipeline-verify
    ${GST_LIBRARIES}
    common_util
    fmt::fmt
    spdlog::spdlog
    yaml-cpp::yaml-cpp
    civetweb::civetweb-cpp
    nlohmann_json::nlohmann_json
    CURL::libcurl
)
