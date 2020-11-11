set(HAVE_BZIP2_SUPPORT ${BZIP2_FOUND})
if(BZIP2_FOUND AND BZIP2_NEED_PREFIX)
    set(NEED_BZ2_PREFIX 1)
endif()

set(HAVE_XZ_SUPPORT ${LIBLZMA_FOUND})

configure_file(config-compression.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-compression.h)
add_definitions(-DQT_NO_CAST_FROM_ASCII)

if(BZIP2_FOUND)
    include_directories(${BZIP2_INCLUDE_DIR})
    set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kbzip2filter.cpp)
    set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${BZIP2_LIBRARIES})
endif()

if(LIBLZMA_FOUND)
    include_directories(${LIBLZMA_INCLUDE_DIRS})
    set(karchive_OPTIONAL_SRCS ${karchive_OPTIONAL_SRCS} kxzfilter.cpp k7zip.cpp)
    set(karchive_OPTIONAL_LIBS ${karchive_OPTIONAL_LIBS} ${LIBLZMA_LIBRARIES})
endif()


set(karchive_SRCS
    karchive.cpp
    kar.cpp
    kcompressiondevice.cpp
    kfilterbase.cpp
    kfilterdev.cpp
    kgzipfilter.cpp
    klimitediodevice.cpp
    knonefilter.cpp
    ktar.cpp
    kzip.cpp
    krcc.cpp
)

ecm_qt_declare_logging_category(karchive_SRCS
    HEADER loggingcategory.h
    IDENTIFIER KArchiveLog
    CATEGORY_NAME kf.archive
    OLD_CATEGORY_NAMES kf5.karchive
    DEFAULT_SEVERITY Warning
    DESCRIPTION "KArchive"
    EXPORT KARCHIVE
)

add_library(KF5Archive ${karchive_SRCS} ${karchive_OPTIONAL_SRCS})
add_library(KF5::Archive ALIAS KF5Archive)
ecm_generate_export_header(KF5Archive
    BASE_NAME KArchive
    GROUP_BASE_NAME KF
    VERSION ${KF5_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.0
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5Archive
    INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF5}/KArchive>"
)

target_link_libraries(KF5Archive
    PUBLIC
        Qt5::Core
    PRIVATE
        ${karchive_OPTIONAL_LIBS}
        ${ZLIB_LIBRARY}
)

set_target_properties(KF5Archive PROPERTIES
    VERSION     ${KARCHIVE_VERSION_STRING}
    SOVERSION   ${KARCHIVE_SOVERSION}
    EXPORT_NAME "Archive"
)

ecm_generate_headers(KArchive_HEADERS
    HEADER_NAMES
    KArchive
    KArchiveEntry
    KArchiveFile
    KArchiveDirectory
    KAr
    KCompressionDevice
    KFilterBase
    KFilterDev
    KRcc
    KTar
    KZip
    KZipFileEntry

    REQUIRED_HEADERS KArchive_HEADERS
)

install(TARGETS KF5Archive
        EXPORT KF5ArchiveTargets
        ${KF5_INSTALL_TARGETS_DEFAULT_ARGS})

if(LIBLZMA_FOUND)
    ecm_generate_headers(KArchive_HEADERS
        HEADER_NAMES
        K7Zip
        REQUIRED_HEADERS KArchive_HEADERS
    )
endif()

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/karchive_export.h
            ${KArchive_HEADERS}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF5}/KArchive
        COMPONENT Devel)

ecm_qt_install_logging_categories(
    EXPORT KARCHIVE
    FILE karchive.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if(BUILD_QCH)
    ecm_add_qch(
        KF5Archive_QCH
        NAME KArchive
        BASE_NAME KF5Archive
        VERSION ${KF5_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KArchive_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        LINK_QCHS
            Qt5Core_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KARCHIVE_EXPORT
            KARCHIVE_DEPRECATED
            "KARCHIVE_DEPRECATED_VERSION(x, y, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KArchive LIB_NAME KF5Archive DEPS "core" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF5}/KArchive)
install(FILES ${PRI_FILENAME}
        DESTINATION ${ECM_MKSPECS_INSTALL_DIR})

