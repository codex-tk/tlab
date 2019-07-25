project(tlab.test)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../tlab/include)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

add_executable(tlab.test ${SRCS})

find_package(ZLIB)
if (ZLIB_FOUND)
    target_link_libraries(tlab.test ${ZLIB_LIBRARIES})
endif(ZLIB_FOUND)

if(Boost_FOUND)
    add_definitions(-DBOOST_LOG_DYN_LINK)
    MESSAGE(${Boost_LIBRARIES})
    target_link_libraries(tlab.test ${Boost_LIBRARIES})
endif()

target_link_libraries(tlab.test tlab)
if(WIN32)
    target_link_libraries(tlab.test ws2_32)
else()
    target_link_libraries(tlab.test dl)
endif()

enable_testing()
add_test(tlab.test tlab.test)
add_custom_command(TARGET tlab.test POST_BUILD COMMAND ctest -C $<CONFIGURATION> --verbose --output-on-failure )
