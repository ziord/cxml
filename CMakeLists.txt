# ===============================================
# Copyright © 2021 Jeremiah Ikosin
# Distributed under the terms of the MIT license.
# ===============================================


# ===============================================
#
# CXML CMake Configurations
#
# ===============================================

cmake_minimum_required(VERSION 3.15)
project(cxml
        VERSION 0.1.0
        DESCRIPTION "An XML library for C with a focus on simplicity and ease of use."
        HOMEPAGE_URL "https://github.com/ziord/cxml"
        LANGUAGES C)

if (NOT CMAKE_C_STANDARD)
        message(WARNING "'CMAKE_C_STANDARD' was not set. This will be automatically set to C11.")
        set(CMAKE_C_STANDARD 11)
elseif(NOT CMAKE_C_STANDARD EQUAL 11)
        message(WARNING "CXML requires C11. 'CMAKE_C_STANDARD' was set to ${CMAKE_C_STANDARD}. "
                "This will be automatically set to C11.")
        set(CMAKE_C_STANDARD 11)
endif()
set(CMAKE_C_STANDARD_REQUIRED  ON)


# ===============================================
#
#   CXML Configuration Options
#
# ===============================================

option(CXML_BUILD_SHARED_LIB "Build cxml as a shared library" OFF)
option(CXML_BUILD_TESTS "Build tests" OFF)
option(CXML_USE_SAX_MOD "Build cxml with SAX features included" OFF)
option(CXML_USE_XPATH_MOD "Build cxml with XPATH features included" OFF)
option(CXML_USE_QUERY_MOD "Build cxml with the query language, and API features included" OFF)


# ===============================================
#
#   CXML Configuration Options Logging
#
# ===============================================

message(STATUS "[cxml] Build cxml with SAX features included: " ${CXML_USE_SAX_MOD})
message(STATUS "[cxml] Build cxml with XPATH features included: " ${CXML_USE_XPATH_MOD})
message(STATUS "[cxml] Build cxml with the query language, and API features included: " ${CXML_USE_QUERY_MOD})
message(STATUS "[cxml] Build tests: " ${CXML_BUILD_TESTS})
message(STATUS "[cxml] Build as a shared library: " ${CXML_BUILD_SHARED_LIB})


# ===============================================
#
#   CXML Source Files
#
# ===============================================

# add the most minimum required files for the library to at least work.

file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "src/core/*.c")
file(GLOB XML CONFIGURE_DEPENDS "src/xml/*.c")
file(GLOB UTILS CONFIGURE_DEPENDS "src/utils/*.c")
file(GLOB CXML CONFIGURE_DEPENDS "src/cxml/*.c")
list(APPEND CXML_SOURCES ${CORE_SOURCES} ${XML} ${UTILS} ${CXML})

list(APPEND CXML_BUILD_OPTIONS)
list(APPEND CXML_INCLUDES "include/cxml" "include/core" "include/utils" "include/xml")

if (CXML_USE_QUERY_MOD)
        file(GLOB QUERY CONFIGURE_DEPENDS "src/query/*.c")
        list(APPEND CXML_SOURCES ${QUERY})
        list(APPEND CXML_BUILD_OPTIONS "CXML_USE_QUERY_MOD")
        list(APPEND CXML_INCLUDES "include/query")
endif ()

if (CXML_USE_SAX_MOD)
        file(GLOB SAX CONFIGURE_DEPENDS "src/sax/*.c")
        list(APPEND CXML_SOURCES ${SAX})
        list(APPEND CXML_BUILD_OPTIONS "CXML_USE_SAX_MOD")
        list(APPEND CXML_INCLUDES "include/sax")
endif()

if (CXML_USE_XPATH_MOD)
        file(GLOB XPATH CONFIGURE_DEPENDS "src/xpath/*.c")
        list(APPEND CXML_SOURCES ${XPATH})
        list(APPEND CXML_BUILD_OPTIONS "CXML_USE_XPATH_MOD")
        list(APPEND CXML_INCLUDES "include/xpath")
endif()


# ===============================================
#
#   CXML Library Setup
#
# ===============================================

#message(STATUS "All sources: " "${CXML_SOURCES}")

if(CXML_BUILD_SHARED_LIB)
        message(STATUS "Building as a shared lib")
        add_library(cxml SHARED ${CXML_SOURCES})
else()
        message(STATUS "Building as a static lib")
        add_library(cxml STATIC ${CXML_SOURCES})
endif()

if (MSVC)
        list(APPEND CXML_WARN_FLAGS "/W4")
else()
        # -Wno-unused-variable -> helps with cxml_for_each(){} loop macro construct,
        # since the compiler assumes that the inner variables driving the loop isn't used, which is wrong.
        list(APPEND CXML_WARN_FLAGS "-Wall" "-Wextra" "-Wno-unused-but-set-variable" "-pedantic")
endif()

include(CheckLibraryExists)
check_library_exists(m fmax "" LIBM_EXISTS)

target_include_directories(cxml PUBLIC include)
target_compile_options(cxml PRIVATE ${CXML_WARN_FLAGS})
target_compile_definitions(cxml PUBLIC ${CXML_BUILD_OPTIONS})

if (LIBM_EXISTS)
        target_link_libraries(cxml m)
endif()


# ===============================================
#
#   CXML Tests Setup
#
# ===============================================

message(STATUS "BUILD options:" "${CXML_BUILD_OPTIONS}")
if (CXML_BUILD_TESTS)
        # get the minimum required test source files for the tests to be performed
        file(GLOB CORE_TEST CONFIGURE_DEPENDS "tests/core/*.c")
        file(GLOB UTILS_TEST CONFIGURE_DEPENDS "tests/utils/*.c")
        file(GLOB XML_TEST CONFIGURE_DEPENDS "tests/xml/*.c")
        list(APPEND CXML_TEST_SOURCES ${CORE_TEST} ${UTILS_TEST} ${XML_TEST})
        list(APPEND CXML_TEST_SOURCES "tests/cxfixture.c")

        if (CXML_USE_QUERY_MOD)
                file(GLOB QUERY_TEST CONFIGURE_DEPENDS "tests/query/*.c")
                list(APPEND CXML_TEST_SOURCES ${QUERY_TEST})
        endif ()
        if (CXML_USE_SAX_MOD)
                file(GLOB SAX_TEST CONFIGURE_DEPENDS "tests/sax/*.c")
                list(APPEND CXML_TEST_SOURCES ${SAX_TEST})
        endif ()
        if (CXML_USE_XPATH_MOD)
                file(GLOB XPATH_TEST CONFIGURE_DEPENDS "tests/xpath/*.c")
                list(APPEND CXML_TEST_SOURCES ${XPATH_TEST})
        endif ()
        add_executable(cxml_tests
                tests/test.c
                ${CXML_TEST_SOURCES})
        target_compile_options(cxml_tests PRIVATE ${CXML_WARN_FLAGS} "-Wno-unused-function")
        target_compile_definitions(cxml_tests PRIVATE ${CXML_BUILD_OPTIONS})
        target_link_libraries(cxml_tests PRIVATE cxml)
        # Add include directories (tests) to a target (cxml_tests).
        target_include_directories(cxml_tests PRIVATE tests)
        add_custom_target(
                full_test
                COMMENT "Run tests"
                COMMAND ${CMAKE_CURRENT_BINARY_DIR}/cxml_tests
                DEPENDS cxml_tests
                VERBATIM)
endif ()
install(TARGETS cxml DESTINATION cxml/lib)
install(DIRECTORY ${CXML_INCLUDES} DESTINATION cxml/include)

# credits: lamarrr/stx
