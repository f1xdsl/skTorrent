add_executable(skTorrent main.cpp)

target_include_directories(skTorrent PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(skTorrent PRIVATE skTorrent_lib)
