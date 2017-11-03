set(SRCS
        aes_key.cpp
        aes_key.h
        align.h
        common_types.h
        container_backend/cia.cpp
        container_backend/cia.h
        container_backend/container.cpp
        container_backend/container.h
        container_backend/disk_directory.cpp
        container_backend/disk_directory.h
        container_backend/exefs.cpp
        container_backend/exefs.h
        container_backend/exheader.cpp
        container_backend/exheader.h
        container_backend/romfs.cpp
        container_backend/romfs.h
        container_backend/rsa.cpp
        container_backend/rsa.h
        container_backend/sha.cpp
        container_backend/sha.h
        container_backend/sd_protected.cpp
        container_backend/sd_protected.h
        container_backend/smdh.cpp
        container_backend/smdh.h
        container_backend/ncch.cpp
        container_backend/ncch.h
        container_backend/ncsd.cpp
        container_backend/ncsd.h
        cryptopp_util.h
        file_backend/aes_cbc.cpp
        file_backend/aes_cbc.h
        file_backend/aes_ctr.cpp
        file_backend/aes_ctr.h
        file_backend/disk_file.cpp
        file_backend/disk_file.h
        file_backend/file.cpp
        file_backend/file.h
        file_backend/memory_file.cpp
        file_backend/memory_file.h
        file_backend/patch_file.cpp
        file_backend/patch_file.h
        file_backend/sub_file.cpp
        file_backend/sub_file.h
        secret_backend/bootrom.cpp
        secret_backend/bootrom.h
        secret_backend/movable_sed.cpp
        secret_backend/movable_sed.h
        secret_backend/secret_database.cpp
        secret_backend/secret_database.h
        string_util.h
        )


create_directory_groups(${SRCS})
add_library(core STATIC ${SRCS})
target_link_libraries(core PRIVATE cryptopp)
target_link_libraries(core PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)
if (CMAKE_COMPILER_IS_GNUCC)
    target_link_libraries(core PRIVATE stdc++fs)
endif()

