cmake_minimum_required(VERSION 3.21)

project(tiledagentbridge LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Test)
find_package(ZLIB REQUIRED)

file(GLOB LIBTILED_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../libtiled/*.cpp"
)

add_library(tiledagent_libtiled STATIC ${LIBTILED_SOURCES})
target_include_directories(tiledagent_libtiled
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../libtiled"
)
target_link_libraries(tiledagent_libtiled
  PUBLIC
    Qt6::Core
    Qt6::Gui
    ZLIB::ZLIB
)

add_executable(tiledagentbridge
  bridge_main.cpp
  bridgeengine.cpp
  jsonrpcserver.cpp
)
target_include_directories(tiledagentbridge PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(tiledagentbridge PRIVATE tiledagent_libtiled Qt6::Core Qt6::Gui)

add_executable(tiledagentbridge_tests
  test_bridgeengine.cpp
  bridgeengine.cpp
)
target_include_directories(tiledagentbridge_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(tiledagentbridge_tests PRIVATE
  TEST_SOURCE_ROOT="${PROJECT_SOURCE_DIR}/../.."
)
target_link_libraries(tiledagentbridge_tests PRIVATE tiledagent_libtiled Qt6::Core Qt6::Gui Qt6::Test)

include(CTest)
add_test(NAME tiledagentbridge_tests COMMAND tiledagentbridge_tests)
