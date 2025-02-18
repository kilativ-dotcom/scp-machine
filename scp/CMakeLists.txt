set(SCP_SRC "${CMAKE_SOURCE_DIR}/scp")

file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

list(FILTER SOURCES EXCLUDE REGEX ".*/tests/.*")
list(FILTER HEADERS EXCLUDE REGEX ".*/tests/.*")

set(CMAKE_C_FLAGS_DEBUG "-DSCP_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-DSCP_DEBUG")
add_definitions(-DSCP_MATH)
add_definitions(-DSCP_STRING)

add_library(scp SHARED ${SOURCES} ${HEADERS})
target_link_libraries(scp
    LINK_PUBLIC sc-core
    LINK_PUBLIC sc-memory
    LINK_PUBLIC sc-kpm-common
)

target_include_directories(scp
    PUBLIC ${SCP_SRC}
    PUBLIC ${SC_MEMORY_PATH}
    PUBLIC ${SC_CORE_PATH}
)

if (${SCP_CLANG_FORMAT_CODE})
    target_clangformat_setup(scp)
endif()

if (${SC_BUILD_TESTS})
    include(${CMAKE_CURRENT_LIST_DIR}/tests/tests.cmake)
endif ()
