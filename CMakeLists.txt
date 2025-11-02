cmake_minimum_required(VERSION 3.14)
project(sysload VERSION 0.1.0 LANGUAGES C)

# Build options
option(SYSLOAD_BUILD_SHARED "Build shared library" ON)
option(SYSLOAD_BUILD_STATIC "Build static library" ON)
option(SYSLOAD_BUILD_EXAMPLE "Build example program" ON)
option(SYSLOAD_BUILD_TESTS "Build tests" ON)

# Library sources
set(SYSLOAD_SRC src/sysload.c)
set(SYSLOAD_HEADERS include/sysload.h)

# Shared lib
if(SYSLOAD_BUILD_SHARED)
        add_library(sysload_shared SHARED ${SYSLOAD_SRC})
        target_include_directories(sysload_shared PUBLIC include)
        set_target_properties(sysload_shared PROPERTIES OUTPUT_NAME sysload)
endif()

# Static lib
if(SYSLOAD_BUILD_STATIC)
        add_library(sysload_static STATIC ${SYSLOAD_SRC})
        target_include_directories(sysload_static PUBLIC include)
        set_target_properties(sysload_static PROPERTIES OUTPUT_NAME sysload)
endif()

# Example
if(SYSLOAD_BUILD_EXAMPLE)
        add_executable(sysload_example example/main.c)
        target_link_libraries(sysload_example PRIVATE sysload_static)
        target_include_directories(sysload_example PRIVATE include)
endif()

# Tests
if(SYSLOAD_BUILD_TESTS)
        enable_testing()
        add_executable(sysload_test tests/test_sysload.c)
        target_link_libraries(sysload_test PRIVATE sysload_static)
        target_include_directories(sysload_test PRIVATE include)
        add_test(NAME sysload_test COMMAND sysload_test)
endif()

# Installation paths
include(GNUInstallDirs)

install(FILES ${SYSLOAD_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(SYSLOAD_BUILD_SHARED)
        install(TARGETS sysload_shared DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()
if(SYSLOAD_BUILD_STATIC)
        install(TARGETS sysload_static DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

# Uninstall target
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
        IMMEDIATE @ONLY
)

add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
)