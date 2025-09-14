add_library(Logger INTERFACE)

set(LOG_DIR "${CMAKE_BINARY_DIR}/logs")
file(MAKE_DIRECTORY "${LOG_DIR}")

target_compile_definitions(Logger INTERFACE LOG_DEFAULT_DIR="${LOG_DIR}")
target_include_directories(Logger INTERFACE ${CMAKE_CURRENT_LIST_DIR})
target_sources(Logger INTERFACE ${CMAKE_CURRENT_LIST_DIR}/Logger.hpp)
