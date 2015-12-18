#include <kernel.h>
#include "../include/kernel.h"

#define MAZE_WIDTH  19
#define MAZE_HEIGHT 16
#define GHOST_CHAR  0x02

#define DIRECTION_UP    0
#define DIRECTION_DOWN  1
#define DIRECTION_LEFT  2
#define DIRECTION_RIGHT 3

typedef struct {
    int x;
    int y;
    int direction;
} GHOST;

WINDOW* pacman_wnd;

char* maze[] = {
        "r--------T--------i",
        "|        |        |",
        "| ri r-i | r-i ri |",
        "| Ll L-l | L-l Ll |",
        "|                 |",
        "| -- | --T-- | -- |",
        "|    |   |   |    |",
        "E--- L--   --l ---e",
        "|        |        |",
        "| -i --- | --- r- |",
        "|  |           |  |",
        "E- | | --T-- | | -e",
        "|    |   |   |    |",
        "| ---t-- | --t--- |",
        "|                 |",
        "L-----------------l",
        NULL
};

void draw_maze_char(char maze_char) {
    char ch = ' ';

    // For details of PC-ASCII characters see:
    // http://www.jimprice.com/jim-asc.shtml
    switch (maze_char) {
        case '|':
            ch = 0xB3;
            break;
        case '-':
            ch = 0xC4;
            break;
        case 'r':
            ch = 0xDA;
            break;
        case 'i':
            ch = 0xBF;
            break;
        case 'L':
            ch = 0xC0;
            break;
        case 'l':
            ch = 0xD9;
            break;
        case 'T':
            ch = 0xC2;
            break;
        case 't':
            ch = 0xC1;
            break;
        case 'E':
            ch = 0xC3;
            break;
        case 'e':
            ch = 0xB4;
            break;
        default:
            ch = 0x20;
            break;
    }
    output_char(pacman_wnd, ch);
}

void draw_maze() {
    int x, y;

    clear_window(pacman_wnd);
    y = 0;
    while (maze[y] != NULL) {
        char* row = maze[y];
        x = 0;
        while (row[x] != '\0') {
            char ch = row[x];
            draw_maze_char(ch);
            x++;
        }
        y++;
    }
    wprintf(pacman_wnd, "PacMan ");
}

// Pseudo random number generator
// http://cnx.org/content/m13572/latest/
int seed = 17489;
int last_random_number = 0;

int random() {
    last_random_number = (25173 * last_random_number + 13849) % 65536;
    return last_random_number;
}

BOOL can_ghost_move_up(GHOST* ghost) {
    return (maze[ghost->y - 1][ghost->x] == ' ');
}

BOOL can_ghost_move_down(GHOST* ghost) {
    return (maze[ghost->y + 1][ghost->x] == ' ');
}

BOOL can_ghost_move_left(GHOST* ghost) {
    return (maze[ghost->y][ghost->x - 1] == ' ');
}

BOOL can_ghost_move_right(GHOST* ghost) {
    return (maze[ghost->y][ghost->x + 1] == ' ');
}

void move_ghost(GHOST* ghost) {
    volatile int lock;
    DISABLE_INTR(lock);

    remove_cursor(pacman_wnd);

    if(ghost->direction == DIRECTION_UP) {
        if(can_ghost_move_up(ghost)) {
            ghost->y--;
        } else {
            ghost->direction = random() % 4;
        }
    } else if(ghost->direction == DIRECTION_LEFT) {
        if(can_ghost_move_left(ghost)) {
            ghost->x--;
        } else {
            ghost->direction = random() % 4;
        }
    } else if(ghost->direction == DIRECTION_DOWN) {
        if(can_ghost_move_down(ghost)) {
            ghost->y++;
        } else {
            ghost->direction = random() % 4;
        }
    } else if(ghost->direction == DIRECTION_RIGHT) {
        if(can_ghost_move_right(ghost)) {
            ghost->x++;
        } else {
            ghost->direction = random() % 4;
        }
    }

    move_cursor(pacman_wnd, ghost->x, ghost->y);
    show_cursor(pacman_wnd);
    ENABLE_INTR(lock);
}

void init_ghost(GHOST* ghost) {
    while(TRUE) {
        int x = random() % MAZE_WIDTH;
        int y = random() % MAZE_HEIGHT;

        if (maze[y][x] != ' ') {
            continue;
        }

        ghost->x = x;
        ghost->y = y;
        ghost->direction = random() % 4;

        break;
    }
}

void create_new_ghost() {
    GHOST ghost;
    init_ghost(&ghost);

    while(TRUE) {
        move_cursor(pacman_wnd, ghost.x, ghost.y);
        move_ghost(&ghost);
        sleep(random() % 25 + 10);
    }
}

void ghost_process(PROCESS self, PARAM param) {
    create_new_ghost();
    resign();
}

void init_pacman(WINDOW* wnd, int num_ghosts) {
    pacman_wnd = wnd;
    pacman_wnd->width = MAZE_WIDTH;
    pacman_wnd->height = MAZE_HEIGHT + 1;
    pacman_wnd->cursor_char = GHOST_CHAR;

    draw_maze();

    int j;
    for (j = 0; j < num_ghosts; j++) {
        create_process(ghost_process, 3, 0, "Ghost");
    }
}

