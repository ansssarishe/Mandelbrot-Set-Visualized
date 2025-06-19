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

const int WIDTH = 1200;
const int HEIGHT = 900;
double aspectRatio = static_cast<double>(WIDTH) / HEIGHT;
double scaleX = 3.0;
double scaleY = scaleX / aspectRatio;
double minX = -2.0;
double maxX = minX + scaleX;
double minY = -scaleY / 2;
double maxY = scaleY / 2;

void calculation(int max_iterations, sf::Image& image){
    for(unsigned int py = 0; py < HEIGHT; py++){
        for(unsigned int px = 0; px < WIDTH; px++){
            double x = minX + px * (maxX - minX) / WIDTH;
            double y = minY + py * (maxY - minY) / HEIGHT;

            Complex c(x, y);
            Complex z(0.0, 0.0);

            int iter = 0;
            while(iter < max_iterations && z.magnitudeSquared() <= 4.0){
                z = z * z + c;
                iter++;
            }

            unsigned int num = static_cast<unsigned int>(255.0 * iter / max_iterations);
            sf::Color color(num, num, num);

            image.setPixel({px, py}, color);
        }
    }

}


int main() {
    int max_iterations = 120;

    sf::RenderWindow screen(sf::VideoMode({WIDTH, HEIGHT}), "Mandelbrot");
    screen.setFramerateLimit(60);

    sf::Image image({WIDTH, HEIGHT}, sf::Color::Black);
    calculation(max_iterations, image);

    sf::Texture texture;
    texture.loadFromImage(image);
    sf::Sprite sprite(texture);

    while (screen.isOpen()) {
        while (const auto event = screen.pollEvent()) {
            if (event->is<sf::Event::Closed>() || (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::Escape)) {
                screen.close();
            }
        }
        screen.clear();
        screen.draw(sprite);
        screen.display();
    }

    return 0;
}