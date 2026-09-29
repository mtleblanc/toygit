add_library(core STATIC
    commit.cpp
    core.cpp
    detail/object.cpp
    dircache.cpp
    file.cpp
    hash.cpp
    lockfile.cpp
    repository.cpp
    tree.cpp
    zlib.cpp
)

target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC ZLIB::ZLIB OpenSSL::Crypto)
