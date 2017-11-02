set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake-modules)

set(SRCS

        format_detect.cpp
        format_detect.h
        main.cpp
        main.h
        secret/secret_config.cpp
        secret/secret_config.h
        secret/secret_import.cpp
        secret/secret_import.h
        secret/secret_input.cpp
        secret/secret_input.h
        session/cia_session.cpp
        session/cia_session.h
        session/exheader_session.cpp
        session/exheader_session.h
        session/ncch_session.cpp
        session/ncch_session.h
        session/ncsd_session.cpp
        session/ncsd_session.h
        session/romfs_hash_session.cpp
        session/romfs_hash_session.h
        session/romfs_session.cpp
        session/romfs_session.h
        session/rsa_session.cpp
        session/rsa_session.h
        session/session.cpp
        session/session.h
        session/sha_session.cpp
        session/sha_session.h
        session/smdh_session.cpp
        session/smdh_session.h
        tab_widget.cpp
        tab_widget.h
        util.h
        )

set(UIS
        secret/secret_config.ui
        secret/secret_import.ui
        secret/secret_input.ui
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
