#pragma once
#include "Chip8.hpp"


namespace debug {


// Print the framebuffer (draw) in the console
void print_fb(const Chip8::framebuffer_t& fb, Short opcode);

// Print the state of the hexadecimal keypad
void print_keypad(const std::array<Byte, 16u>& keypad);

// Print extensive information about the current state
// of the chip8: registers, pc, current opcode (disassemble)
void pretty_print_state(const Chip8& c8);


}

