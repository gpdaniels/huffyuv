# Define the minimum version of CMake that is required.
CMAKE_MINIMUM_REQUIRED(VERSION 3.5.1)
CMAKE_POLICY(VERSION 3.5.1)

# Project name.
SET(PROJECT_NAME "huffyuv")
PROJECT(${PROJECT_NAME})

# Print status.
MESSAGE(STATUS "Configuring ${PROJECT_NAME}...")

################################################################################

# Enable testing.
ENABLE_TESTING()

# Require c11.
SET(CMAKE_C_STANDARD 11)
SET(CMAKE_C_STANDARD_REQUIRED ON)
SET(CMAKE_C_EXTENSIONS OFF)

# Require c++17.
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CXX_EXTENSIONS OFF)

# Position independent code.
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Visual studio warning level.
SET(CMAKE_CXX_WARNING_LEVEL 4)
SET(CMAKE_C_WARNING_LEVEL 4)

# Enable grouping by folder in an IDE.
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################

# Check for the build type only.
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    SET(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build." FORCE)
    SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "" "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
ENDIF()

# Check for the configuration type.
IF(NOT CMAKE_CONFIGURATION_TYPES)
    # If there was no configuration type then set it to the build type.
    SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
ENDIF()

################################################################################

# Set compiler build flags.
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-newline-eof")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-id-macro")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-disabled-macro-expansion")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exit-time-destructors")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat-pedantic")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-newline-eof")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-reserved-id-macro")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-padded")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-disabled-macro-expansion")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-global-constructors")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-exit-time-destructors")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-exceptions")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wfatal-errors")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpedantic")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP8")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_HAS_EXCEPTIONS=0")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GR-")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ITERATOR_DEBUG_LEVEL=0")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_MATH_DEFINES=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_ENABLE_ATOMIC_ALIGNMENT_FIX=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_NONSTDC_NO_WARNINGS=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS=1")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler.")
ENDIF()

################################################################################

# Set compiler native flags.
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")
ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler.")
ENDIF()

################################################################################

# Set compiler linker flags.
IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    SET(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -latomic")
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
    # None
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # None
ELSEIF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    # None
ELSE()
    MESSAGE(FATAL_ERROR "Unsupported compiler.")
ENDIF()

################################################################################

# Find threading.
SET(THREADS_PREFER_PTHREAD_FLAG ON)
FIND_PACKAGE(Threads REQUIRED)

################################################################################

FIND_PROGRAM(FFMPEG ffmpeg)
IF(NOT FFMPEG)
    MESSAGE(FATAL_ERROR "ffmpeg not found!")
ENDIF()

# Copy samples to binay dir.
SET(SAMPLES
    "camera2_hfyu32.avi"
    "rgb24_interlaced.avi"
    "rgb_predgrad.avi"
    "rgb_predleft.avi"
    "rgb_predleftnodecorr.avi"
    "rgb_v1.avi"
    "yuv_predgrad.avi"
    "yuv_predleft.avi"
    "yuv_predmed.avi"
    "yuv_v1.avi"
)
FOREACH(SAMPLE ${SAMPLES})
    FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/samples")
    IF(NOT EXISTS "${CMAKE_BINARY_DIR}/samples/${SAMPLE}")
        MESSAGE(STATUS "Downloading sample '${SAMPLE}'...")
        FILE(DOWNLOAD "http://samples.mplayerhq.hu/V-codecs/HuffYUV/${SAMPLE}" "${CMAKE_BINARY_DIR}/samples/${SAMPLE}")
    ENDIF()
    FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/frames")
    IF(NOT EXISTS "${CMAKE_BINARY_DIR}/frames/${SAMPLE}/000000.ppm")
        MESSAGE(STATUS "Expanding sample '${SAMPLE}'...")
        FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/frames/${SAMPLE}")
        EXECUTE_PROCESS(COMMAND ${FFMPEG} "-i" "${CMAKE_BINARY_DIR}/samples/${SAMPLE}" "-start_number" "0" "${CMAKE_BINARY_DIR}/frames/${SAMPLE}/%06d.ppm")
    ENDIF()
ENDFOREACH()

################################################################################

INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/source")

################################################################################

ADD_EXECUTABLE(nothing
    "${CMAKE_SOURCE_DIR}/source/avi.hpp"
    "${CMAKE_SOURCE_DIR}/source/huffyuv.hpp"
    "${CMAKE_SOURCE_DIR}/tests/nothing.cpp"
)
ADD_TEST(NAME nothing COMMAND $<TARGET_FILE:nothing>)
SET_TESTS_PROPERTIES(nothing PROPERTIES TIMEOUT 10)

ADD_EXECUTABLE(validate_samples
    "${CMAKE_SOURCE_DIR}/source/avi.hpp"
    "${CMAKE_SOURCE_DIR}/source/huffyuv.hpp"
    "${CMAKE_SOURCE_DIR}/tests/ppm.hpp"
    "${CMAKE_SOURCE_DIR}/tests/samples.hpp"
    "${CMAKE_SOURCE_DIR}/tests/validate_samples.cpp"
)
ADD_TEST(NAME validate_samples COMMAND $<TARGET_FILE:validate_samples>)
SET_TESTS_PROPERTIES(validate_samples PROPERTIES TIMEOUT 10)

ADD_EXECUTABLE(encode_match_samples
    "${CMAKE_SOURCE_DIR}/source/avi.hpp"
    "${CMAKE_SOURCE_DIR}/source/huffyuv.hpp"
    "${CMAKE_SOURCE_DIR}/tests/ppm.hpp"
    "${CMAKE_SOURCE_DIR}/tests/samples.hpp"
    "${CMAKE_SOURCE_DIR}/tests/encode_match_samples.cpp"
)
ADD_TEST(NAME encode_match_samples COMMAND $<TARGET_FILE:encode_match_samples>)
SET_TESTS_PROPERTIES(encode_match_samples PROPERTIES TIMEOUT 10)

ADD_EXECUTABLE(decode_match_samples
    "${CMAKE_SOURCE_DIR}/source/avi.hpp"
    "${CMAKE_SOURCE_DIR}/source/huffyuv.hpp"
    "${CMAKE_SOURCE_DIR}/tests/ppm.hpp"
    "${CMAKE_SOURCE_DIR}/tests/samples.hpp"
    "${CMAKE_SOURCE_DIR}/tests/decode_match_samples.cpp"
)
ADD_TEST(NAME decode_match_samples COMMAND $<TARGET_FILE:decode_match_samples>)
SET_TESTS_PROPERTIES(decode_match_samples PROPERTIES TIMEOUT 10)

################################################################################

# Print status.
MESSAGE(STATUS "Finished configuring ${PROJECT_NAME}.")
