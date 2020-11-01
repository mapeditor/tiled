cmake_minimum_required(VERSION 2.8.12)

project(BZip2GZip)

find_package(KF5Archive ${KF5_VERSION} REQUIRED)

add_executable(bzip2gzip main.cpp)
target_link_libraries(bzip2gzip KF5::Archive)
