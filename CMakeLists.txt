cmake_minimum_required(VERSION 3.17.0)

project(RICOM LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

MESSAGE(STATUS "Using ${CMAKE_CXX_COMPILER_ID} Compiler!")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

if (UNIX)
    set(SDL2_INCLUDE_DIR /usr/include/SDL2)
    set(FFTW3_INCLUDE_DIR /usr/include)
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-Ofast")
endif(UNIX)

if (WIN32)
    set(SDL2_INCLUDE_DIR C:/SDL2_VC/include/)
    set(SDL2_LIB_DIR C:/SDL2_VC/lib/x64)
    set(BUILD_DIR ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    MESSAGE(STATUS ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    link_directories(${SDL2_LIB_DIR})

    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
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
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /QxKABYLAKE")
        set(MMD_LIB C:/Program\ Files\ \(x86\)/Intel/oneAPI/compiler/latest/windows/redist/intel64_win/compiler)
        set(BUILD_DIR ${PROJECT_BINARY_DIR})
        file(COPY ${MMD_LIB}/libmmd.dll DESTINATION ${BUILD_DIR})
        file(COPY ${MMD_LIB}/svml_dispmd.dll DESTINATION ${BUILD_DIR})
    endif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "IntelLLVM")

    file(COPY ${SDL2_LIB_DIR}/SDL2.dll DESTINATION ${BUILD_DIR})
    file(COPY ${SDL2_LIB_DIR}/SDL2_image.dll DESTINATION ${BUILD_DIR})

endif (WIN32)


add_executable(RICOM src/main.cpp)

target_sources( RICOM PRIVATE
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_sdl.cpp
    src/progress_bar.cpp
    src/TimpixInterface.cpp
    src/Ricom.cpp 
    src/main.cpp
    )

target_include_directories( RICOM PUBLIC
    imgui
    imgui/backends
    ${SDL2_INCLUDE_DIR}
    ${FFTW3_INCLUDE_DIR}
    include
    src
    )

find_package(OpenGL REQUIRED)
target_link_libraries(RICOM PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries(RICOM PUBLIC SDL2main SDL2 SDL2_image ${CMAKE_DL_LIBS})
target_link_libraries(RICOM PUBLIC fftw3)
