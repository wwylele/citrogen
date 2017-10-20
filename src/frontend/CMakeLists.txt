set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake-modules)

set(SRCS
        cia_session.cpp
        cia_session.h
        exheader_session.cpp
        exheader_session.h
        format_detect.cpp
        format_detect.h
        main.cpp
        main.h
        ncch_session.cpp
        ncch_session.h
        ncsd_session.cpp
        ncsd_session.h
        romfs_hash_session.cpp
        romfs_hash_session.h
        romfs_session.cpp
        romfs_session.h
        rsa_session.cpp
        rsa_session.h
        secret_config.cpp
        secret_config.h
        secret_import.cpp
        secret_import.h
        secret_input.cpp
        secret_input.h
        session.cpp
        session.h
        sha_session.cpp
        sha_session.h
        smdh_session.cpp
        smdh_session.h
        tab_widget.cpp
        tab_widget.h
        util.h
        )

set(UIS
        secret_config.ui
        secret_import.ui
        secret_input.ui
        )

qt5_wrap_ui(UI_HDRS ${UIS})

create_directory_groups(${SRCS} ${UIS})

file(GLOB_RECURSE ICONS ${CMAKE_SOURCE_DIR}/dist/icons/*)

add_executable(citrogen ${SRCS} ${UI_HDRS} ${ICONS})
target_link_libraries(citrogen PRIVATE core)
target_link_libraries(citrogen PRIVATE Qt5::Widgets)
target_link_libraries(citrogen PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

if (MSVC)
    include(CopyQt5Deps)
    copy_Qt5_deps(citrogen)
endif()
