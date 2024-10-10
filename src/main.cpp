#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>

#define HORIZONTAL_CELLS 500
#define VERTICAL_CELLS 500
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

class SandCells : public sf::Drawable, public sf::Transformable {
    public:
        SandCells() {
            int num_vertices = HORIZONTAL_CELLS*VERTICAL_CELLS*4;
            m_vertices = sf::VertexArray(sf::Quads, num_vertices);
            float cell_width = WINDOW_WIDTH/HORIZONTAL_CELLS;
            float cell_height = WINDOW_HEIGHT/VERTICAL_CELLS;

            // idx is vertices index, i,j represents sand cell at that coordinate
            int idx;
            for (int i = 0; i < HORIZONTAL_CELLS; i++) {
                for (int j = 0; j < VERTICAL_CELLS; j++) {
                    idx = (i + j*HORIZONTAL_CELLS)*4;
                    m_vertices[idx].position = sf::Vector2f(i*cell_width, j*cell_height);                   // Top left (base)
                    m_vertices[idx+1].position = sf::Vector2f((i+.9999)*cell_width, j*cell_height);         // Top right
                    m_vertices[idx+2].position = sf::Vector2f((i+.9999)*cell_width, (j+.9999)*cell_height); // Bottom right (opposite)
                    m_vertices[idx+3].position = sf::Vector2f((i)*cell_width, (j+.9999)*cell_height);       // Bottom left             
                }
            }
        }

        void randomize_cells() { 
            for (int i = 0; i < HORIZONTAL_CELLS; i++) {
                for (int j = 0; j < VERTICAL_CELLS; j++) {
                    this->set_height(i, j, rand() % MAX_CELL_HEIGHT);
                }
            }
        }

        /*
            Turn the given sand cell at x,y in "sand coordinates" to the color specified.
        */
        void set_cell_color(int x, int y, sf::Color color) {
            int idx = get_vertex_index(x, y);
            m_vertices[idx].color = color;
            m_vertices[idx+1].color = color;
            m_vertices[idx+2].color = color;
            m_vertices[idx+3].color = color;
        }

        int get_vertex_index(int x, int y) {
            return (x + y*HORIZONTAL_CELLS)*4;
        }

        void set_height(int x, int y, int height) {
            if (height > MAX_CELL_HEIGHT || height < 0) {
                std::cout << "Error: Tried to assign cell at (" << x << ", " << y << ")" << " invalid height of " << height << " (max " << MAX_CELL_HEIGHT << ")" << std::endl;
                return;
            }
            heights[x][y] = height;
            set_cell_color(x, y, getJetColor(height, MAX_CELL_HEIGHT));
        }

    private:
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const {
            // Apply transform
            states.transform *= getTransform();
            // Don't need tiles
            states.texture = NULL;
            // Draw
            target.draw(m_vertices, states);
        }
        int heights[HORIZONTAL_CELLS][VERTICAL_CELLS];
        sf::VertexArray m_vertices;

};

int main()
{
    auto window = sf::RenderWindow{ { WINDOW_HEIGHT, WINDOW_WIDTH }, "CMake SFML Project" };
    window.setFramerateLimit(144);

    std::default_random_engine generator;
    std::normal_distribution<int> distribution(MEAN_CELL_STARTING_HEIGHT, 2);

    SandCells sc;
    for (int i = 0; i < HORIZONTAL_CELLS; i++) {
        for (int j = 0; j < VERTICAL_CELLS; j++) {
            sc.set_cell_color(i, j, getJetColor(i, HORIZONTAL_CELLS));
        }
    }
    sc.randomize_cells();

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
        window.draw(sc);
        window.display();
    }
}