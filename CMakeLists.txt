cmake_minimum_required(VERSION 3.10)
project(gstreamer_cookbook)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Ofast -Wfatal-errors -g  -D_MWAITXINTRIN_H_INCLUDED")

find_package(PkgConfig REQUIRED)

if (APPLE)
    set(GST_INCLUDE_DIRS /Library/Frameworks/GStreamer.framework/Headers)
    set(GST_LIBRARY_DIRS /Library/Frameworks/GStreamer.framework/Versions/Current/lib)

    include_directories(${GST_INCLUDE_DIRS})
    link_directories(${GST_LIBRARY_DIRS})

    set(GTEST_INCLUDE_DIRS /opt/homebrew/Cellar/googletest/1.14.0/include)
    set(GTEST_LIBRARY_DIRS /opt/homebrew/Cellar/googletest/1.14.0/lib)
    include_directories(${GTEST_INCLUDE_DIRS})
    link_directories(${GTEST_LIBRARY_DIRS})
    
elseif(UNIX)
    pkg_search_module(GST REQUIRED gstreamer-1.0)
    include_directories(${GST_INCLUDE_DIRS})
    link_directories(${GST_LIBRARY_DIRS})
endif()


find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(civetweb CONFIG REQUIRED)
find_package(nlohmann_json 3.11.3 REQUIRED)
find_package(CURL REQUIRED)

if(NOT spdlog_FOUND)
    message(WARNING "spdlog not found!")
else()
    message(STATUS "Found spdlog on: " ${spdlog_DIR})
endif(NOT spdlog_FOUND)

if(civetweb_FOUND)
    message(STATUS "civetweb found!")
endif(civetweb_FOUND)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/util)
include_directories(${PROJECT_SOURCE_DIR}/src)

set(GST_LIBRARIES -lgstreamer-1.0 -lgobject-2.0 -lglib-2.0)
set(GST_RTC_LIBS -lgstwebrtc-1.0 -lgstsdp-1.0 -lgstrtp-1.0 -lsoup-2.4 -ljson-glib-1.0)

add_subdirectory(src)
add_subdirectory(util)
add_subdirectory(test)


