set(SRCS
        aes_cbc.cpp
        aes_cbc.h
        aes_ctr.cpp
        aes_ctr.h
        aes_key.cpp
        aes_key.h
        align.h
        cia.cpp
        cia.h
        common_types.h
        container.cpp
        container.h
        cryptopp_util.h
        disk_file.cpp
        disk_file.h
        exefs.cpp
        exefs.h
        exheader.cpp
        exheader.h
        file.cpp
        file.h
        memory_file.cpp
        memory_file.h
        ncch.cpp
        ncch.h
        ncsd.cpp
        ncsd.h
        romfs.cpp
        romfs.h
        rsa.cpp
        rsa.h
        secret_database.cpp
        secret_database.h
        sha.cpp
        sha.h
        smdh.cpp
        smdh.h
        sub_file.cpp
        sub_file.h
        )


create_directory_groups(${SRCS})
add_library(core STATIC ${SRCS})
target_link_libraries(core PRIVATE cryptopp)
target_link_libraries(core PRIVATE ${PLATFORM_LIBRARIES} Threads::Threads)

