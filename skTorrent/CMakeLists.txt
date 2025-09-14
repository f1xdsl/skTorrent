file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_library(skTorrent_lib
    ${HEADERS}
    ${SOURCES}
)

find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

target_include_directories(skTorrent_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(skTorrent_lib PUBLIC Logger)
target_link_libraries(skTorrent_lib PRIVATE OpenSSL::Crypto CURL::libcurl)
