cmake_minimum_required(VERSION 3.4.0)

find_package(Boost REQUIRED COMPONENTS thread system program_options date_time)

if(Boost_FOUND)
    message("Boost_FOUND !!!!")
    message("Boost_INCLUDE_DIRS = '${Boost_INCLUDE_DIRS}'")
    message("Boost_LIBRARIES = '${Boost_LIBRARIES}'")
else()
    message(FATAL_ERROR "Boost NOT FOUND")
endif()

add_subdirectory(parser)

add_executable(${PROJECT_NAME}
    ${CMAKE_CURRENT_SOURCE_DIR}/CsvParser.cpp
    )

target_link_libraries(${PROJECT_NAME}
    Boost::boost
    Boost::program_options
    Boost::thread
    Boost::system
    Boost::date_time
    parser
    )
