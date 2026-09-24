cmake_minimum_required(VERSION 3.28)

project(ws-error
    VERSION 0.1.0
    DESCRIPTION "Universal C++23 error contract: strongly-typed Error<Code> for std::expected"
    LANGUAGES CXX)

# ---------------------------------------------------------------------------
# Header-only. The target is INTERFACE so consumers get the include path and the
# C++23 requirement and nothing else — no archive, no link step, no dependency.
# ---------------------------------------------------------------------------
add_library(ws-error INTERFACE)
add_library(ws-error::ws-error ALIAS ws-error)

target_include_directories(ws-error INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

target_compile_features(ws-error INTERFACE cxx_std_23)

# ---------------------------------------------------------------------------
# Tests. Built only when this library is the top-level project, so consuming it
# through FetchContent never drags our test binary into someone else's build.
# ---------------------------------------------------------------------------
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(WS_ERROR_STANDALONE ON)
else()
    set(WS_ERROR_STANDALONE OFF)
endif()

option(WS_ERROR_BUILD_TESTS "Build ws-error's standalone test suite" ${WS_ERROR_STANDALONE})

if(WS_ERROR_BUILD_TESTS)
    enable_testing()

    add_executable(ws-error-tests tests/test_error.cpp)
    target_link_libraries(ws-error-tests PRIVATE ws-error)
    target_compile_options(ws-error-tests PRIVATE -Wall -Wextra -Werror)
    set_target_properties(ws-error-tests PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED ON)

    add_test(NAME ws-error.all COMMAND ws-error-tests)
endif()
