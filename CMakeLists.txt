cmake_minimum_required(VERSION 3.14)
project(sysload VERSION 0.1.2 LANGUAGES C)

# Build options
option(SYSLOAD_BUILD_SHARED "Build shared library" ON)
option(SYSLOAD_BUILD_STATIC "Build static library" ON)
option(SYSLOAD_BUILD_EXAMPLE "Build example program" ON)
option(SYSLOAD_BUILD_TESTS "Build tests" ON)

# Sources
set(SYSLOAD_SRC src/sysload.c)
set(SYSLOAD_HEADERS include/sysload.h)

# Shared library
if(SYSLOAD_BUILD_SHARED)
    add_library(sysload_shared SHARED ${SYSLOAD_SRC})
    set_target_properties(sysload_shared PROPERTIES OUTPUT_NAME sysload)
    target_include_directories(sysload_shared PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Static library
if(SYSLOAD_BUILD_STATIC)
    add_library(sysload_static STATIC ${SYSLOAD_SRC})
    set_target_properties(sysload_static PROPERTIES OUTPUT_NAME sysload)
    target_include_directories(sysload_static PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Alias sysload
if(SYSLOAD_BUILD_SHARED)
    add_library(sysload ALIAS sysload_shared)
elseif(SYSLOAD_BUILD_STATIC)
    add_library(sysload ALIAS sysload_static)
endif()

# Example program
if(SYSLOAD_BUILD_EXAMPLE)
    add_executable(sysload_example example/main.c)
    target_link_libraries(sysload_example PRIVATE sysload)
endif()

# Tests
if(SYSLOAD_BUILD_TESTS)
    enable_testing()
    add_executable(sysload_test tests/test_sysload.c)
    target_link_libraries(sysload_test PRIVATE sysload)
    add_test(NAME sysload_test COMMAND sysload_test)
endif()

# Install
include(GNUInstallDirs)

install(FILES ${SYSLOAD_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(SYSLOAD_BUILD_SHARED)
    install(TARGETS sysload_shared
            EXPORT sysloadTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(SYSLOAD_BUILD_STATIC)
    install(TARGETS sysload_static
            EXPORT sysloadTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

install(EXPORT sysloadTargets
        FILE sysloadTargets.cmake
        NAMESPACE sysload::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/sysload)

install(CODE "execute_process(COMMAND /sbin/ldconfig \"${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}\" )")

# Uninstall
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
    COMMAND /sbin/ldconfig
)