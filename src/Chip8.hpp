#pragma once
#include <cstdint>
#include <array>
#include <span>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <string>
#include <fmt/format.h>

using Byte = unsigned char;
using Short = std::uint16_t;

inline Byte rand_byte() {
    static std::mt19937 gen{};
    return std::uniform_int_distribution<Byte>{}(gen);
}

// LE <-> BE conversion
inline Short byte_swap(Short val) noexcept {
    return ((val & 0x00FF) << 8) |
        ((val & 0xFF00) >> 8);
}


struct Chip8Base {
    // Total 4k of memory (0x000-0xFFF)
    // 0x000-0x1FF - Chip8 interpreter / Internal data
    // 0x200-0xFFF - Program RAM
    std::array<Byte, 4096u> memory{};
    std::span<Byte, 80u> fonts{
        memory.data(), 80ull
    };
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
    // Width: 64px, Height: 32px
    static constexpr size_t fb_width{ 64u };
    static constexpr size_t fb_height{ 32u };
    using framebuffer_t = std::array<Byte, fb_width * fb_height>;
    framebuffer_t frame{};

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
private:
    bool draw_flag{ false };
    static const std::array<Byte, 80> fontset;

public:
    Chip8() noexcept {
        init_fontset();
    }

    void emulate_cycle() noexcept {

        // Note: Big-endian
        opcode = memory[pc] << 8 | memory[pc + 1];

        decode_opcode();

        // Update Timers
        if (delay_timer) { --delay_timer; }

        if (sound_timer) {
            // Make sound?
            --sound_timer;
        }
    }

    void load_program(std::span<Byte> program) noexcept {
        std::memcpy(RAM.data(), program.data(), program.size());
    }

    using Chip8Base::framebuffer_t;
    const framebuffer_t& framebuffer() const noexcept {
        return frame;
    }

    bool should_draw() noexcept { return draw_flag; }
    void reset_draw_flag() noexcept { draw_flag = false; }

    Short get_opcode() const noexcept { return opcode; }

    void key_press(Byte id) noexcept { key[id] = 1; }
    void key_release(Byte id) noexcept { key[id] = 0; }

private:
    void decode_opcode() noexcept;

    void unknown_opcode(Short op) {
        throw std::runtime_error{ fmt::format("Unknown opcode: {:#06x}", opcode) };
    }
    void init_fontset() noexcept {
        std::memcpy(
            memory.data(), fontset.data(), fontset.size()
        );
    }

};
