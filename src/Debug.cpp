#include "Debug.hpp"

#include "Chip8.hpp"
#include <fmt/format.h>
#include <string_view>





void debug::print_fb(const Chip8::framebuffer_t& fb, Short opcode) {

    auto to_hex_char = [](size_t i) {
        return "0123456789ABCDEF"[i % 0x10];
    };

    fmt::print("\n{:#06x}\n", (opcode));

    const auto& fb_width = Chip8Base::fb_width;
    std::string line_buf(fb_width, ' ');
    fmt::print("   0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n\n");
    for (size_t line{ 0 }; line < Chip8Base::fb_height; ++line) {
        for (size_t col{ 0 }; col < line_buf.size(); ++col) {
            size_t pos{ line * fb_width + col };
            line_buf[col] = fb[pos] ? 'X' : '.';
        }
        fmt::print("{:2} {}\n", to_hex_char(line), line_buf);
    }
}





void debug::print_keypad(const std::array<Byte, 16u>& keypad) {
    thread_local std::array<char, 16u> buffer{};
    assert(keypad.size() == buffer.size());
    fmt::print("0123456789ABCDEF\n");
    for (size_t i{ 0 }; i < buffer.size(); ++i) {
        buffer[i] = keypad[i] ? 'X' : '.';
    }
    std::string_view sv{ buffer.begin(), buffer.end() };
    fmt::print("{}\n", sv);
}





void debug::pretty_print_state(const Chip8& c8) {

    Short opcode = c8.get_opcode();

    struct OpcodeInfo {
        std::string_view name{ "???" };
        std::string_view pattern{ "" };
        std::string_view desc{ "" };
    };

    OpcodeInfo info{};

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0:
                    // 00E0 - Clear the screen
                    info = { "CLS", "00E0", "Clear the screen" };
                    break;
                case 0x00EE:
                    // 00EE - Return from subroutine
                    info = { "RET", "00EE", "Return from subroutine" };
                    break;
                default:
                    break;
            }
            break;

        case 0x1000:
            // 1NNN - Jump to address NNN
            info = { "JUMP", "1NNN", "Jump to address NNN" };
            break;

        case 0x2000:
            // 2NNN - Call subroutine at NNN
            info = { "CALL", "2NNN", "Call subroutine at NNN" };
            break;

        case 0x3000:
            // 3XNN - Skip next instruction if VX == NN
            info = { "SKPCEQ", "3XNN", "Skip next instruction if VX == NN" };
            break;

        case 0x4000:
            // 4XNN - Skip next instruction if VX != NN
            info = { "SKPCNEQ", "4XNN", "Skip next instruction if VX != NN" };
            break;

        case 0x5000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    // 5XY0 - Skip next instr. if VX == VY
                    info = { "SKIPEQ", "5XY0", "Skip next instruction if VX == VY" };
                    break;
                default:
                    break;
            }
            break;

        case 0x6000:
            // 6XNN - Set VX to NN
            info = { "SETC", "6XNN", "Set VX to NN" };
            break;

        case 0x7000:
            // 7XNN - Add NN to VX (no change to carry flag)
            info = { "ADDCNF", "7XNN", "Add NN to VX (no change to carry flag)" };
            break;

        case 0x8000:
            switch (
                // Thanks based C++17 init-statements
                Byte X = (opcode & 0x0F00) >> 8,
                Y = (opcode & 0x00F0) >> 4;
                opcode & 0x000F
            ) {
                case 0x0000:
                    // 8XY0 - Set VX to the value of VY
                    info = { "SET", "8XY0", "Set VX to the value of VY" };
                    break;
                case 0x0001:
                    // 8XY1 - Set VX to VX | VY
                    info = { "SETOR", "8XY1", "Set VX to VX | VY" };
                    break;
                case 0x0002:
                    // 8XY2 - Set VX to VX & VY
                    info = { "SETAND", "8XY2", "Set VX to VX & VY" };
                    break;
                case 0x0003:
                    // 8XY1 - Set VX to VX ^ VY
                    info = { "SETXOR", "8XY3", "Set VX to VX ^ VY" };
                    break;
                case 0x0004:
                    // 8XY4 - Set VX to VX + VY (with carry)
                    info = { "ADD", "8XY4", "Set VX to VX + VY (with carry)" };
                    break;
                case 0x0005:
                    // 8XY5 - Set VX to VX - VY (with borrow)
                    // VF set to 0 if there's a borrow, else 1
                    info = { "SUB", "8XY5", "Set VX to VX - VY (with borrow)" };
                    break;
                case 0x0006:
                    // 8XY6 - Store the least significant bit
                    // of VX in VF and then shift VX to the
                    // right by 1 bit
                    info = { "RSHFT", "8XY6", "Shift VX right by 1 bit (set carry)" };
                    break;
                case 0x0007:
                    // 8XY7 - Set VX to VY - VX (with borrow)
                    // VF set to 0 if there's a borrow, else 1
                    info = { "SUBI", "8XY7", "Set VX to VY - VX (with borrow)" };
                    break;
                case 0x000E:
                    // 8XYE - Store the most significant bit
                    // of VX in VF and then shift VX to the
                    // left by 1 bit
                    info = { "LSHFT", "8XYE", "Shift VX left by 1 bit (set carry)" };
                    break;
                default:
                    break;
            }

        case 0x9000:
            switch (opcode & 0x000F) {
                case 0x0000:
                    // 9XY0 - Skip next instr. if VX != VY
                    info = { "SKPNEQ", "9XY0", "Skip next instruction if VX != VY" };
                    break;
                default:
                    break;
            }
            break;

        case 0xA000:
            // ANNN - Set I ot the address NNN
            info = { "SETI", "ANNN", "Set I ot the address NNN" };
            break;

        case 0xB000:
            // BNNN - Jump to address NNN plus V0
            info = { "JUMPAT", "BNNN", "Jump to address NNN plus V0" };
            break;

        case 0xC000:
            // CXNN - Set VX to rand() & NN
            info = { "RAND", "CXNN", "Set VX to rand() & NN" };
            break;

        case 0xD000:
            // DXYN - Draw a sprite at (VX, VY)
            // 8 pixels wide and N pixels high.
            // Set the carry flag if collision
            // occured between any pixels.
            info = { "DRAW", "DXYN", "Draw a sprite at (VX, VY) and set collision" };
            break;

        case 0xE000:
            switch (
                Byte X = (opcode & 0x0F00) >> 8;
                opcode & 0x00FF
            ) {
                case 0x009E:
                    // EX9E - Skip next instr.
                    // if key in VX is pressed
                    info = { "SKPKEY", "EX9E", "Skip next instr. if key in VX is pressed" };
                    break;
                case 0x00A1:
                    // EXA1 - Skip next instr.
                    // if key in VX in not pressed
                    info = { "SKPNKEY", "EXA1", "Skip next instr. if key in VX is not pressed" };
                    break;
                default:
                    break;
            }
            break;

        case 0xF000:
            switch (
                Byte X = (opcode & 0x0F00) >> 8;
                opcode & 0x00FF
            ) {
                case 0x0007:
                    // FX07 - Set VX to the value
                    // of the delay timer
                    info = { "GETDT", "FX07", "Set VX to the value of the delay timer" };
                    break;
                case 0x000A:
                    // FX0A - Await the key press,
                    // then store the key in VX
                    // (blocking)
                    info = { "WAITKEY", "FX0A", "Await the key then store in VX" };
                    // TODO: ehh, what???
                    break;
                case 0x0015:
                    // FX15 - Set the delay timer to VX
                    info = { "SETDT", "FX15", "Set the delay timer to VX" };
                    break;
                case 0x0018:
                    // FX18 - Set the sound timer to VX
                    info = { "SETST", "FX18", "Set the sound timer to VX" };
                    break;
                case 0x001E:
                    // FX1E - Add VX to I (no carry)
                    info = { "IADD", "FX1E", "Add VX to I (no carry)" };
                    break;
                case 0x0029:
                    // FX29 - Set I to the location
                    // of the sprite for the char in VX
                    info = { "IFONT", "FX29", "Set I to the location of the char in VX" };
                    break;
                case 0x0033:
                    // FX33 - Store the binary-coded decimal
                    // representation of VX with
                    info = { "BCD", "FX33", "Store the binary-coded decimal representation of VX" };
                    break;
                case 0x0055:
                    // FX55 - Stores from V0 to VX (including)
                    // in memory starting at address I
                    info = { "STORE", "FX55", "Store from V0 to VX (incl.) at address I" };
                    break;
                case 0x0065:
                    // FX65 - Fills from V0 to VX (including)
                    // with values from memory at address I
                    info = { "FILL", "FX65", "Fill from V0 to VX (incl.) from address I" };
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    fmt::print(
        "[{:#06X}] PC={:04X} I={:04X} V[{:02X}]    {:10} {:8}  {} \n",
        opcode,
        c8.get_pc(),
        c8.get_index(),
        fmt::join(c8.get_registers(), ","),
        info.name, info.pattern,
        info.desc
    );

}
