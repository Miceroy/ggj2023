# Specify minimum supported cmake version
cmake_minimum_required(VERSION 3.10)

# Project GGJ2023Game
project(GGJ2023Game)

##
## Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add hungerland engine
option(HUNGERLAND_BUILD_EXAMPLES "Build hungerland examples" OFF)
add_subdirectory("ext/hungerland")

include_directories("include")

file(GLOB_RECURSE GAME_INC_FILES "./include/*.h")

## GGJ2023Game executable
add_executable(GGJ2023CarGame src/car_game_main.cpp ${GAME_INC_FILES})
target_link_libraries(GGJ2023CarGame hungerland)
add_custom_command(TARGET GGJ2023CarGame POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory
	${PROJECT_SOURCE_DIR}/assets
	${PROJECT_BINARY_DIR}/assets
	COMMENT "Copying game asset files to binary directory")

