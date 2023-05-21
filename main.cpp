#include <iostream>
#include <vector>
#include <stdlib.h>
#include <gtkmm.h>

using namespace Gtk;
using namespace Glib;

// some constants
const int size = 40;
const int padding = 2;
const int width = 8;
const int height = 16;

// all used shapes
const int SHAPES[][3][3] = {
    {
        {0, 0, 0},
        {1, 1, 1},
        {0, 0, 0},
    },

    {
        {1, 0, 0},
        {1, 1, 1},
        {0, 0, 0},
    },

    {
        {0, 0, 1},
        {1, 1, 1},
        {0, 0, 0},
    },

    {
        {1, 1, 0},
        {1, 1, 0},
        {0, 0, 0},
    },

    {
        {0, 1, 1},
        {1, 1, 0},
        {0, 0, 0},
    },

    {
        {0, 1, 0},
        {1, 1, 1},
        {0, 0, 0},
    },

    {
        {1, 1, 0},
        {0, 1, 1},
        {0, 0, 0},
    }
};

// all used colors
const GdkRGBA COLORS[] = {
    {0, 0, 0, 1},
    {1, 0, 0, 1},
    {0, 1, 0, 1},
    {0, 0, 1, 1},
    {1, 1, 0, 1},
    {1, 0, 1, 1},
    {0, 1, 1, 1},
    {1, 0.5, 0, 1}
};

class GameBoard {
public:
    int board[width][height];
    int temp[3][3];

    GameBoard() {
        reset();
    }

    // resets the whole board
    void reset() {
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                board[x][y] = 0;
            }
        }
    }

    // returns the color for given coordinates
    int color(int x, int y) {
        return board[x][y];
    }

    // returns true if the shape can be moved to given coordinates, otherwise false
    bool is_valid_move(int shape[3][3], int shape_x, int shape_y) {
        // check not empty points
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if (shape[x][y]) {
                    int new_x = shape_x + x;
                    int new_y = shape_y + y;

                    if (new_x < 0 || new_x >= width || new_y >= height || (new_y >= 0 && board[new_x][new_y])) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    // returns true if the shape can be rotated on given coordinates, otherwise false
    bool is_valid_rotate(int shape[3][3], int shape_x, int shape_y) {
        // copy source shape
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                temp[x][y] = shape[x][y];
            }
        }

        // simulate rotation
        for (int y = 0; y < 2; y++) {
            int value = temp[0][y];
            temp[0][y] = temp[2 - y][0];
            temp[2 - y][0] = temp[2][2 - y];
            temp[2][2 - y] = temp[y][2];
            temp[y][2] = value;
        }

        // check not empty points
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if (temp[x][y]) {
                    int new_x = shape_x + x;
                    int new_y = shape_y + y;

                    if (new_x < 0 || new_x >= width || new_y >= height || (new_y >= 0 && board[new_x][new_y])) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    // places the shape on the board
    void place_shape(int shape[3][3], int shape_x, int shape_y, int color) {
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if (shape[x][y]) {
                    int new_x = shape_x + x;
                    int new_y = shape_y + y;

                    if (new_y >= 0)
                        board[new_x][new_y] = color;
                }
            }
        }
    }

    // removes all full lines and returns removed count
    int remove_full_lines() {
        int last = height - 1;

        for (int y = height - 1; y >= 0; y--) {
            bool full_line = true;
            for (int x = 0; x < width; x++) {
                if (!board[x][y]) {
                    full_line = false;
                    break;
                }
            }

            if (!full_line) {
                for (int x = 0; x < width; x++) {
                    board[x][last] = board[x][y];
                }

                last--;
            }
        }

        for (int x = 0; x < width; x++) {
            for (int y = last; y >= 0; y--) {
                board[x][y] = 0;
            }
        }

        return last + 1;
    }

    // returns true if the game is over, otherwise false
    bool is_game_over() {
        for (int x = 0; x < width; x++)
            if (board[x][0])
                return true;

        return false;
    }
};

class GameArea : public DrawingArea {
public:
    GameBoard board;

    bool paused = false;
    bool gameover = false;

    int score = 0;

    int current_shape[3][3];
    int current_color;

    int current_x = 0;
    int current_y = 0;

    GameArea() {
        set_size_request(
            width * size + (width + 1) * padding,
            height * size + (height + 1) * padding
        );

        // connect area to draw signal
        signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context> &cairo) {
            draw(cairo);
            return true;
        });

        // connect area to timeout signal
        signal_timeout().connect([&]() {
            update();
            return true;
        }, 500);

        spawn_new_shape();
        show();
    }

    // draws the whole game area
    void draw(const Cairo::RefPtr<Cairo::Context> &cairo) {
        // draw background
        cairo->set_source_rgb(0.6, 0.6, 0.6);
        cairo->rectangle(0, 0, get_width(), get_height());

        cairo->fill();
        cairo->set_source_rgb(0, 0, 0);

        int x = 0;
        int y = 0;

        // draw vertical grid
        for (int i = 0; i <= width; i++) {
            cairo->rectangle(x, 0, padding, get_height());
            cairo->fill();

            x += size;
            x += padding;
        }

        // draw horizontal grid
        for (int i = 0; i <= height; i++) {
            cairo->rectangle(0, y, get_width(), padding);
            cairo->fill();

            y += size;
            y += padding;
        }

        // draw occupied cells
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                int color = board.color(x, y);
                if (color == 0) continue;

                draw_square(cairo, x, y, COLORS[color]);
            }
        }

        // draw current shape
        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                if (current_shape[x][y]) {
                    int point_x = current_x + x;
                    int point_y = current_y + y;

                    draw_square(cairo, point_x, point_y, COLORS[current_color]);
                }
            }
        }

        cairo->clip();
    }

    // draws a square on the given coordinates
    void draw_square(const Cairo::RefPtr<Cairo::Context> &cairo, int x, int y, GdkRGBA color) {
        cairo->set_source_rgb(color.red, color.green, color.blue);
        cairo->rectangle(
            x * size + (x + 1) * padding,
            y * size + (y + 1) * padding, size, size
        );

        cairo->fill();
    }

    // updates the current game state
    void update() {
        if (paused || gameover) return;

        if (board.is_valid_move(current_shape, current_x, current_y + 1)) {
            move_down();
            return;
        }

        board.place_shape(current_shape, current_x, current_y, current_color);
        score += board.remove_full_lines();

        if (board.is_game_over()) {
            gameover = true;
            return;
        }

        spawn_new_shape();
        queue_draw();
    }

    // resets everything
    void reset() {
        paused = false;
        gameover = false;

        score = 0;
        board.reset();

        spawn_new_shape();
        queue_draw();
    }

    // moves the current shape down
    void move_down() {
        current_y++;
        queue_draw();
    }

    // moves the current shape by x axis in the given direction (if it can be moved)
    void move_x(int x) {
        if (!board.is_valid_move(current_shape, current_x + x, current_y))
            return;

        current_x += x;
        queue_draw();
    }

    // rotates the current shape (if it can be rotated)
    void rotate() {
        if (!board.is_valid_rotate(current_shape, current_x, current_y))
            return;

        for (int y = 0; y < 2; y++) {
            int value = current_shape[0][y];
            current_shape[0][y] = current_shape[2 - y][0];
            current_shape[2 - y][0] = current_shape[2][2 - y];
            current_shape[2][2 - y] = current_shape[y][2];
            current_shape[y][2] = value;
        }

        queue_draw();
    }

    // spawns a new random shape with random color
    void spawn_new_shape() {
        int index = random(0, 7);

        for (int x = 0; x < 3; x++) {
            for (int y = 0; y < 3; y++) {
                current_shape[x][y] = SHAPES[index][x][y];
            }
        }

        current_color = random(1, 7);
        current_x = random(3, 5);
        current_y = -3;
    }

    // returns a random number in [min, max)
    int random(int min, int max) {
        return (rand() % (max - min)) + min;
    }
};

class Game {
public:
    Window *window;
    Fixed *fixed;

    Box *button_box;
    Label *score_label;
    Label *pause_label;

    Button *reset_button;
    Button *pause_button;
    Button *exit_button;

    GameArea area;

    // loads all game UI
    void load() {
        auto ui = Builder::create_from_file("project.glade");

        ui->get_widget("window", window);
        ui->get_widget("fixed", fixed);

        fixed->add(area);
        fixed->move(area, size, size);

        ui->get_widget("button_box", button_box);
        ui->get_widget("score_label", score_label);
        ui->get_widget("pause_label", pause_label);

        ui->get_widget("reset_button", reset_button);
        ui->get_widget("pause_button", pause_button);
        ui->get_widget("exit_button", exit_button);

        window->signal_key_press_event().connect([&](const GdkEventKey *event) {
            std::cout << event->keyval << std::endl;

            if (area.paused || area.gameover)
                return true;

            switch(event->keyval) {
                // move left
                case 1734: case GDK_KEY_a:
                    area.move_x(-1);
                    break;

                // move right
                case 1751: case GDK_KEY_d:
                    area.move_x(1);
                    break;

                // rotate
                case 1753: case GDK_KEY_s:
                    area.rotate();
                    break;
            }

            return true;
        });

        // reset button resets the game
        reset_button->signal_clicked().connect([&]() {
            area.reset();
        });

        // pause button pauses the game
        pause_button->signal_clicked().connect([&]() {
            area.paused = !area.paused;
        });

        // exit button exits the game
        // very useful feature, I know
        exit_button->signal_clicked().connect([&]() {
            std::exit(0);
        });

        // connect game area to timeout signal
        // essentially it updates the entire UI based on the state of the game
        signal_timeout().connect([&]() {
            // the game is over, show that
            if (area.gameover) {
                pause_label->set_opacity(1);
                pause_label->set_text("Game Over");

                pause_button->set_sensitive(false);
                return true;
            }

            // the game is paused, show that
            if (area.paused) {
                pause_label->set_opacity(1);
                pause_label->set_text("Game Paused");

                pause_button->set_sensitive(true);
                pause_button->set_label("Resume");

                pause_button->set_tooltip_text("Resume the game");
                set_icon(pause_button, "media-playback-pause-symbolic");
            } else {
                score_label->set_text("Score: " + std::to_string(area.score));

                pause_label->set_opacity(0);
                pause_label->set_text("");

                pause_button->set_sensitive(true);
                pause_button->set_label("Pause");

                pause_button->set_tooltip_text("Pause the game");
                set_icon(pause_button, "media-playback-start-symbolic");
            }

            return true;
        }, 10);

        // set random seed
        srand((unsigned) time (NULL));
    }

    // why am I even supposed to do this?
    void set_icon(Button *button, ustring icon) {
        int margin_start = button->get_image()->get_margin_start();
        int margin_end = button->get_image()->get_margin_end();
        int margin_top = button->get_image()->get_margin_top();
        int margin_bottom = button->get_image()->get_margin_bottom();

        button->set_image_from_icon_name(icon);

        button->get_image()->set_margin_start(margin_start);
        button->get_image()->set_margin_end(margin_end);
        button->get_image()->set_margin_top(margin_top);
        button->get_image()->set_margin_bottom(margin_bottom);
    }
};

int main(int argc, char** argv) {
    auto app = Application::create(argc, argv);

    Game game;
    game.load();

    return app->run(*game.window);
}
