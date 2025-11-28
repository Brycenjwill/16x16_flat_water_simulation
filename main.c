#include <SDL2/SDL.h>
#include <string.h>

#define WINDOW_SIZE 640 // Display is a square
#define GRID_SIZE 16
#define WATER_PIXEL_SIZE (WINDOW_SIZE / GRID_SIZE) // Water is represented as pixels

uint8_t display[16][16] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
    {0,0,0,0,0,1,0,1,0,1,0,0,0,1,0,0},
    {0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,1,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

uint8_t next_display[16][16];

int get_semi_random_direction(int column, int row) {
  if (row % 2) {
    return column % 2;
  }
  return row % 2;
}

int column_to_x_val(int column) {
  // Get starting x value of column
  return column * WATER_PIXEL_SIZE;
};

int row_to_y_val(int row) {
  // Get starting y value of row
  return row * WATER_PIXEL_SIZE;
};

// Check if the target grid lcoation exists
int target_within_bounds(int gravity_x, int gravity_y, int row, int column) {
  // Check if a future x location is off the display (column x value is the left border)
  if ((column + gravity_x) >= GRID_SIZE) {
    return 0;
  }
  if ((column + gravity_x) < 0) {
    return 0;
  }

  // Check if a future y location is off the display (column x value is the left border)
  if ((row + gravity_y) >= GRID_SIZE) {
    return 0;
  }
  if ((row + gravity_y) < 0) {
    return 0;
  }

  // Good to go!
  return 1;
};

// Check if the target grid location is empty
int target_empty(int gravity_x, int gravity_y, int row, int column) {
  // Check if a future location is already populated with water
  if (display[row + gravity_y][column + gravity_x] == 1) {
    return 0;
  }
  return 1;
}

void update_water_grid(int gravity_x, int gravity_y) {
    memset(next_display, 0, sizeof(next_display)); // Reset next_gri

    for (int row = 0; row < 16; row ++) {
      for (int column = 0; column < 16; column ++) {
        if (display[row][column] == 1) {
          // Make sure target is within bounds
          if (target_within_bounds(gravity_x, gravity_y, row, column)) {
            if (target_empty(gravity_x, gravity_y, row, column)) {
              next_display[row][column] = 0;
              next_display[row + gravity_y][column + gravity_x] = 1;
            }
            else {
              // Still want the water to exist, just not move
              next_display[row][column] = 1;
            }
          }
          else {
            // Still want the water to exist, just not move
            next_display[row][column] = 1;
          }
        }
      }
    }

    memcpy(display, next_display, sizeof(display)); // Copy next_grid onto display
};

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* win = SDL_CreateWindow("Water Sim",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_SIZE, WINDOW_SIZE, 0);

    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, 0);

    int running = 1;
    SDL_Event e;

    const int FRAME_TIME = 128; // ~8 FPS limit

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
      float gravity_x = 0;
      float gravity_y = 0;

      // Create gravity values
      if (left)  gravity_x -= 1;
      if (right) gravity_x += 1;
      if (up)    gravity_y -= 1;
      if (down)  gravity_y += 1;

      update_water_grid(gravity_x, gravity_y);

      for (int row = 0; row < 16; row ++) {
        for (int column = 0; column < 16; column ++) {
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
