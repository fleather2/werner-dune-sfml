#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>

#define HORIZONTAL_CELLS 100
#define VERTICAL_CELLS 100
#define WINDOW_HEIGHT 1000
#define WINDOW_WIDTH 1000
#define MAX_CELL_HEIGHT 15
#define MEAN_CELL_STARTING_HEIGHT 5

sf::Color getJetColor(int intensity, int max){
    // Get a normalized value between 0 and 255
    uint8_t int_norm = intensity * 255 / max;
    if (int_norm < 51) {
        return sf::Color(0, 0, int_norm*5);
    } else if (51 <= int_norm && int_norm < 102) {
        return sf::Color(0, (int_norm-51)*5, 255);
    } else if (102 <= int_norm && int_norm < 153) {
        return sf::Color(0, 255, 255-((int_norm-102)*5));
    } else if (153 <= int_norm && int_norm < 204) {
        return sf::Color((int_norm-153)*5, 255, 0);
    } else if (204 <= int_norm && int_norm < 255) {
        return sf::Color(255, 255-((int_norm-204)*5), 0);
    }
    return sf::Color::White;
    std::cout << "Error: Invalid color value " <<  intensity << " for scale 0-" << max << std::endl;
}

struct Cell {
    sf::CircleShape cellShape; 
    int height;
};

int main()
{
    auto window = sf::RenderWindow{ { WINDOW_HEIGHT, WINDOW_WIDTH }, "CMake SFML Project" };
    window.setFramerateLimit(144);

    Cell cells[HORIZONTAL_CELLS][VERTICAL_CELLS];
    std::default_random_engine generator;
    std::normal_distribution<int> distribution(MEAN_CELL_STARTING_HEIGHT, 2);

    for (int i = 0; i < HORIZONTAL_CELLS; i++) {
        for (int j = 0; j < VERTICAL_CELLS; j++) {
            cells[i][j].height = distribution(generator);
            cells[i][j].cellShape = sf::CircleShape(3);
            cells[i][j].cellShape.setPosition((i/(float)HORIZONTAL_CELLS)*WINDOW_WIDTH, (j/(float)VERTICAL_CELLS)*WINDOW_HEIGHT);
        }
    }

    while (window.isOpen())
    {
        for (auto event = sf::Event{}; window.pollEvent(event);)
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        window.clear();
        for (int i = 0; i < HORIZONTAL_CELLS; i++) {
            for (int j = 0; j < VERTICAL_CELLS; j++) {
                cells[i][j].cellShape.setFillColor(getJetColor(i, HORIZONTAL_CELLS));
                //cells[i][j].cellShape.setFillColor(sf::Color::Red);
                window.draw(cells[i][j].cellShape);
            }
        }
        window.display();
    }
}
