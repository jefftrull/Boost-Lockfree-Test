# Build configuration file for testing the proposed Lockfree Boost library
# 2011-05-18 Jeff Trull <jetrull@sbcglobal.net>
# you need to define BOOST_LOCKFREE_DIR to point to the library to be tested

cmake_minimum_required(VERSION 2.6)

project(lockfree_test)

find_package(Boost REQUIRED COMPONENTS thread program_options)

include_directories(${Boost_INCLUDE_DIR} ${BOOST_LOCKFREE_DIR})

file(GLOB CPP_FILES *.cpp)
foreach(CPP ${CPP_FILES})
    get_filename_component(TGT ${CPP} NAME_WE)
    add_executable(${TGT} WIN32 ${CPP})
    target_link_libraries(${TGT} ${Boost_LIBRARIES})
endforeach()
