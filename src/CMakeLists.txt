cmake_minimum_required(VERSION 3.13)

project(sdlsynth C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Get SDL
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# add_subdirectory(amy)

add_executable(sdlsynth
    # synth.cpp
    testsynth2.cpp
    visualiser.cpp
    visualiser.h
    # tinysynth/tinysynth.cpp
    # tinysynth/tinysynth.h
    tinysynth2/tinysynth2.cpp
    tinysynth2/tinysynth2.h
    tinysynth2/osc.h
    tinysynth2/osc.cpp
    tinysynth2/utils.h
    tinysynth2/adsr.h
    tinysynth2/adsr.cpp
)

add_executable(lut
    lut.cpp
)

add_executable(lut8
    lut8.cpp
)

target_link_libraries(sdlsynth 
    PUBLIC ${SDL2_LIBRARIES} 
)

