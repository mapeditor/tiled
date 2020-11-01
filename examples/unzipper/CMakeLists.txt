cmake_minimum_required(VERSION 2.8.12)

project(Unzipper)

find_package(KF5Archive ${KF5_VERSION} REQUIRED)

add_executable(unzipper main.cpp)
target_link_libraries(unzipper KF5::Archive)
