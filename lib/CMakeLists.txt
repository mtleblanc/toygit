add_library(core STATIC
    add_command.cpp
    commit.cpp
    core.cpp
    detail/object.cpp
    error.cpp
    dircache.cpp
    file.cpp
    hash.cpp
    lockfile.cpp
    repository.cpp
    status_command.cpp
    tree.cpp
    zlib.cpp
)

target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC ZLIB::ZLIB OpenSSL::Crypto)
