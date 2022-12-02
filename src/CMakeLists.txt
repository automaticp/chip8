file(GLOB chip8_sources *.cpp)
add_executable(chip8 ${chip8_sources})

target_compile_features(chip8 PRIVATE cxx_std_20)
target_include_directories(chip8 PRIVATE .)
target_link_libraries(chip8 PRIVATE sfml-system sfml-graphics sfml-window fmt::fmt)
