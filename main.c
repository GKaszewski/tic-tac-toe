#include "SDL.h"
#include <stdlib.h>

#define PIXEL_SIZE 64
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

typedef struct Cell {
    int x;
    int y;
    char value;
} Cell;

typedef struct Grid {
    int width;
    int height;
    Cell *cells;
} Grid;

typedef struct GameState {
    Grid grid;
    int player_turn;
    int game_over;
    Cell *player_characters;
    Cell *opponent_characters;
    int player_character_count;
    int opponent_character_count;
} GameState;

void initialize_cell(Cell *cell, int x, int y) {
    cell->x = x;
    cell->y = y;
    cell->value = ' ';
}

Cell *get_cell(Grid *grid, int x, int y) {
    return &grid->cells[y * grid->width + x];
}

char get_random_character() {
    return rand() % 2 == 0 ? 'x' : 'o';
}


void initialize_grid(Grid *grid, int width, int height) {
    grid->width = width;
    grid->height = height;
    grid->cells = malloc(sizeof(Cell) * width * height);
    for (int i = 0; i < width * height; i++) {
        initialize_cell(&grid->cells[i], i % width, i / width);
    }
}

void destroy_grid(Grid *grid) {
    free(grid->cells);
}

void initialize_game_state(GameState *game_state, int width, int height) {
    initialize_grid(&game_state->grid, width, height);
    game_state->player_turn = SDL_TRUE;
    game_state->game_over = SDL_FALSE;
    game_state->player_characters = malloc(sizeof(Cell) * 9);
    game_state->opponent_characters = malloc(sizeof(Cell) * 9);
    game_state->player_character_count = 0;
    game_state->opponent_character_count = 0;
}

void destroy_game_state(GameState *game_state) {
    destroy_grid(&game_state->grid);
    free(game_state->player_characters);
    free(game_state->opponent_characters);
}

Cell *get_cell_by_mouse_click(GameState *game_state, int x, int y) {
    int offset_x = (SCREEN_WIDTH - game_state->grid.width * PIXEL_SIZE) / 2;
    int offset_y = (SCREEN_HEIGHT - game_state->grid.height * PIXEL_SIZE) / 2;
    if (x < offset_x || x > offset_x + game_state->grid.width * PIXEL_SIZE) {
        return NULL;
    }
    if (y < offset_y || y > offset_y + game_state->grid.height * PIXEL_SIZE) {
        return NULL;
    }
    int cell_x = (x - offset_x) / PIXEL_SIZE;
    int cell_y = (y - offset_y) / PIXEL_SIZE;
    return get_cell(&game_state->grid, cell_x, cell_y);
}

void set_cell_value(Cell *cell, char value) {
    cell->value = value;
}

int is_cell_empty(Cell *cell) {
    return cell->value == ' ';
}

// returns 1 if player won, 2 if opponent won, 0 otherwise
int check_winner(GameState *game_state) {
    int player_won = SDL_FALSE;
    int opponent_won = SDL_FALSE;

    //check rows
    for (int x = 0; x < game_state->grid.width; x++) {
        int player_count = 0;
        int opponent_count = 0;
        for (int y = 0; y < game_state->grid.height; y++) {
            Cell *cell = get_cell(&game_state->grid, x, y);
            if (cell->value == game_state->player_characters[0].value) {
                player_count++;
            } else if (cell->value == game_state->opponent_characters[0].value) {
                opponent_count++;
            }
        }
        if (player_count == 3) {
            player_won = SDL_TRUE;
        }
        if (opponent_count == 3) {
            opponent_won = SDL_TRUE;
        }
    }

    //check columns
    for (int y = 0; y < game_state->grid.height; y++) {
        int player_count = 0;
        int opponent_count = 0;
        for (int x = 0; x < game_state->grid.width; x++) {
            Cell *cell = get_cell(&game_state->grid, x, y);
            if (cell->value == game_state->player_characters[0].value) {
                player_count++;
            } else if (cell->value == game_state->opponent_characters[0].value) {
                opponent_count++;
            }
        }
        if (player_count == 3) {
            player_won = SDL_TRUE;
        }
        if (opponent_count == 3) {
            opponent_won = SDL_TRUE;
        }
    }

    //check diagonals
    int player_count = 0;
    int opponent_count = 0;
    for (int i = 0; i < game_state->grid.width; i++) {
        Cell *cell = get_cell(&game_state->grid, i, i);
        if (cell->value == game_state->player_characters[0].value) {
            player_count++;
        } else if (cell->value == game_state->opponent_characters[0].value) {
            opponent_count++;
        }
    }

    if (player_count == 3) {
        player_won = SDL_TRUE;
    }
    if (opponent_count == 3) {
        opponent_won = SDL_TRUE;
    }

    player_count = 0;
    opponent_count = 0;

    if (player_won) return 1;
    if (opponent_won) return 2;
    return 0;
}

void draw_grid(SDL_Renderer *renderer, int width, int height, int offset_x, int offset_y) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0x00);
    for (int x = 0; x < 1 + width * PIXEL_SIZE; x += PIXEL_SIZE) {
        SDL_RenderDrawLine(renderer, x + offset_x, offset_y, x + offset_x, offset_y + height * PIXEL_SIZE);
    }
    for (int y = 0; y < 1 + height * PIXEL_SIZE; y += PIXEL_SIZE) {
        SDL_RenderDrawLine(renderer, offset_x, y + offset_y, offset_x + width * PIXEL_SIZE, y + offset_y);
    }
}

void draw_x(SDL_Renderer *renderer, int x, int y, int offset_x, int offset_y) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 0x00);
    SDL_RenderDrawLine(renderer, x * PIXEL_SIZE + 10 + offset_x, y * PIXEL_SIZE + 10 + offset_y, (x + 1) * PIXEL_SIZE - 10 + offset_x,
                       (y + 1) * PIXEL_SIZE - 10 + offset_y);
    SDL_RenderDrawLine(renderer, x * PIXEL_SIZE + 10 + offset_x, (y + 1) * PIXEL_SIZE - 10 + offset_y, (x + 1) * PIXEL_SIZE - 10 + offset_x,
                       y * PIXEL_SIZE + 10 + offset_y);
}

void draw_o(SDL_Renderer *renderer, int x, int y, int offset_x, int offset_y) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 0x00);
    SDL_Rect rect = {x * PIXEL_SIZE + 10 + offset_x, y * PIXEL_SIZE + 10 + offset_y, PIXEL_SIZE - 20, PIXEL_SIZE - 20};
    SDL_RenderDrawRect(renderer, &rect);
}

int main(int argc, char **argv) {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }
    SDL_SetWindowTitle(window, "TicTacToe");
    SDL_RenderSetVSync(renderer, 1);

    GameState game_state;
    initialize_game_state(&game_state, 3, 3);

    while (SDL_TRUE) {
        while (game_state.player_turn == SDL_FALSE && game_state.game_over == SDL_FALSE) {
            int x = rand() % 3;
            int y = rand() % 3;
            Cell *cell = get_cell(&game_state.grid, x, y);
            if (is_cell_empty(cell)) {
                set_cell_value(cell, 'o');
                game_state.opponent_characters[game_state.opponent_character_count] = *cell;
                game_state.opponent_character_count++;
                game_state.player_turn = SDL_TRUE;
            }
        }

        SDL_PollEvent(&event);
        if (event.type == SDL_QUIT) {
            break;
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (game_state.game_over) {
                initialize_game_state(&game_state, 3, 3);
                continue;
            }
            if (game_state.player_turn) {
                Cell *cell = get_cell_by_mouse_click(&game_state, event.button.x, event.button.y);
                if (cell != NULL && is_cell_empty(cell)) {
                    set_cell_value(cell, 'x');
                    game_state.player_characters[game_state.player_character_count] = *cell;
                    game_state.player_character_count++;
                    game_state.player_turn = SDL_FALSE;
                }
            }
        }

        int winner = check_winner(&game_state);
        if (winner == 1) {
            game_state.game_over = SDL_TRUE;
            printf("Player won!\n");
        } else if (winner == 2) {
            game_state.game_over = SDL_TRUE;
            printf("Opponent won!\n");
        }

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
        SDL_RenderClear(renderer);
        draw_grid(renderer, 3, 3, (SCREEN_WIDTH - 3 * PIXEL_SIZE) / 2, (SCREEN_HEIGHT - 3 * PIXEL_SIZE) / 2);
        for (int i = 0; i < 9; i++) {
            Cell *cell = &game_state.grid.cells[i];
            if (cell->value == 'x') {
                draw_x(renderer, cell->x, cell->y, (SCREEN_WIDTH - 3 * PIXEL_SIZE) / 2, (SCREEN_HEIGHT - 3 * PIXEL_SIZE) / 2);
            } else if (cell->value == 'o') {
                draw_o(renderer, cell->x, cell->y, (SCREEN_WIDTH - 3 * PIXEL_SIZE) / 2, (SCREEN_HEIGHT - 3 * PIXEL_SIZE) / 2);
            }
        }
        SDL_RenderPresent(renderer);
    }

    destroy_game_state(&game_state);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}