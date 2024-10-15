#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstdlib>


#define HORIZONTAL_CELLS 500
#define VERTICAL_CELLS 500
#define WINDOW_HEIGHT 1000
#define WINDOW_WIDTH 1000
#define MAX_CELL_HEIGHT 20
#define MEAN_CELL_STARTING_HEIGHT 3
#define SALTATION_HOP_DISTANCE 10

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

int neg_mod(int a, int b) {
    return ((a % b) + b) % b;
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
                    this->set_height(i, j, rand() % (MAX_CELL_HEIGHT-5));
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

        int set_height(int x, int y, int height) {
            if (height > MAX_CELL_HEIGHT || height < 0) {
                //std::cout << "Error: Tried to assign cell at (" << x << ", " << y << ")" << " invalid height of " << height << " (max " << MAX_CELL_HEIGHT << ")" << std::endl;
                return -1;
            }
            heights[x][y] = height;
            set_cell_color(x, y, getJetColor(height, MAX_CELL_HEIGHT));
            return 0;
        }

        /*
            Check if theres a 2-value delta between cell at x,y and its 8 Moore neighbors
            TODO should this be recursive?
        */
        bool avalanche(int x, int y) {
            int cell_height = heights[x][y];
            if (cell_height > 0) {
                std::vector<int> neighbors{0, 1, 2, 3, 4, 5, 6, 7};
                auto rng = std::default_random_engine {};
                std::shuffle(neighbors.begin(), neighbors.end(), rng);
                int neighbor_height;

                for (auto n : neighbors) {
                    int n_y = neg_mod(x + neighbor_values[n][0], HORIZONTAL_CELLS);
                    int n_x = neg_mod(y + neighbor_values[n][1], VERTICAL_CELLS);
                    //std::cout << "For cell " << x << ", " << y << std::endl;
                    neighbor_height = heights[n_x][n_y];
                    if (cell_height - neighbor_height > 2) {
                        if (set_height(n_x, n_y, heights[n_x][n_y]+1) == 0) {
                            set_height(x, y, cell_height-1);
                            avalanche(n_x, n_y);
                            return true;
                        }
                    }
                }
            }
            return false;
        }

        bool is_in_shadow(int x, int y) {
            int cell_height = heights[x][y];
            for (int h = 1; h < MAX_CELL_HEIGHT - cell_height; h++) {
                int height_potential_shadow = heights[neg_mod(x-h, HORIZONTAL_CELLS)][y];
                if (atan((height_potential_shadow - cell_height)/h) > 15) {
                    return true;
                }
            }
            return false;
        }

        void transfer_sand(int src_x, int src_y, int dest_x, int dest_y) {
            set_height(src_x, src_y, heights[src_x][src_y]-1);
            set_height(dest_x, dest_y, heights[dest_x][dest_y] + 1);
            avalanche(dest_x, dest_y);
        }

        void blow_cell(int x, int y) {
            int cell_height = heights[x][y];
            if (cell_height == 0 && is_in_shadow(x,y)) {
                return;
            }

            int dest_x = x;
            while (true) {
                dest_x = (dest_x + SALTATION_HOP_DISTANCE) % HORIZONTAL_CELLS;
                // 100% of the time, deposit (move sand bit from this cell to the dest cell)
                if (is_in_shadow(dest_x, y)) {
                    transfer_sand(x, y, dest_x, y);
                    return;
                }
                // Too tall
                if (heights[dest_x][y] > MAX_CELL_HEIGHT) {
                    continue;
                }
                // If cell is empty, 40% of the time, deposit
                if (heights[dest_x][y] == 0 && ((double) rand() / RAND_MAX) > 0.4) {
                    transfer_sand(x, y, dest_x, y);
                    return; 
                }  
                // 60% of the time, deposit
                else if (((double) rand() / RAND_MAX) > 0.6) {
                    transfer_sand(x, y, dest_x, y);
                    return;
                }                   
            }

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
        int neighbor_values[8][2] = {
            {0, -1}, {0, 1},  {1, 1}, 
            {-1, 0},          {1, 0},
            {1, -1}, {0, -1}, {-1, -1}
        };
        sf::VertexArray m_vertices;

};

int main() {
    auto window = sf::RenderWindow{ { WINDOW_HEIGHT, WINDOW_WIDTH }, "CMake SFML Project" };
    window.setFramerateLimit(144);

    SandCells sc;
    sc.randomize_cells();

    int wind_speed = 20000;

    bool did_avalanche = true;
    while (did_avalanche) {
        did_avalanche = false;
        for (int i = 0; i < HORIZONTAL_CELLS; i++) {
            for (int j = 0; j < VERTICAL_CELLS; j++) {
                did_avalanche |= sc.avalanche(i, j);
            }
        }
    }
    
    // Main loop
    while (window.isOpen()) {
        // Check for window closed
        for (auto event = sf::Event{}; window.pollEvent(event);) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        

        for (int i = 0; i < wind_speed; i++) {
            sc.blow_cell(rand() % VERTICAL_CELLS, rand() % HORIZONTAL_CELLS);
        }


        


        window.clear();
        window.draw(sc);
        window.display();
    }
}