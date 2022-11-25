cmake_minimum_required(VERSION 3.20)
project(chip8)


find_package(
    SFML
    COMPONENTS system window graphics
    CONFIG REQUIRED
)


add_subdirectory(src)
