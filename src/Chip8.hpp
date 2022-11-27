#pragma once
#include <cstdint>
#include <array>
#include <span>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <random>

using Byte = unsigned char;
using Short = std::uint16_t;

inline Byte rand_byte() {
    static std::mt19937 gen{};
    return std::uniform_int_distribution<Byte>{}(gen);
}


struct Chip8Base {
    // Total 4k of memory (0x000-0xFFF)
    // 0x000-0x1FF - Chip8 interpreter / Internal data
    // 0x200-0xFFF - Program RAM
    std::array<Byte, 4096u> memory{};
    std::span<Byte, (0x1000-0x200)> RAM{
        memory.begin() + 0x200, memory.end()
    };

    // 15 8-bit registers V1..VE and
    // a 'carry flag' register VF
    std::array<Byte, 16u> V;

    // Current opcode
    Short opcode{};

    // Index register
    Short I{};

    // Program Counter
    Short pc{ 0x200 };

    // Screen B/W
    std::array<Byte, 64u*32u> frame{};

    // Hardware timers
    Byte delay_timer{};
    Byte sound_timer{};

    // Stack and stack pointer
    class CallStack {
    private:
        std::array<Short, 16u> stack_{};
        Byte sp_{};

    public:
        void push(Short pc) noexcept {
            stack_[sp_] = pc;
            ++sp_;
        }
        Short pop() noexcept {
            --sp_;
            return stack_[sp_];
        }
    };

    CallStack stack{};

    // Hex Keypad
    std::array<Byte, 16u> key{};

};



class Chip8 : private Chip8Base {


public:
    Chip8() = default;

    void emulate_cycle() noexcept {

        // Note: Big-endian
        opcode = memory[pc] << 8 | memory[pc + 1];

        switch (opcode & 0xF000) {
            case 0x0000:
                switch (opcode) {
                    case 0x00E0:
                        // 00E0 - Clear the screen
                        std::fill(frame.begin(), frame.end(), 0);
                        break;
                    case 0x00EE:
                        // 00EE - Return from subroutine
                        pc = stack.pop();
                        // TODO?
                        break;
                    default:
                        assert(false && "Unknown opcode");
                }
                break;

            case 0x1000:
                // 1NNN - Jump to address NNN
                pc = opcode & 0x0FFF;
                break;

            case 0x2000:
                // 2NNN - Call subroutine at NNN
                stack.push(pc);
                pc = opcode & 0x0FFF;
                break;

            case 0x3000:
                // 3XNN - Skip next instruction if VX == NN
                if (
                    V[(opcode & 0x0F00) >> 8] ==
                    (opcode & 0x00FF)
                ) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;

            case 0x4000:
                // 4XNN - Skip next instruction if VX != NN
                if (
                    V[(opcode & 0x0F00) >> 8] !=
                    (opcode & 0x00FF)
                ) {
                    pc += 4;
                } else {
                    pc += 2;
                }
                break;

            case 0x5000:
                switch (opcode & 0x000F) {
                    case 0x0000:
                        // 5XY0 - Skip next instr. if VX == VY
                        if (
                            V[(opcode & 0x0F00) >> 8] ==
                            V[(opcode & 0x00F0) >> 4]
                        ) {
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                        break;
                    default:
                        assert(false && "Unknown opcode");
                }
                break;

            case 0x6000:
                // 6XNN - Set VX to NN
                V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
                pc += 2;
                break;

            case 0x7000:
                // 7XNN - Add NN to VX (no change to carry flag)
                V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
                pc += 2;
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
                        V[X] = V[Y];
                        pc += 2;
                        break;
                    case 0x0001:
                        // 8XY1 - Set VX to VX | VY
                        V[X] |= V[Y];
                        pc += 2;
                        break;
                    case 0x0002:
                        // 8XY2 - Set VX to VX & VY
                        V[X] &= V[Y];
                        pc += 2;
                        break;
                    case 0x0003:
                        // 8XY1 - Set VX to VX ^ VY
                        V[X] ^= V[Y];
                        pc += 2;
                        break;
                    case 0x0004:
                        // 8XY4 - Set VX to VX + VY (with carry)
                        V[0xF] = V[Y] > (0xFF - V[X]);
                        V[X] += V[Y];
                        pc += 2;
                        break;
                    case 0x0005:
                        // 8XY5 - Set VX to VX - VY (with borrow)
                        // VF set to 0 if there's a borrow, else 1
                        // FIXME: is this right???
                        // WHATS A BORROW??? AHHHHH
                        V[0xF] = V[X] < V[Y];
                        V[X] -= V[Y];
                        pc += 2;
                        break;
                    case 0x0006:
                        // 8XY6 - Store the least significant bit
                        // of VX in VF and then shift VX to the
                        // right by 1 bit
                        V[0xF] = V[X] & 0x01;
                        V[X] >>= 1;
                        pc += 2;
                        break;
                    case 0x0007:
                        // 8XY7 - Set VX to VY - VX (with borrow)
                        // VF set to 0 if there's a borrow, else 1
                        // WHATS A BORROW??? HELLOOOO???
                        V[0xF] = V[Y] < V[X];
                        V[X] = V[Y] - V[X];
                        pc += 2;
                        break;
                    case 0x000E:
                        // 8XYE - Store the most significant bit
                        // of VX in VF and then shift VX to the
                        // left by 1 bit
                        V[0xF] = V[X] & 0x80;
                        V[X] <<= 1;
                        pc += 2;
                        break;
                    default:
                        assert(false && "Unknown opcode");
                }

            case 0x9000:
                switch (opcode & 0x000F) {
                    case 0x0000:
                        // 9XY0 - Skip next instr. if VX != VY
                        if (
                            V[(opcode & 0x0F00) >> 8] !=
                            V[(opcode & 0x00F0) >> 4]
                        ) {
                            pc += 4;
                        } else {
                            pc += 2;
                        }
                        break;
                    default:
                        assert(false && "Unknown opcode");
                }
                break;

            case 0xA000:
                // ANNN - Set I ot the address NNN
                I = opcode & 0x0FFF;
                pc += 2;
                break;

            case 0xB000:
                // BNNN - Jump to address NNN plus V0
                pc = V[0x0] + (opcode & 0x0FFF);
                break;

            case 0xC000:
                // CXNN - Set VX to rand() & NN
                V[(opcode & 0x0F00) >> 8] =
                    rand_byte() & (opcode & 0x00FF);
                pc += 2;
                break;

            case 0xD000: {
                // DXYN - Draw a sprite at (VX, VY)
                // 8 pixels wide and N pixels high
                // TODO
            }
            // TODO rest of opcodes
            default:
                assert(false && "Unknown opcode");
        }

        // Update Timers
        if (delay_timer) { --delay_timer; }

        if (sound_timer) {
            // Make sound?
            --sound_timer;
        }
    }



    void load_program(std::span<Byte> program) {
        std::memcpy(RAM.data(), program.data(), program.size());
    }


};
