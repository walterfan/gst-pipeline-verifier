set(CMAKE_CXX_FLAGS "-Wno-old-style-cast")
set(CMAKE_CXX_FLAGS "-Wno-unused-variable")
set(CMAKE_CXX_FLAGS "-Wno-sign-conversion")

aux_source_directory(${CMAKE_SOURCE_DIR}/util common_util_srcs)

add_library(common_util ${common_util_srcs})

target_link_libraries(common_util 
    ${GST_LIBRARIES} 
    fmt::fmt 
    spdlog::spdlog
    yaml-cpp::yaml-cpp
    pthread)