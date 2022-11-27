#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Graphics/Transform.hpp>
#include <SFML/Window/WindowStyle.hpp>
#include <cmath>


int main() {

    sf::ContextSettings settings{};
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.antialiasingLevel = 8;

    sf::RenderWindow window{
        sf::VideoMode{800, 600}, "Hello, SFML!", sf::Style::Default, settings
    };
    window.setVerticalSyncEnabled(true);

    sf::CircleShape shape{ 100.f };
    shape.setFillColor(sf::Color::Green);


    sf::Clock clock{};

    constexpr float speed{ 5.0f };

    while (window.isOpen()) {

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            } else if (event.type == sf::Event::KeyPressed) {
                switch (event.key.code) {
                    case sf::Keyboard::Key::W:
                        shape.move(0.f, -speed); break;
                    case sf::Keyboard::Key::S:
                        shape.move(0.f, speed); break;
                    case sf::Keyboard::Key::A:
                        shape.move(-speed, 0.f); break;
                    case sf::Keyboard::Key::D:
                        shape.move(speed, 0.f); break;
                    default: break;
                }
            }
        }

        window.clear();
        window.draw(shape);
        window.display();

    }

}
