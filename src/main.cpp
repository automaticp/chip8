#include "Canvas.hpp"
#include "Chip8.hpp"
#include "Debug.hpp"
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


static std::optional<std::vector<Byte>> read_binary(const std::string& file) {

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

    while (window.isOpen()) {
        auto next_frame =
            std::chrono::time_point_cast<frame>(std::chrono::steady_clock::now())
            + frame{ 1 };


        for (unsigned cycle{ 0 };
            cycle < cycles_per_frame;
            ++cycle)
        {
            canvas.process_events(chip8);
            chip8.emulate_cycle();
            debug::pretty_print_state(chip8);
            // debug::print_keypad(chip8.get_keys());
        }

        chip8.update_timers();

        if (chip8.should_draw()) {
            canvas.update(chip8.framebuffer());
            canvas.redraw();
            chip8.reset_draw_flag();
        }

        std::this_thread::sleep_until(next_frame);

    }

}
