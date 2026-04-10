cmake_minimum_required(VERSION 3.20)
project(Legacy_PC_HighBar_PoC C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2 SDL2_ttf)

add_executable(legacy-pc-highbar-poc
    src/main.c
    src/screen.c
)

target_include_directories(legacy-pc-highbar-poc PRIVATE
    src
    ${SDL2_INCLUDE_DIRS}
)

target_compile_options(legacy-pc-highbar-poc PRIVATE ${SDL2_CFLAGS_OTHER})
target_link_libraries(legacy-pc-highbar-poc PRIVATE ${SDL2_LIBRARIES} m)
