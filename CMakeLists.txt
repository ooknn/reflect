cmake_minimum_required(VERSION 3.8)
project(reflect LANGUAGES CXX)
add_executable(reflect "")

set_property(TARGET reflect PROPERTY CXX_STANDARD 11)
set_property(TARGET reflect PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET reflect PROPERTY CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(CMAKE_BUILD_TYPE Debug)
find_package(Boost )
include_directories(${Boost_INCLUDE_DIR})
set(RapidJSON_INCLUDE_DIRS thirdparty/rapidjson/include)
target_include_directories(reflect PRIVATE ${RapidJSON_INCLUDE_DIRS})
target_sources(reflect PRIVATE 
    main.cc
    )
