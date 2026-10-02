add_executable(toygit_tests
    blob_test.cpp
    index_test.cpp
)

target_link_libraries(toygit_tests PRIVATE core Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(toygit_tests)
