#include <SDL2/SDL.h>
#include <string.h>

// Display is a square
#define GRID_SIZE 16
#define WINDOW_SIZE 640
#define WATER_PIXEL_SIZE (WINDOW_SIZE / GRID_SIZE)

#define FPS 16
struct position {
  int row;
  int column;
};
uint8_t display[GRID_SIZE][GRID_SIZE];

uint8_t next_display[GRID_SIZE][GRID_SIZE];

// Just don't want water to flow the same way every time
int semi_random(int column, int row) {
    static int ticker = 0;
    ticker++;
    return (ticker + row + column) & 1;
}


int column_to_x_val(int column) {
  // Get starting x value of column
  return column * WATER_PIXEL_SIZE;
};

int row_to_y_val(int row) {
  // Get starting y value of row
  return row * WATER_PIXEL_SIZE;
};


int valid_location(int row, int column) {
  if (row < 0 || row >= GRID_SIZE) {
    return 0;
  }
  if (column < 0 || column >= GRID_SIZE) {
    return 0;
  }
  return 1;
}

// Check if the target grid location exists
int target_within_bounds(int gravity_x, int gravity_y, int row, int column) {
  // Check if a future x location is off the display (column x value is the left border)
  return valid_location(row + gravity_y, column + gravity_x);
};

// Check if grid location is empty. Assumes location is valid
int is_empty(int row, int column) {
  if (display[row][column] == 1) {
    return 0;
  }
  return 1;
}

// Check if a location in next_display is empty. Assumes location is valid
int next_display_empty(int row, int column) {
  if (next_display[row][column] == 1) {
    return 0;
  }
  return 1;
}

// Check if the target grid location is empty
int target_empty(int gravity_x, int gravity_y, int row, int column) {
  // Check if a future location is already populated with water
  return is_empty(row + gravity_y, column + gravity_x) && next_display_empty(row + gravity_y, column + gravity_x);
}

struct position get_flow_location(int gravity_x, int gravity_y, int row, int column) {
  struct position target_location;
  int rand = semi_random(column, row);

  // If moving diaganol, check if we can flow in either the x or y direction as per gravity
  if (gravity_x && gravity_y) {
    if (rand) {
      if (valid_location(row, column + gravity_x)) {
        if (is_empty(row, column + gravity_x) && next_display_empty(row, column + gravity_x)) {
          target_location.row = row;
          target_location.column = column + gravity_x;
          return target_location;
        }
      }
      if (valid_location(row + gravity_y, column)) {
        if (is_empty(row + gravity_y, column) && next_display_empty(row + gravity_y, column)) {
          target_location.row = row + gravity_y;
          target_location.column = column;
          return target_location;
        }
      }
    }
    target_location.row = row;
    target_location.column = column;
    return target_location;
  }

  // If moving vertical, check if we can flow left or right diaganolly.
  // If not, check if we can flow just left or right
  if (gravity_y) {
    // Diaganol checks
    if (rand) {
      if (valid_location(row + gravity_y, column + 1)) {
        if (is_empty(row + gravity_y, column + 1) && next_display_empty(row + gravity_y, column + 1)) {
          target_location.row = row + gravity_y;
          target_location.column = column + 1;
          return target_location;
        }
      }
    }
    else if (valid_location(row + gravity_y, column - 1)) {
      if (is_empty(row + gravity_y, column - 1) && next_display_empty(row + gravity_y, column - 1)) {
        target_location.row = row + gravity_y;
        target_location.column = column - 1;
        return target_location;
      }
    }
    // Horizontal checks
    if (rand) {
      if (valid_location(row, column - 1)) {
        if (is_empty(row, column - 1) && next_display_empty(row, column - 1)) {
          target_location.row = row;
          target_location.column = column - 1;
          return target_location;
        }
      }
    }
    else if (valid_location(row, column + 1)) {
      if (is_empty(row, column + 1) && next_display_empty(row, column + 1)) {
        target_location.row = row;
        target_location.column = column + 1;
        return target_location;
      }
    }
  }

  // If moving horizontal, check if we can flow up or down diaganolly.
  // If not, check if we can flow just up or down.
  if (gravity_x) {
    // Diaganol checks
    if (rand) {
      if (valid_location(row + 1, column + gravity_x)) {
        if (is_empty(row + 1, column + gravity_x) && next_display_empty(row + 1, column + gravity_x)) {
          target_location.row = row + 1;
          target_location.column = column + gravity_x;
          return target_location;
        }
      }
    else if (valid_location(row - 1, column + gravity_x)) {
      if (is_empty(row - 1, column + gravity_x) && next_display_empty(row - 1, column + gravity_x)) {
        target_location.row = row - 1;
        target_location.column = column + gravity_x;
        return target_location;
      }
    }
    }
    // Vertical checks
    if (rand) {
    if (valid_location(row - 1, column)) {
      if (is_empty(row - 1, column) && next_display_empty(row - 1, column)) {
        target_location.row = row - 1;
        target_location.column = column;
        return target_location;
      }
    }
    }
    else if (valid_location(row + 1, column)) {
      if (is_empty(row + 1, column) && next_display_empty(row + 1, column)) {
        target_location.row = row + 1;
        target_location.column = column;
        return target_location;
      }
    }
  }
  target_location.row = row;
  target_location.column = column;
  return target_location;
}

void update_water_grid(int gravity_x, int gravity_y) {
    memset(next_display, 0, sizeof(next_display)); // Reset next_gri

    // Determine row iteration order
    int row_start, row_end, row_step;
    if (gravity_y > 0) {               // moving DOWN -> iterate bottom→top
        row_start = GRID_SIZE - 1; row_end = -1; row_step = -1;
    } else if (gravity_y < 0) {        // moving UP -> iterate top→bottom
        row_start = 0; row_end = GRID_SIZE; row_step = 1;
    } else {                           // no vertical -> default
        row_start = 0; row_end = GRID_SIZE; row_step = 1;
    }

    // Determine column iteration order
    int col_start, col_end, col_step;
    if (gravity_x > 0) {               // moving RIGHT -> iterate right→left
        col_start = GRID_SIZE - 1; col_end = -1; col_step = -1;
    } else if (gravity_x < 0) {        // moving LEFT -> iterate left→right
        col_start = 0; col_end = GRID_SIZE; col_step = 1;
    } else {                           // no horizontal -> default
        col_start = 0; col_end = GRID_SIZE; col_step = 1;
    }

    for (int row = row_start; row != row_end; row += row_step) {
        for (int column = col_start; column != col_end; column += col_step) {
            if (display[row][column] != 1) // We only need to check movement on locations with water
                continue;
            if (target_within_bounds(gravity_x, gravity_y, row, column)) {
                if (target_empty(gravity_x, gravity_y, row, column)) {
                    next_display[row + gravity_y][column + gravity_x] = 1;
                }
                else {
                    struct position flow_location = get_flow_location(gravity_x, gravity_y, row, column);
                    next_display[flow_location.row][flow_location.column] = 1;
                }
            }
            else {
                next_display[row][column] = 1;
            }
        }
    }

    memcpy(display, next_display, sizeof(display)); // Copy next_grid onto display
};

int main() {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int column = 0; column < GRID_SIZE; column++) {
      if (row >= (int)(GRID_SIZE * .75)) {
        display[row][column] = 1;
      }
      else {
        display[row][column] = 0;
      }
    }
  }

  SDL_Init(SDL_INIT_VIDEO);

  SDL_Window* win = SDL_CreateWindow("Water Sim",
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      WINDOW_SIZE, WINDOW_SIZE, 0);

  SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

  int running = 1;
  SDL_Event e;

  const int FRAME_TIME = 1000 / FPS;

  while (running) {
    Uint32 frame_start = SDL_GetTicks();

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) running = 0;
    }

    SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
    SDL_RenderClear(ren);

    // Read key input
    const Uint8 *keys = SDL_GetKeyboardState(NULL);

    int up    = keys[SDL_SCANCODE_UP];
    int down  = keys[SDL_SCANCODE_DOWN];
    int left  = keys[SDL_SCANCODE_LEFT];
    int right = keys[SDL_SCANCODE_RIGHT];

    // Convert to gravity vector
    int gravity_x = 0;
    int gravity_y = 0;

    // Create gravity values
    if (left)  gravity_x -= 1;
    if (right) gravity_x += 1;
    if (up)    gravity_y -= 1;
    if (down)  gravity_y += 1;

    update_water_grid(gravity_x, gravity_y);

    for (int row = 0; row < GRID_SIZE; row ++) {
      for (int column = 0; column < GRID_SIZE; column ++) {
        if (display[row][column] == 1) {
          // Draw water
          SDL_SetRenderDrawColor(ren, 0, 120, 255, 255);
          SDL_Rect r = { column_to_x_val(column), row_to_y_val(row), WATER_PIXEL_SIZE, WATER_PIXEL_SIZE };
          SDL_RenderFillRect(ren, &r);
        }
      }
    }

    // Limit framerate
    Uint32 frame_duration = SDL_GetTicks() - frame_start;
    if (frame_duration < FRAME_TIME) {
        SDL_Delay(FRAME_TIME - frame_duration);
    }

    SDL_RenderPresent(ren);
  }

  SDL_DestroyRenderer(ren);
  SDL_DestroyWindow(win);
  SDL_Quit();
}
