#include <SFML/Graphics.hpp>
#include <iostream>

int main() {
    const int SCR_WIDTH = 1200;
    const int SCR_HEIGHT = 900;
    sf::RenderWindow screen(sf::VideoMode({SCR_WIDTH, SCR_HEIGHT}), "Mandelbrot Shader");
    screen.setFramerateLimit(60);

    // Load the shader from the file
    sf::Shader shader;
    std::filesystem::path shaderPath = "/Users/ansstsvv/complex/build/bin/mandelbrot.frag";

    // Now, pass the path object to the function
    if (!shader.loadFromFile(shaderPath, sf::Shader::Type::Fragment)) {
        std::cerr << "Error: Failed to load " << shaderPath << std::endl;
        std::cerr << "Please ensure the file is in the correct directory (next to the executable)." << std::endl;
        return -1;
    }
   

    // A simple plane to draw the shader on
    sf::RectangleShape screenPlane({(float)SCR_WIDTH, (float)SCR_HEIGHT});

    // Shader parameters
    sf::Vector2f center(-0.7f, 0.0f);
    float zoom = 3.0f;
    int max_iterations = 50;

    while (screen.isOpen()) {
        // --- Event Handling ---
        while (const auto event = screen.pollEvent()) {
            if (event->is<sf::Event::Closed>()) screen.close();
            // Simple zoom and pan controls
            if (event->is<sf::Event::KeyPressed>()) {
                switch (event->getIf<sf::Event::KeyPressed>()->code) {
                    case sf::Keyboard::Key::Escape: screen.close(); break;
                    case sf::Keyboard::Key::S: center.y -= 0.1f * zoom; break;
                    case sf::Keyboard::Key::W: center.y += 0.1f * zoom; break;
                    case sf::Keyboard::Key::A: center.x -= 0.1f * zoom; break;
                    case sf::Keyboard::Key::D: center.x += 0.1f * zoom; break;
                    case sf::Keyboard::Key::J: zoom *= 0.95f; max_iterations += 2; break; // Zoom in
                    case sf::Keyboard::Key::K: zoom *= 1.05f; max_iterations -= 2; break; // Zoom out
                    default: break;
                }
                if (max_iterations < 50) max_iterations = 50;
            }
        }

        // --- Update & Draw ---
        screen.clear();
        
        // Pass the updated variables to the shader every frame
        shader.setUniform("u_resolution", sf::Vector2f(SCR_WIDTH, SCR_HEIGHT));
        shader.setUniform("u_center", center);
        shader.setUniform("u_zoom", zoom);
        shader.setUniform("u_max_iterations", max_iterations);

        // Draw the plane, applying the shader to it
        screen.draw(screenPlane, &shader);
        
        screen.display();
    }
    return 0;
}