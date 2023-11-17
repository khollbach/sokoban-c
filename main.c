#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <stdbool.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef int8_t i8;

typedef struct {
    u8 world[9][8];
    i8 player_x;
    i8 player_y;
} game;

void load_world(game *g);
void draw_world(game *g);
bool check_win(game *g);
void gr();
void gr_clear();
void beep();
u16 gr_coord_to_addr(u8 x, u8 y);

char _read_val; // needed so the compiler doesn't optimize away the read
#define ADDR(a) ((char*)a)
#define READ(a) (_read_val = *ADDR(a))
#define WRITE(a, x) (*ADDR(a) = x)

enum low_res_color {
    black = 0,
    magenta = 1,
    dark_blue = 2,
    purple = 3,
    dark_green = 4,
    grey_1 = 5,
    medium_blue = 6,
    light_blue = 7,
    brown = 8,
    orange = 9,
    grey_2 = 10,
    pink = 11,
    green = 12,
    yellow = 13,
    aqua = 14,
    white = 15,
};

enum object {
    floor = 0b0000,
    wall = 0b0001,
    box = 0b0010,
    target = 0b0100,
    player = 0b1000,
};

enum object_color {
    floor_color = grey_1,
    wall_color = magenta,
    box_color = brown,
    target_color = orange,
    player_color = dark_blue,
};

const u8 level_x = 8;
const u8 level_y = 9;
const u8 level[9][8] = {
    {0, 0, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 0, 0, 0, 1, 0},
    {1, 4, 8, 2, 0, 0, 1, 0},
    {1, 1, 1, 0, 2, 4, 1, 0},
    {1, 4, 1, 1, 2, 0, 1, 0},
    {1, 0, 1, 0, 4, 0, 1, 0},
    {1, 2, 0, 6, 2, 2, 4, 1},
    {1, 0, 0, 0, 4, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
};

int main() {
    game g;
    u8 key, *tile, *behind;
    i8 key_x, key_y;

    gr();
    gr_clear();
    load_world(&g);

    while (1) {
        if (check_win(&g)) {
            cprintf("you win!");
            beep();
            cgetc();
            return 0;
        }
        draw_world(&g);
        key = cgetc();

        key_x = key_y = 0;
        switch (key) { // WASD: move
        case 82: case 114: // R: reset
            return main();
        case 87: case 119: // up
            key_y = -1;
            break;
        case 83: case 115: // down
            key_y = 1;
            break;
        case 65: case 97: // left
            key_x = -1;
            break;
        case 68: case 100: // right
            key_x = 1;
            break;
        }

        tile = &g.world[g.player_y + key_y][g.player_x + key_x];
        if (*tile & wall) continue;
        if (*tile & box) {
            behind = &g.world[g.player_y + key_y*2][g.player_x + key_x*2];
            if (*behind & wall || *behind & box) continue;

            // Push the box.
            *tile ^= box;
            *behind ^= box;
        }

        g.player_x += key_x;
        g.player_y += key_y;
    }

    return 0;
}

void load_world(game *g) {
    u8 x, y;
    i8 nil = -1;
    memcpy(g->world, level, level_x * level_y);

    // Extract the player.
    g->player_x = nil;
    g->player_y = nil;
    for (y = 0; y < level_y; y++) {
        for (x = 0; x < level_x; x++) {
            if (g->world[y][x] & player) {
                g->world[y][x] ^= player;
                assert(g->player_x == nil && g->player_y == nil);
                g->player_x = x;
                g->player_y = y;
            }
            assert(!(g->world[y][x] & player));
        }
    }
    assert(g->player_x != nil && g->player_y != nil);
}

void draw_world(game *g) {
    u8 x, y, tile, color;
    for (y = 0; y < level_y; y++) {
        for (x = 0; x < level_x; x++) {
            tile = g->world[y][x];

            color = floor_color;
            if (tile & target) {
                color = target_color;
                tile ^= target;
            }
            if (tile == wall) color = wall_color;
            if (tile == box) color = box_color;
            if (x == g->player_x && y == g->player_y) {
                color = player_color;
            }

            WRITE(gr_coord_to_addr(x, y), color << 4 | color);
        }
    }
}

bool check_win(game *g) {
    u8 x, y, tile;
    for (y = 0; y < level_y; y++) {
        for (x = 0; x < level_x; x++) {
            tile = g->world[y][x];
            if (!!(tile & box) != !!(tile & target)) {
                return false;
            }
        }
    }
    return true;
}

// Activate graphics mode.
void gr() {
    WRITE(0xC050, 0);
}

void gr_clear() {
    // Note that this clobbers screen holes.
    memset(ADDR(0x400), 0, 0x400);
}

void beep() {
    cprintf("\a");
}

u16 gr_coord_to_addr(u8 x, u8 y) {
    u8 group;
    u16 base, offset;
    assert(x < 40);
    assert(y < 24);

    group = y / 8;
    switch (group) {
    case 0:
        base = 0x400;
        break;
    case 1:
        base = 0x428;
        break;
    case 2:
        base = 0x450;
        break;
    }

    offset = y % 8 * 0x80;

    return base + offset + x;
}
