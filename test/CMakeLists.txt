
aux_source_directory(${CMAKE_SOURCE_DIR}/test unit_test_srcs)

add_executable(unit_test ${unit_test_srcs})

target_link_libraries(unit_test
    common_util
    gst-pipeline-util
    CURL::libcurl
    nlohmann_json::nlohmann_json
    gtest::gtest)