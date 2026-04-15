add_executable(toygit toygit/main.cpp)
target_link_libraries(toygit PRIVATE core)

add_executable(tgit-inflate toygit/inflate.cpp)
target_link_libraries(tgit-inflate PRIVATE core)
