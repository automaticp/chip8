#include "Canvas.hpp"
#include "Chip8.hpp"
#include "Disassembler.hpp"
#include <fmt/format.h>
#include <chrono>
#include <ios>
#include <iterator>
#include <thread>
#include <cassert>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <optional>
#include <vector>
#include <string>
#include <string_view>

using frame = std::chrono::duration<double, std::ratio<1, 60>>;

constexpr size_t cycles_per_frame{ 10 };


std::optional<std::vector<Byte>> read_binary(const std::string& file) {

    std::ifstream fs{ file, std::ios_base::binary };

    if (!fs.fail()) {
        try {
            return std::vector<Byte>{
                std::istreambuf_iterator<char>(fs),
                std::istreambuf_iterator<char>(),
            };
        } catch (std::ios_base::failure&) {
            return {};
        }
    }
    return {};
}


void print_fb(const Chip8::framebuffer_t& fb, Short opcode) {

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


void print_keypad(const std::array<Byte, 16u>& keypad) {
    thread_local std::array<char, 16u> buffer{};
    assert(keypad.size() == buffer.size());
    fmt::print("0123456789ABCDEF\n");
    for (size_t i{ 0 }; i < buffer.size(); ++i) {
        buffer[i] = keypad[i] ? 'X' : '.';
    }
    std::string_view sv{ buffer.begin(), buffer.end() };
    fmt::print("{}\n", sv);
}


int main(int argc, const char* argv[]) {

    // Argument parser would be nice, maybe...
    if (argc < 2) {
        std::cout << "Usage:\n    chip8 [file]\n";
        return 0;
    }

    std::string file{ argv[1] };
    auto program = read_binary(file);
    if (!program.has_value()) {
        std::cerr << "Unable to open file: " << file << '\n';
        return 1;
    }


    Canvas canvas{};
    auto& window = canvas.window();

    Chip8 chip8{};
    chip8.load_program(program.value());

    auto frame_clock =
        std::chrono::time_point_cast<frame>(
            std::chrono::steady_clock::now()
        );

    while (window.isOpen()) {
        frame_clock =
            std::chrono::time_point_cast<frame>(std::chrono::steady_clock::now())
            + frame{ 1 };


        for (unsigned cycle{ 0 };
            cycle < cycles_per_frame;
            ++cycle)
        {
            canvas.process_events(chip8);
            chip8.emulate_cycle();
            pretty_print_state(chip8);
            // print_keypad(chip8.get_keys());
        }


        if (chip8.should_draw()) {
            canvas.update(chip8.framebuffer());
            canvas.redraw();
            chip8.reset_draw_flag();
        }

        std::this_thread::sleep_until(frame_clock);

    }

}
