# Copyright (C) 2021 Thomas Friedrich, Chu-Ping Yu, 
# University of Antwerp - All Rights Reserved. 
# You may use, distribute and modify
# this code under the terms of the GPL3 license.
# You should have received a copy of the GPL3 license with
# this file. If not, please visit: 
# https://www.gnu.org/licenses/gpl-3.0.en.html
# 
# Authors: 
#   Thomas Friedrich <thomas.friedrich@uantwerpen.be>
#   Chu-Ping Yu <chu-ping.yu@uantwerpen.be>
# 

cmake_minimum_required(VERSION 3.21.0)

project(RICOM LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

MESSAGE(STATUS "Using ${CMAKE_CXX_COMPILER_ID} Compiler!")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (UNIX)
    set(SDL2_INCLUDE_DIR /usr/include/SDL2)
    set(FFTW3_DIR /usr/include)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
    # SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    # SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    # SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif(UNIX)

if (WIN32)
    set(SDL2_INCLUDE_DIR C:/SDL2_VC/include/)
    set(SDL2_LIB_DIR C:/SDL2_VC/lib/x64)
    set(FFTW3_DIR C:/FFTW)
    set(BUILD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    MESSAGE(STATUS ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    link_directories(${SDL2_LIB_DIR})
    link_directories(${FFTW3_DIR})

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread -std=c++17")
        set(CMAKE_CXX_FLAGS_DEBUG "-g")
        set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
    endif()

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /nologo")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GL")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GA")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qpar")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /favor:INTEL64")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qstd:c++17 /Ox")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Qopt-assume-no-loop-carried-dep=2")
        # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /QxKABYLAKE")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
        set(MMD_LIB C:/Program\ Files\ \(x86\)/Intel/oneAPI/compiler/latest/windows/redist/intel64_win/compiler)
        set(BUILD_DIR ${PROJECT_BINARY_DIR})
        file(COPY ${MMD_LIB}/libmmd.dll DESTINATION ${BUILD_DIR})
        file(COPY ${MMD_LIB}/libmmdd.dll DESTINATION ${BUILD_DIR})
        file(COPY ${MMD_LIB}/svml_dispmd.dll DESTINATION ${BUILD_DIR})
    endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")

    file(COPY ${SDL2_LIB_DIR}/SDL2.dll DESTINATION ${BUILD_DIR})
    file(COPY ${SDL2_LIB_DIR}/SDL2_image.dll DESTINATION ${BUILD_DIR})
    file(COPY ${FFTW3_DIR}/libfftw3f-3.dll DESTINATION ${BUILD_DIR})
    file(COPY ${FFTW3_DIR}/libfftw3f-3.lib DESTINATION ${BUILD_DIR})
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /STACK:10485760")
endif (WIN32)

add_executable(RICOM src/main.cpp)

target_sources( RICOM PRIVATE
imgui/misc/cpp/imgui_stdlib.cpp
imgui/imgui_draw.cpp
imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_sdl.cpp
    imgui/imgui.cpp
    src/SocketConnector.cpp
    src/FileConnector.cpp
    src/ProgressMonitor.cpp
    src/Ricom.cpp 
    src/cameras/CheetahInterface.cpp
    src/cameras/CheetahWrapper.cpp
    src/cameras/TimepixInterface.cpp
    src/cameras/TimepixWrapper.cpp
    src/cameras/MerlinInterface.cpp
    src/cameras/MerlinWrapper.cpp
    src/Camera.cpp
    src/main.cpp
    src/GuiUtils.cpp
    src/SdlImageWindow.cpp
    src/ImGuiImageWindow.cpp
    src/RunCLI.cpp
    src/RunGUI.cpp
    )

## cpr
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                            GIT_TAG 0817715923c9705e68994eb52ef9df3f6845beba) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
set(CPR_ENABLE_SSL OFF)
FetchContent_MakeAvailable(cpr)
target_link_libraries(RICOM PRIVATE cpr::cpr)
##json
# include(FetchContent)
# FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
# FetchContent_MakeAvailable(json)
# target_link_libraries(RICOM PRIVATE nlohmann_json::nlohmann_json)
set(JSON_BuildTests OFF CACHE INTERNAL "")

target_include_directories( RICOM PRIVATE
    imgui
    imgui/backends
    imgui/misc/cpp
    ${SDL2_INCLUDE_DIR}
    ${FFTW3_DIR}
    include
    src
    src/cameras
    )

find_package(OpenGL REQUIRED)



target_link_libraries(RICOM PRIVATE ${OPENGL_LIBRARIES})
if (WIN32)
    target_link_libraries(RICOM PRIVATE SDL2main SDL2 SDL2_image libfftw3f-3 ${CMAKE_DL_LIBS})
    add_custom_command(TARGET RICOM POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy -t $<TARGET_FILE_DIR:RICOM> $<TARGET_RUNTIME_DLLS:RICOM>
    COMMAND_EXPAND_LISTS
    )
else ()
    target_link_libraries(RICOM PRIVATE SDL2main SDL2 SDL2_image fftw3f ${CMAKE_DL_LIBS})
endif (WIN32)