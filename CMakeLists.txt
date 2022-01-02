cmake_minimum_required(VERSION 3.21.2)
project(OmniaSDB CXX)

file (STRINGS "build.nr" BUILD_NUMBER)
set(CMAKE_CXX_STANDARD 17)
 
list(APPEND sources
    ${CMAKE_CURRENT_LIST_DIR}/src/database.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/reader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/types.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/writer.cpp
)

list(APPEND headers
    ${CMAKE_CURRENT_LIST_DIR}/src/sdb/database.h
    ${CMAKE_CURRENT_LIST_DIR}/src/sdb/reader.h
    ${CMAKE_CURRENT_LIST_DIR}/src/sdb/types.h
    ${CMAKE_CURRENT_LIST_DIR}/src/sdb/writer.h
    ${CMAKE_CURRENT_LIST_DIR}/src/sdb/sdb.h
)

list(APPEND frontend_test_src
    ${CMAKE_CURRENT_LIST_DIR}/frontend-test/main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/frontend-test/tile.cpp
)

add_library(omnia-sdb SHARED ${sources})
add_executable(frontend-test ${frontend_test_src})

add_compile_options(-m32 -MMD -MP -Wall -ggdb)

target_compile_definitions(omnia-sdb PUBLIC __DEBUG__ __BUILD_NUMBER__=${BUILD_NUMBER} __MAJOR_VER__=0 __MINOR_VER__=1)

set(CMAKE_BUILD_TYPE Debug)
include_directories(./src)

target_include_directories(frontend-test PUBLIC ./src)
target_include_directories(omnia-sdb PUBLIC ./src/sdb)

target_link_libraries(frontend-test omnia-sdb)

add_custom_command(TARGET omnia-sdb POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_directory
	${CMAKE_SOURCE_DIR}/extra/ $<TARGET_FILE_DIR:omnia-sdb>
VERBATIM
)

if (WIN32)
	add_custom_command ( OUTPUT build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/inc_bnr.exe
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	)
endif (WIN32)
if (UNIX)
	add_custom_command ( OUTPUT build.nr
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/inc_bnr
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
	)
endif (UNIX)

add_custom_target(
  IncBnr ALL
  DEPENDS build.nr
)

install(TARGETS omnia-sdb DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Build/bin)
install(FILES ${headers} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/Build/include/sdb)
