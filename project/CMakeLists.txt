# worms - ReXGlue Recompiled Project
#
# SDK boilerplate lives in generated/rexglue.cmake.

cmake_minimum_required(VERSION 3.25)
project(worms LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(generated/rexglue.cmake)

# Sources
set(WORMS_SOURCES
    src/main.cpp
    src/stubs.cpp
)

# ponytail: temporary bring-up harvest. Tolerant indirect dispatcher logs every
# unregistered computed-call target (instead of fataling on the first) so one
# play-through harvests them all for batch registration. Toggle OFF for release.
option(WORMS_HARVEST "Build with tolerant-dispatch harvest scaffold" OFF)
if(WORMS_HARVEST)
    list(APPEND WORMS_SOURCES src/dispatch_tolerance.cpp)
endif()

if(WIN32)
    add_executable(worms WIN32 ${WORMS_SOURCES})
else()
    add_executable(worms ${WORMS_SOURCES})
endif()

if(WORMS_HARVEST AND WIN32)
    target_link_options(worms PRIVATE "LINKER:/force:multiple")
endif()

rexglue_setup_target(worms)
