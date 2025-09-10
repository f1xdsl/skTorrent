find_package(GTest REQUIRED)


macro(AddTest TEST_FILE)
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    string(REGEX REPLACE "Test$" "" TEST_TARGET ${TEST_NAME})

    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} PRIVATE skTorrent_lib GTest::gtest_main)

    foreach(dep IN LISTS ARGN)
        find_package(${dep} REQUIRED)
        if(TARGET ${dep}::${dep})
            target_link_libraries(${TEST_NAME} PRIVATE ${dep}::${dep})
        elseif(TARGET ${dep})
            target_link_libraries(${TEST_NAME} PRIVATE ${dep})
        else()
            message(WARNING "Dependency '${dep}' was found, but no matching target to link")
        endif()
    endforeach()

    # add_test(NAME ${TEST_TARGET} COMMAND ${TEST_NAME})
endmacro(AddTest)

AddTest("BencodeTest.cpp")
AddTest("TorrentMetaTest.cpp")
