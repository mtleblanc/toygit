add_library(core STATIC
    commit.cpp
    core.cpp
    zlib.cpp
)

target_include_directories(core PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(core PUBLIC ZLIB::ZLIB OpenSSL::Crypto)
