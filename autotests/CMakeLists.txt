remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

find_package(Qt5Test ${REQUIRED_QT_VERSION} CONFIG QUIET)
find_package(Qt5Network)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

ecm_add_tests(
    karchivetest.cpp
    kfiltertest.cpp
    deprecatedtest.cpp
    LINK_LIBRARIES KF5::Archive Qt5::Test
)

target_link_libraries(kfiltertest ${ZLIB_LIBRARIES})

########### klimitediodevicetest ###############

ecm_add_test(
    klimitediodevicetest.cpp
    ../src/klimitediodevice.cpp
    TEST_NAME klimitediodevicetest
    LINK_LIBRARIES Qt5::Test
)
target_include_directories(klimitediodevicetest
    PRIVATE $<TARGET_PROPERTY:KF5Archive,INTERFACE_INCLUDE_DIRECTORIES>)

########## kcompressiondevicetest ##############
if (Qt5Network_FOUND)
    ecm_add_test(
        kcompressiondevicetest.cpp
        LINK_LIBRARIES KF5::Archive Qt5::Test Qt5::Network
    )

    set(testDir $<TARGET_FILE_DIR:kcompressiondevicetest>)
    get_filename_component(topdir ${CMAKE_CURRENT_SOURCE_DIR}/.. ABSOLUTE)

    add_custom_command(TARGET kcompressiondevicetest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar czf
        ${testDir}/kcompressiondevice_test.tar.gz examples
        WORKING_DIRECTORY ${topdir})
    add_custom_command(TARGET kcompressiondevicetest POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E tar cjf
        ${testDir}/kcompressiondevice_test.tar.bz2 examples
        WORKING_DIRECTORY ${topdir})

    if (LIBLZMA_FOUND)
        add_custom_command(TARGET kcompressiondevicetest POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E tar cJf
            ${testDir}/kcompressiondevice_test.tar.xz examples
            WORKING_DIRECTORY ${topdir})
    endif()
endif()
