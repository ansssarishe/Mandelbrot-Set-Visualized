#include <iostream>
#include <complex>
#include <cmath>
#include <vector>
#include <thread>

#include <SFML/Graphics.hpp>

// Complext structure to store complex numbers and manage operations
struct Complex {
    double real;
    double imag;
    Complex() : real(0.0), imag(0.0) {}
    Complex(double r, double i) : real(r), imag(i) {}
    Complex operator*(const Complex& other) const { return Complex(real * other.real - imag * other.imag, real * other.imag + imag * other.real); }
    Complex operator+(const Complex& other) const { return Complex(real + other.real, imag + other.imag); }
    double magnitudeSquared() const { return real * real + imag * imag; }
};

const int WIDTH = 4000;
const int HEIGHT = 3000;
double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
double scaleX = 3.0;
double scaleY = scaleX / aspectRatio;
double minX = -2.0;
double maxX = minX + scaleX;
double minY = -scaleY / 2;
double maxY = scaleY / 2;



const int SCR_WIDTH = 1200;
const int SCR_HEIGHT = 900;

unsigned int thread_count = std::thread::hardware_concurrency();

sf::Color getColor(int iter, int max_iterations) {
    if (iter == max_iterations) {
        return sf::Color::Black;
    }
    // Map iteration count to a hue value (0 to 360)
    double hue = fmod(sqrt(static_cast<double>(iter)) * 360.0, 360.0);
    // A simple way to convert HSV/HSL to RGB is needed here
    // For simplicity, here's a basic rainbow-like effect:
    uint8_t r = static_cast<uint8_t>(sin(0.05 * iter + 0) * 127 + 128);
    uint8_t g = static_cast<uint8_t>(sin(0.05 * iter + 2) * 127 + 128);
    uint8_t b = static_cast<uint8_t>(sin(0.05 * iter + 4) * 127 + 128);
    return sf::Color(r, g, b);
}

void calculate_region(int max_iterations, double minX, double maxX, double minY, double maxY,
                      int T_min, int T_max, sf::Image& image);

void delegation(int max_iterations, double minX, double maxX, double minY, double maxY, sf::Image& image){
    unsigned int rows_per_thread = HEIGHT / thread_count;
    std::vector<std::thread> threads;

    for(int thread = 0; thread < thread_count; thread++){
        int T_min = thread * rows_per_thread;
        int T_max = (thread + 1) * rows_per_thread;

        threads.emplace_back(calculate_region,
                             max_iterations, minX, maxX, minY, maxY,
                             T_min, T_max, std::ref(image));
    }

    for (auto& t : threads)
    t.join();

    threads.clear();
}

void calculate_region(int max_iterations, double minX, double maxX, double minY, double maxY, int T_min, int T_max, sf::Image& image){
    const double dx = (maxX - minX) / WIDTH;
    const double dy = (maxY - minY) / HEIGHT;

    for(unsigned int py = T_min; py < T_max; py++){
        for(unsigned int px = 0; px < WIDTH; px++){
            double x = minX + px * dx;
            double y = minY + py * dy;

            std::complex<double> c(x, y);
            std::complex<double> z(0.0, 0.0);

            int iter = 0;
            while(iter < max_iterations && std::norm(z) <= 4.0){
                z = z * z + c;
                iter++;
            }

            unsigned int num = static_cast<unsigned int>(255.0 * iter / max_iterations);
            sf::Color color = getColor(iter, max_iterations);

            image.setPixel({px, py}, color);
        }
    }
}

int main() {
    int max_iterations = 40;

    sf::RenderWindow screen(sf::VideoMode({SCR_WIDTH, SCR_HEIGHT}), "Mandelbrot");
    screen.setFramerateLimit(60);

    sf::Image image({WIDTH, HEIGHT}, sf::Color::Black);
    delegation(max_iterations, minX, maxX, minY, maxY, image);

    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    sf::View view({WIDTH / 2, HEIGHT / 2}, {SCR_WIDTH, SCR_HEIGHT});
    screen.setView(view);

    float zoom = 1;
    float zoom_scale = 1.25;
    

    while (screen.isOpen()) {
        while (const auto event = screen.pollEvent()) {
            if(event->is<sf::Event::Closed>()){
                screen.close();
            }

            if(event->is<sf::Event::KeyPressed>()){
                switch(event->getIf<sf::Event::KeyPressed>()->code){
                    case sf::Keyboard::Key::Escape: screen.close(); break;

                    case sf::Keyboard::Key::W: view.move({0.f, -100.f}); break;
                    case sf::Keyboard::Key::S: view.move({0.f, 100.f}); break;
                    case sf::Keyboard::Key::A: view.move({-100.f, 0.f}); break;
                    case sf::Keyboard::Key::D: view.move({100.f, 0.f}); break;

                    case sf::Keyboard::Key::J: zoom *= zoom_scale; view.zoom(zoom_scale); break;
                    case sf::Keyboard::Key::K: zoom /= zoom_scale; view.zoom(1.f/zoom_scale); break;
                }
            }
        }

        if (zoom >= 2.f || zoom <= 0.5f) {
            // Step 1: Get current view bounds
            sf::Vector2f center = view.getCenter();
            sf::Vector2f size = view.getSize();

            float left   = center.x - size.x / 2.f;
            float right  = center.x + size.x / 2.f;
            float top    = center.y - size.y / 2.f;
            float bottom = center.y + size.y / 2.f;

            // Step 2: Convert to complex plane
            double newMinX = minX + (left   / WIDTH) * (maxX - minX);
            double newMaxX = minX + (right  / WIDTH) * (maxX - minX);
            double newMinY = minY + (top    / HEIGHT) * (maxY - minY);
            double newMaxY = minY + (bottom / HEIGHT) * (maxY - minY);

            max_iterations = static_cast<int>(max_iterations * 1.5);
            if (max_iterations > 2000) max_iterations = 2000;
            // Step 3: Recalculate Mandelbrot
            delegation(max_iterations, newMinX, newMaxX, newMinY, newMaxY, image);
            texture.loadFromImage(image);
            sprite.setTexture(texture);

            // Step 4: Update global bounds & reset view
            minX = newMinX; maxX = newMaxX;
            minY = newMinY; maxY = newMaxY;

            view.setCenter({WIDTH / 2.f, HEIGHT / 2.f});
            view.setSize({SCR_WIDTH, SCR_HEIGHT});
            zoom = 1;
        }


        screen.setView(view);
        screen.clear();
        screen.draw(sprite);
        screen.display();
    }

    return 0;
}