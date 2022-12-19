#pragma once
#include "Chip8.hpp"
#include <SFML/Config.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>


class Canvas {
private:
    sf::RenderWindow window_;

    using tex_buffer_t = std::array<
        sf::Uint8,
        Chip8Base::fb_width * Chip8Base::fb_height * 4
    >;
    // Buffer for transforming 8bit to 32bit frame
    tex_buffer_t tex_buffer_;

    sf::Texture tex_;
    sf::Sprite sprite_;

public:
    Canvas() :
        window_{
            sf::VideoMode{
                800, 600
            },
            "Chip8",
            sf::Style::Default,
            sf::ContextSettings{
                0, 0,
                8,
                3, 3,
                sf::ContextSettings::Default,
                false
            }
        }
    {
        window_.setKeyRepeatEnabled(false);
        tex_.create(Chip8Base::fb_width, Chip8Base::fb_height);
        sprite_.setTexture(tex_);
        sprite_.setScale(
            1.0f * sf::Vector2f{ 800.f / Chip8Base::fb_width, 600.f / Chip8Base::fb_height }
        );
    }


    sf::RenderWindow& window() noexcept {
        return window_;
    }
    const sf::RenderWindow& window() const noexcept {
        return window_;
    }


    void update(const Chip8::framebuffer_t& fb) {
        const sf::Color bg{ 0x00, 0x2B, 0x36 }; // Solarized Dark
        const sf::Color fg{ 0x83, 0x94, 0x96 }; //
        const sf::Color offset{ fg - bg };

        for (size_t i{ 0 }; i < fb.size(); ++i) {
            size_t j{ i * 4 };

            // RGBA
            tex_buffer_[j + 0] = bg.r + fb[i] * offset.r;
            tex_buffer_[j + 1] = bg.g + fb[i] * offset.g;
            tex_buffer_[j + 2] = bg.b + fb[i] * offset.b;
            tex_buffer_[j + 3] = 0xFF;
        }
        tex_.update(tex_buffer_.data());
    }

    void redraw() {
        window_.clear(sf::Color{ 0u, 0u, 0u });
        window_.draw(sprite_);
        window_.display();
    }



    void process_events(Chip8& chip8) {

        sf::Event event;
        while (window_.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window_.close();
                    break;
                case sf::Event::Resized:
                    // sprite_.setScale()
                    break;
                case sf::Event::KeyPressed:
                    process_key_pressed(chip8, event);
                    break;
                case sf::Event::KeyReleased:
                    process_key_released(chip8, event);
                    break;
                default:
                    break;
            }
        }

    }

private:
    void process_key_pressed(Chip8& chip8, const sf::Event& event) {

        using Key = sf::Keyboard::Key;

        switch (event.key.code) {
            case Key::Escape:
                window_.close();
                break;

            case Key::Num1: chip8.key_press(0x01); break;
            case Key::Num2: chip8.key_press(0x02); break;
            case Key::Num3: chip8.key_press(0x03); break;
            case Key::Num4: chip8.key_press(0x0C); break;
            case Key::Q:    chip8.key_press(0x04); break;
            case Key::W:    chip8.key_press(0x05); break;
            case Key::E:    chip8.key_press(0x06); break;
            case Key::R:    chip8.key_press(0x0D); break;
            case Key::A:    chip8.key_press(0x07); break;
            case Key::S:    chip8.key_press(0x08); break;
            case Key::D:    chip8.key_press(0x09); break;
            case Key::F:    chip8.key_press(0x0E); break;
            case Key::Z:    chip8.key_press(0x0A); break;
            case Key::X:    chip8.key_press(0x00); break;
            case Key::C:    chip8.key_press(0x0B); break;
            case Key::V:    chip8.key_press(0x0F); break;

            default: break;
        }
    }

    void process_key_released(Chip8& chip8, const sf::Event& event) {

        using Key = sf::Keyboard::Key;

        switch (event.key.code) {
            case Key::Num1: chip8.key_release(0x01); break;
            case Key::Num2: chip8.key_release(0x02); break;
            case Key::Num3: chip8.key_release(0x03); break;
            case Key::Num4: chip8.key_release(0x0C); break;
            case Key::Q:    chip8.key_release(0x04); break;
            case Key::W:    chip8.key_release(0x05); break;
            case Key::E:    chip8.key_release(0x06); break;
            case Key::R:    chip8.key_release(0x0D); break;
            case Key::A:    chip8.key_release(0x07); break;
            case Key::S:    chip8.key_release(0x08); break;
            case Key::D:    chip8.key_release(0x09); break;
            case Key::F:    chip8.key_release(0x0E); break;
            case Key::Z:    chip8.key_release(0x0A); break;
            case Key::X:    chip8.key_release(0x00); break;
            case Key::C:    chip8.key_release(0x0B); break;
            case Key::V:    chip8.key_release(0x0F); break;

            default: break;
        }
    }

};

