#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <thread>

#include <SFML/Graphics.hpp>

// Complex struct is unchanged
struct Complex {
    double real;
    double imag;
    Complex() : real(0.0), imag(0.0) {}
    Complex(double r, double i) : real(r), imag(i) {}
    Complex operator*(const Complex& other) const { return Complex(real * other.real - imag * other.imag, real * other.imag + imag * other.real); }
    Complex operator+(const Complex& other) const { return Complex(real + other.real, imag + other.imag); }
    double magnitudeSquared() const { return real * real + imag * imag; }
};

// The mandelbrot_worker function is no longer needed, it will be replaced by a lambda.

// --- The Dispatcher Function ---
// Manages the threads.
void generateMandelbrot(sf::Image& image, Complex center, double zoom, int max_iterations) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    if (num_threads == 0) num_threads = 2;

    std::cout << "Generating with " << num_threads << " threads..." << std::endl;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);
    const unsigned int HEIGHT = 900;
    const unsigned int WIDTH = 1440;
    unsigned int rows_per_thread = HEIGHT / num_threads;

    for (unsigned int i = 0; i < num_threads; ++i) {
        unsigned int startY = i * rows_per_thread;
        unsigned int endY = (i == num_threads - 1) ? HEIGHT : (i + 1) * rows_per_thread;

        // --- SOLUTION: Use a Lambda Function ---
        // We define the worker's logic right here.
        // [&image] captures the image by reference.
        // [=] captures all other needed variables (like center, zoom, etc.) by value.
        threads.emplace_back([&image, center, zoom, max_iterations, startY, endY, WIDTH, HEIGHT] {
            const double viewWidth = 3.5;
            const double viewHeight = 2.0;

            for (unsigned int y = startY; y < endY; y++) {
                for (unsigned int x = 0; x < WIDTH; x++) {
                    double real = center.real + (static_cast<double>(x) / WIDTH - 0.5) * (viewWidth / zoom);
                    double imag = center.imag + (static_cast<double>(y) / HEIGHT - 0.5) * (viewHeight / zoom);


                    Complex c(real, imag);
                    Complex z(0, 0);

                    int iterations = 0;
            while (iterations < max_iterations && z.magnitudeSquared() < 4.0) {
                z = z * z + c;
                iterations++;
            }

            if(iterations < max_iterations){
                double hue = 0.7 + fmod(iterations, 255) / 255.0 * 0.1; // Smooth color
                double saturation = 0.8;
                double value = 1.0;

                double c = value * saturation;
                double h_prime = fmod(hue * 6.0, 6.0);
                double x_color = c * (1.0 - fabs(fmod(h_prime, 2.0) - 1.0));
                double m = value - c;

                sf::Color color;
                if (h_prime >= 0 && h_prime < 1) color = sf::Color((c + m) * 255, (x_color + m) * 255, (m) * 255);
                else if (h_prime >= 1 && h_prime < 2) color = sf::Color((x_color + m) * 255, (c + m) * 255, (m) * 255);
                else if (h_prime >= 2 && h_prime < 3) color = sf::Color((m) * 255, (c + m) * 255, (x_color + m) * 255);
                else if (h_prime >= 3 && h_prime < 4) color = sf::Color((m) * 255, (x_color + m) * 255, (c + m) * 255);
                else if (h_prime >= 4 && h_prime < 5) color = sf::Color((x_color + m) * 255, (m) * 255, (c + m) * 255);
                else color = sf::Color((c + m) * 255, (m) * 255, (x_color + m) * 255);

                image.setPixel({x, y}, color);
            }
            else{
                image.setPixel({x, y}, sf::Color::Black);
            }
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
    std::cout << "Generation complete." << std::endl;
}


int main() {
    const sf::VideoMode desktopMode = sf::VideoMode::getDesktopMode();
    const unsigned int WIDTH = desktopMode.size.x;
    const unsigned int HEIGHT = desktopMode.size.y;
    int max_iterations = 120;

    sf::RenderWindow screen(desktopMode, "Multithreaded Mandelbrot Explorer", sf::Style::None, sf::State::Fullscreen);
    screen.setFramerateLimit(60);

    Complex center(-0.75, 0.0);
    double zoom = 1.0;
    bool viewChanged = true;

    sf::Image image({WIDTH, HEIGHT}, sf::Color::Black);
    sf::Texture texture;
    sf::Sprite sprite(texture);

    while (screen.isOpen()) {
        while (const auto event = screen.pollEvent()) {
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
                screen.close();
            }
            if (const auto* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
                if (scrolled->delta > 0) zoom *= 1.25; else zoom /= 1.25;
                viewChanged = true;
            }
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::K)) {
                zoom *= 1.25;
            }
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::J)) {
                zoom /= 1.25;
            }
        }

        double panSpeed = 0.1 / zoom;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) { center.imag -= panSpeed; viewChanged = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) { center.imag += panSpeed; viewChanged = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) { center.real -= panSpeed; viewChanged = true; }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) { center.real += panSpeed; viewChanged = true; }

        if (viewChanged) {
            max_iterations = static_cast<int>(120 * sqrt(zoom));
            
            generateMandelbrot(image, center, zoom, max_iterations);
            
            // --- SOLUTION: Check the return value of loadFromImage ---
            if (!texture.loadFromImage(image)) {
                std::cerr << "Error: Could not load texture from image." << std::endl;
            } else {
                sprite.setTexture(texture, true); // 'true' resets the texture rect — important for resizing.
            }

            sprite.setTexture(texture);
            viewChanged = false;
        }

        screen.clear();
        screen.draw(sprite);
        screen.display();
    }

    return 0;
}