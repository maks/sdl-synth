cmake_minimum_required(VERSION 3.13)

project(sdlsynth C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(sdlsynth src/synth.cpp)
target_link_libraries(sdlsynth ${SDL2_LIBRARIES})



# add_executable(sdlsynth
#     src/synth.cpp
#     amy/algorithms.c
#     amy/amy.c
#     amy/delay.c
#     amy/envelope.c
#     amy/filters.c
#     amy/patches.c
#     amy/log2_exp2.c
#     amy/examples.c
#     amy/oscillators.c
#     amy/partials.c
#     amy/pcm.c
# )

# target_link_libraries(

# )

