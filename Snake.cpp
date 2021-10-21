#include <vector>
#include <iostream>
#include <conio.h>
#include <Windows.h>
#include <random>


// This snake game i made all by myself with
// absolutely no guides on how the snake game
// works. This is my first 'game' created.


/* Game structure is pretty straigtforward:
* Four classes: Chunk, Snake, Map, Game.
* =========================================
* CHUNK: Contains chunk_type, direction,
* (x, y) coordinates. Head chunk contains
* direction. Also contains character for
* output in console.
* =========================================
* SNAKE: can moves and eat. Cannot detect
* limits of the map. Does not have direction.
* Snake consists of chunks, that on its
* own has a direction.
* =========================================
* MAP: controls food generation and direct
* snake, also detect if its dead.
* =========================================
* GAME: Launch game and vise versa, another
* features with game.
*/

class Chunk
{
public:
    enum class Direction { Up = 0, Down, Left, Right, None };
    enum class Type { Head = 0, Body, Food, Border, Space };
    enum class Symbol : char {
        Head = '@', Body = '*', Food = '$',
        Border = '#', Space = ' ', Skeleton = '+'
    };
private:
    Symbol symbol;
    size_t x, y;

    Direction direction;
    Type chunk_type;
public:
    Chunk() = delete;   // Always specify position for each chunk of the snake/map and etc/
    Chunk(size_t x, size_t y, Direction direction, Type chunk_type, Symbol symbol)
    {
        this->chunk_type = chunk_type;
        this->direction = direction;
        this->symbol = symbol;
        this->x = x;
        this->y = y;
    }

    inline const Direction& get_direction() const { return direction; }
    inline const Symbol& get_symbol() const { return symbol; }
    inline const Type& get_type() const { return chunk_type; }
    inline const size_t& get_x() const { return x; }
    inline const size_t& get_y() const { return y; }

    inline void set_direction(Direction direction) { this->direction = direction; }
    inline void set_type(Type chunk_type) { this->chunk_type = chunk_type; }
    inline void set_symbol(Symbol symbol) { this->symbol = symbol; }
    inline void set_x(const size_t x) { this->x = x; }
    inline void set_y(const size_t y) { this->y = y; }
};


class Snake
{
private:
    std::vector<Chunk> snake_body;
    // ch stands for chunk
    using ch_dir = Chunk::Direction;
    using ch_type = Chunk::Type;
    using ch_sym = Chunk::Symbol;
public:
    inline const std::vector<Chunk>& get_body() const { return this->snake_body; }
    inline const Chunk& get_head() const { return this->snake_body.front(); }
    inline const Chunk& get_tail() const { return this->snake_body.back(); }

    inline void set_head_direction(ch_dir direction) { this->snake_body.front().set_direction(direction); }
    inline void set_head_x(size_t x) { this->snake_body.front().set_x(x); }
    inline void set_head_y(size_t y) { this->snake_body.front().set_y(y); }

    Snake(size_t x, size_t y, ch_dir direction, ch_type chunk_type, ch_sym symbol)
    {
        Chunk head_chunk(x, y, direction, chunk_type, symbol);
        snake_body.push_back(head_chunk);
    }

    void move(ch_dir new_direction)
    {
        ch_dir direction_buffer = get_head().get_direction();

        for (auto& chunk : snake_body)
        {
            switch (chunk.get_direction()) {
            case ch_dir::Down:
            {
                size_t current_y = chunk.get_y();
                chunk.set_y(current_y + 1);
                break;
            }
            case ch_dir::Up:
            {
                size_t current_y = chunk.get_y();
                chunk.set_y(current_y - 1);
                break;
            }
            case ch_dir::Left:
            {
                size_t current_x = chunk.get_x();
                chunk.set_x(current_x - 1);
                break;
            }
            case ch_dir::Right:
            {
                size_t current_x = chunk.get_x();
                chunk.set_x(current_x + 1);
                break;
            }
            default:
                break;
            }
        }
        // When snake moves, chunks at the nodes change 
        for (size_t i = snake_body.size() - 1; i > 0; i--)
        {
            ch_dir prev_node_dir = snake_body[i].get_direction();
            ch_dir next_node_dir = snake_body[i - 1].get_direction();
            if (next_node_dir != prev_node_dir)
            {
                snake_body[i].set_direction(next_node_dir);
            }
        }
    }

    void eat()
    {
        Chunk tail_chunk = get_tail();
        ch_dir tail_direction = tail_chunk.get_direction(); // Direction of the new tail is gonna be 
                                                            // the same as previous one 
        size_t new_tail_x = tail_chunk.get_x();
        size_t new_tail_y = tail_chunk.get_y();

        switch (tail_direction) {
        case ch_dir::Down:
        {
            new_tail_y--;
            break;
        }
        case ch_dir::Up:
        {
            new_tail_y++;
            break;
        }
        case ch_dir::Left:
        {
            new_tail_x++;
            break;
        }
        case ch_dir::Right:
        {
            new_tail_x--;
            break;
        }
        default:
            break;
        };
        snake_body.emplace_back(new_tail_x, new_tail_y, tail_direction, ch_type::Body, ch_sym::Body);
    }
};


class Map
{
private:
    // After Map constructor it will never be resized that's why i use 2dVector
    // It's would be better to use 2dArray instead of 2dVector because of perfomance
    // Or even 1dArray

    using chunk_map = std::vector<std::vector<Chunk>>;
    using map_row = std::vector<Chunk>;

    using ch_dir = Chunk::Direction;
    using ch_type = Chunk::Type;
    using ch_sym = Chunk::Symbol;

    bool snake_dead;    // check of snake is not out of border

    size_t width, height;
    chunk_map game_map;

    size_t food_x, food_y;

    Snake snake{ width / 2, height / 2, ch_dir::None, ch_type::Head, ch_sym::Head };
public:
    // +2 cause there's a border on all of the sides
    Map() : width(31 + 2), height(21 + 2)    // map 20x20 chunks
    {
        snake_dead = false;
        map_initialize();
        food_generate();
    }

    void map_initialize()
    {
        size_t last_row_y = height - 1;
        size_t first_row_y = 0;

        game_map.reserve(height);   // reserve space for rows
        map_row top_border;
        top_border.reserve(width);

        for (size_t x = 0; x < width; x++)
        {
            // push border chunk
            top_border.emplace_back(x, first_row_y, ch_dir::None, ch_type::Border, ch_sym::Border);
        }
        game_map.emplace_back(top_border);

        // 1 cause of the top and bottom border cause we start from the second line 
        // and stops before the last line => height - 1
        for (size_t y = first_row_y + 1; y < last_row_y; y++)
        {
            size_t first_col_x = 0;
            size_t last_col_x = width - 1;

            map_row map_side_borders;
            map_side_borders.reserve(width);

            map_side_borders.emplace_back(first_col_x, y, ch_dir::None, ch_type::Border, ch_sym::Border);

            for (size_t x = first_col_x + 1; x < last_col_x; x++)   // spaces between borders
            {
                map_side_borders.emplace_back(x, y, ch_dir::None, ch_type::Space, ch_sym::Space);
            }
            map_side_borders.emplace_back(last_col_x, y, ch_dir::None, ch_type::Border, ch_sym::Border);

            game_map.emplace_back(map_side_borders);
        }

        map_row bottom_border;
        bottom_border.reserve(width);

        for (size_t x = 0; x < width; x++)
        {
            // push border chunk
            bottom_border.emplace_back(x, last_row_y, ch_dir::None, ch_type::Border, ch_sym::Border);
        }
        game_map.emplace_back(bottom_border);
    }

    void map_print()
    {
        Chunk head = snake.get_head();
        map_row snake_body = snake.get_body();

        size_t snake_x = head.get_x();
        size_t snake_y = head.get_y();
        size_t snake_size = snake_body.size();
        ch_dir snake_direction = head.get_direction();

        for (auto& chunk : snake.get_body())
        {
            size_t chunk_x = chunk.get_x();
            size_t chunk_y = chunk.get_y();
            game_map[chunk_y][chunk_x] = chunk;
        }

        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                Chunk map_chunk = game_map[y][x];
                putchar(static_cast<char>(map_chunk.get_symbol()));
            }
            putchar('\n');
        }

        for (auto& chunk : snake.get_body())
        {
            size_t chunk_x = chunk.get_x();
            size_t chunk_y = chunk.get_y();
            Chunk space_chunk = Chunk(chunk_x, chunk_y, ch_dir::None, ch_type::Space, ch_sym::Space);
            game_map[chunk_y][chunk_x] = std::move(space_chunk);
        }

        switch (snake_direction) {
        case ch_dir::Down:
            printf("\n       Snake direction: = Down        ");
            break;
        case ch_dir::Up:
            printf("\n       Snake direction: = Up          ");
            break;
        case ch_dir::Left:
            printf("\n       Snake direction: = Left        ");
            break;
        case ch_dir::Right:
            printf("\n       Snake direction: = Right       ");
            break;
        case ch_dir::None:
            printf("\n       Snake direction: = None        ");
            break;
        default:
            break;
        };

        printf("\n            Food eaten: = %zu      \n", snake_size - 1);
        printf("   Snake's head symbol: = %c        \n", static_cast<char>(head.get_symbol()));
        printf("      Map allowed size: width = %zu, height = %zu      \n", width - 2, height - 2);
        printf("      Food coordinates: X = %zu, Y = %zu      \n", food_x, food_y);
        printf(" Map chunk of the head: X = %zu, Y = %zu      \n", game_map[snake_y][snake_x].get_x(),
            game_map[snake_y][snake_x].get_y());
        printf("Snake head coordinates: X = %zu, Y = %zu      \n\n\n", head.get_x(), head.get_y());
    }

    void food_generate()
    {
        std::random_device rd;
        std::mt19937 mt(rd());

        std::uniform_int_distribution<size_t> dist_width(0, width - 1);
        std::uniform_int_distribution<size_t> dist_height(0, height - 1);

        size_t food_x = dist_width(mt);
        size_t food_y = dist_height(mt);

        Chunk food_chunk = Chunk(food_x, food_y, ch_dir::None, ch_type::Food, ch_sym::Food);
        map_row snake_body = snake.get_body();

        while (true)
        {
            if (game_map[food_y][food_x].get_type() == ch_type::Space)
            {
                game_map[food_y][food_x] = std::move(food_chunk);
                break;
            }
            food_x = dist_width(mt);
            food_y = dist_height(mt);
        }

        this->food_x = food_x;
        this->food_y = food_y;
    }
  
    void direct_snake(ch_dir new_direction)
    {
        auto& head = snake.get_head();
        ch_dir current_direction = head.get_direction();

        switch (new_direction) {
        case ch_dir::Down:
            if (current_direction != ch_dir::Up)
            {
                snake.set_head_direction(ch_dir::Down);
                snake.move(ch_dir::Down);
            }
            else
            {
                snake.move(current_direction);
            }
            break;
        case ch_dir::Up:
            if (current_direction != ch_dir::Down)
            {
                snake.set_head_direction(ch_dir::Up);
                snake.move(ch_dir::Up);
            }
            else
            {
                snake.move(current_direction);
            }
            break;
        case ch_dir::Left:
            if (current_direction != ch_dir::Right)
            {
                snake.set_head_direction(ch_dir::Left);
                snake.move(ch_dir::Left);
            }
            else
            {
                snake.move(current_direction);
            }
            break;
        case ch_dir::Right:
            if (current_direction != ch_dir::Left)
            {
                snake.set_head_direction(ch_dir::Right);
                snake.move(ch_dir::Right);
            }
            else
            {
                snake.move(current_direction);
            }
            break;
        default:
            break;
        };

        food_collision_detect();
        border_collision_detect();
        body_collision_detect();
    }

    void food_collision_detect()
    {
        auto& new_head = snake.get_head();
        size_t new_head_x = new_head.get_x();
        size_t new_head_y = new_head.get_y();

        if (new_head_x == food_x && new_head_y == food_y)
        {
            Chunk space_chunk = Chunk(food_x, food_y, ch_dir::None, ch_type::Space, ch_sym::Space);
            game_map[food_y][food_x] = std::move(space_chunk);
            food_generate();
            snake.eat();
        }
    }

    void body_collision_detect()
    {
        auto& new_head = snake.get_head();
        size_t new_head_x = new_head.get_x();
        size_t new_head_y = new_head.get_y();

        // Temporary solution
        size_t snake_length = snake.get_body().size();

        for (size_t i = 1; i < snake_length && !snake_dead; i++)
        {
            size_t chunk_x = snake.get_body()[i].get_x();
            size_t chunk_y = snake.get_body()[i].get_y();

            if (chunk_x == new_head_x && chunk_y == new_head_y)
            {
                snake_dead = true;
            }
            ;
        }
    }

    void border_collision_detect()
    {
        auto& new_head = snake.get_head();
        size_t new_head_x = new_head.get_x();
        size_t new_head_y = new_head.get_y();

        // the idea was to implement a dynamic map made up of fragments 
        // so that we could easily tell the program when we were stuck. 
        // But the problem is that I can't figure out how to move the snake without 
        // having to clear the map before the next frame. Cause initally
        // we place snake on the map, render the frame and get rid of the snake 
        // on the map. And then since map don's contains snake we are not able
        // to detect if head encounter the body. So below the temporary solution.

        // How algorithm was like in my head before implementing collision detection
        if (game_map[new_head_y][new_head_x].get_type() == ch_type::Border
            /*|| game_map[new_head_y][new_head_x].get_type() == ch_type::Body*/)
        {
            snake_dead = true;  // hit the border or body
        }
    }

    void end_game_map()
        {
            for (size_t y = 0; y < height; y++)
            {
                for (size_t x = 0; x < width; x++)
                {
                    game_map[y][x].set_symbol(ch_sym::Space);
                }
            }

            for (auto& chunk : snake.get_body())
            {
                size_t chunk_x = chunk.get_x();
                size_t chunk_y = chunk.get_y();
                game_map[chunk_y][chunk_x].set_symbol(ch_sym::Skeleton);
            }

            for (size_t y = 0; y < height; y++)
            {
                for (size_t x = 0; x < width; x++)
                {
                    Chunk map_chunk = game_map[y][x];
                    putchar(static_cast<char>(map_chunk.get_symbol()));
                }
                putchar('\n');
            }

            printf("\n\n\n\n\n\n\n\n\nGAME OVER!");
            printf("\nYOUR SCORE IS: %zu\n", snake.get_body().size());
        }

    inline const size_t& get_map_height() const { return height; };
    inline const chunk_map& get_map() const { return game_map; };
    inline const size_t& get_map_width() const { return width; };
    inline const Snake& get_snake() const { return snake; }
    inline const bool& is_snake_dead() const { return snake_dead; }
};


class Game
{
public:
    enum class State { On = 0, Off };
private:
    enum class Button
    {
        W_up = 119, S_down = 115, A_left = 97,
        D_right = 100, Space = 32, Esc = 27
    };

    State game_state;
    Map game_map;

    DWORD vertical_delay = 100;
    DWORD horizontal_delay = 75;
public:

    Game() : game_state(State::On)
    {
        hide_cursor();
    }

    void end()
    {
        render(0);  // number here does not matter cause that's the last frame
        game_state = State::Off;
        game_map.end_game_map();
    }

    void start()
    {
        wint_t last_key_pressed = static_cast<wint_t>(Button::Space);

        while (game_state == State::On)
        {
            // prevent the same key registration straight 
            if (_kbhit())
            {
                wint_t key_buffer = _getwch();

                if (key_buffer != last_key_pressed)
                {
                    last_key_pressed = key_buffer;
                }
            }

            if (game_map.is_snake_dead())
            {
                end();
                break;
            }

            switch (last_key_pressed)
            {
            case static_cast<wint_t>(Button::S_down):
                render(vertical_delay);
                game_map.map_print();
                game_map.direct_snake(Chunk::Direction::Down);
                break;
            case static_cast<wint_t>(Button::W_up):
                render(vertical_delay);
                game_map.map_print();
                game_map.direct_snake(Chunk::Direction::Up);
                break;
            case static_cast<wint_t>(Button::A_left):
                render(horizontal_delay);
                game_map.map_print();
                game_map.direct_snake(Chunk::Direction::Left);
                break;
            case static_cast<wint_t>(Button::D_right):
                render(horizontal_delay);
                game_map.map_print();
                game_map.direct_snake(Chunk::Direction::Right);
                break;
            case static_cast<wint_t>(Button::Space):
                render(0);
                game_map.map_print();
                game_map.direct_snake(Chunk::Direction::None);
                break;
            case static_cast<wint_t>(Button::Esc):
                printf("See you!\n\n");
                game_state = State::Off;
                break;
            default:
                break;
            }
        }
    };

    void render(DWORD time)
    {
        clear_console();
        Sleep(time);
    }

    void hide_cursor()
    {
        static const HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        CONSOLE_CURSOR_INFO info;
        info.dwSize = 1;    // works with every number from 1 to 100 as microsoft docs says
        info.bVisible = false;
        SetConsoleCursorInfo(consoleHandle, &info);
    }

    void clear_console()
    {
        // Flickering appears even if we use fillconsoleoutputcharacter(...)
        // implemeting double buffer teqniuque is quite long for me 
        // so just move cursor and draw the frame above the last one.
        static const HANDLE output_window = GetStdHandle(STD_OUTPUT_HANDLE);
        COORD position = { 0, 0 };
        SetConsoleCursorPosition(output_window, position);
    }
};


void snake_game_start()
{
    Game game_map;
    game_map.start();
}


int main()
{
    snake_game_start();
    return 0;
}



