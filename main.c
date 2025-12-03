#include <SDL2/SDL.h>
#include <string.h>
#include <math.h> 

// Display is a square
#define GRID_SIZE 16
#define WINDOW_SIZE 640
#define WATER_PIXEL_SIZE (WINDOW_SIZE / GRID_SIZE)

// Useful constants for physics simulation
#define MAX_MASS_AT_REST  1.0f // Max mass when no pressure is being applied
#define MIN_MASS          .0001f // The minimum amount of water in a cell for it to be considered to have water
#define FLOW_RATE         0.5f // Dampening factor for flow per frame. 1.0 causes oscillatoin
#define FIXED_TIMESTEP    4 // Updates per frame
#define MAX_COMPRESS      (MAX_MASS_AT_REST * 1.5f) // Allows water to compress into a cell when acted on by gravity
#define MIN_FLOW_DIFF     .005f
#define HORIZONTAL_RATE   .5f
#define RENDER_THRESHOLD 0.05f

#define FPS 30

float current_mass[16][16];
float new_mass[16][16];

int index_to_dim(int index) {
  return WATER_PIXEL_SIZE * index;
}

void update_current_mass(int gravity_x, int gravity_y) {
    for(int r = 0; r < GRID_SIZE; r ++) {
        for(int c = 0; c < GRID_SIZE; c ++) {
            float mass = current_mass[r][c];
            if (mass < MIN_MASS) {
                // If it's air, move its mass (0) to new_mass and continue
                new_mass[r][c] += mass;
                continue;
            }

            int gr = r + gravity_y;
            int gc = c + gravity_x;

            // 1. Direct gravity flow
            if (gr >= 0 && gr < GRID_SIZE && gc >= 0 && gc < GRID_SIZE) {
                
                float neighbor_mass = current_mass[gr][gc];
                float remaining_space = MAX_COMPRESS - neighbor_mass; // Use MAX_MASS constant

                if (remaining_space > MIN_MASS) { // Only attempt flow if space exists
                    
                    float available_flow = fminf(mass, remaining_space);
                    float flow_amount = available_flow * FLOW_RATE;
                    new_mass[gr][gc] += flow_amount;
                    mass -= flow_amount; // Mass remaining in source cell
                }
            } 

            // --- 2. Overflow handling
            int ar = r - gravity_y; 
            int ac = c - gravity_x; 

            if ((gravity_x != 0 || gravity_y != 0) && 
                (mass > MAX_MASS_AT_REST) && 
                (ar >= 0 && ar < GRID_SIZE && ac >= 0 && ac < GRID_SIZE)) 
            {
                
                float overflow_amount = mass - MAX_MASS_AT_REST; 
                
                new_mass[ar][ac] += overflow_amount;
                
                mass -= overflow_amount; 
                
            }

            // --- 3. Level out the mass

            int is_gravity_off = (gravity_x == 0 && gravity_y == 0); 

            int lr1 = r - gravity_x; int lc1 = c + gravity_y;
            int lr2 = r + gravity_x; int lc2 = c - gravity_y;

            if (lr1 >= 0 && lr1 < GRID_SIZE && lc1 >= 0 && lc1 < GRID_SIZE) {
                float neighbor_mass = current_mass[lr1][lc1];
                float mass_diff = mass - neighbor_mass;

                if (mass_diff > MIN_MASS && (mass_diff > MIN_FLOW_DIFF || is_gravity_off)) {
                    
                    // Use HORIZONTAL_RATE to control the speed of uncompression
                    float flow = mass_diff * HORIZONTAL_RATE * 0.5f; 
                    flow = fminf(flow, mass); 

                    new_mass[lr1][lc1] += flow;
                    mass -= flow;
                }
            }

            if (lr2 >= 0 && lr2 < GRID_SIZE && lc2 >= 0 && lc2 < GRID_SIZE) {
                float neighbor_mass = current_mass[lr2][lc2];
                float mass_diff = mass - neighbor_mass;

                if (mass_diff > MIN_MASS && (mass_diff > MIN_FLOW_DIFF || is_gravity_off)) {
                    
                    float flow = mass_diff * HORIZONTAL_RATE * 0.5f; 
                    flow = fminf(flow, mass); 

                    new_mass[lr2][lc2] += flow;
                    mass -= flow;
                }
            }

            new_mass[r][c] += mass;
        }
    }
}

int main() {
  for (int row = 0; row < GRID_SIZE; row++) {
    for (int column = 0; column < GRID_SIZE; column++) {
      if (row >= (int)(GRID_SIZE * .75)) {
        current_mass[row][column] = .75;
      }
      else {
        current_mass[row][column] = 0.0;
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

    // Update FIXED_TIMESTEP times per frame
    for (int i = 0; i < FIXED_TIMESTEP; i ++) {
      memset(new_mass, 0, sizeof(new_mass));
      update_current_mass(gravity_x, gravity_y);
      memcpy(current_mass, new_mass, sizeof(current_mass));
    }


    for (int row = 0; row < GRID_SIZE; row ++) {
      for (int column = 0; column < GRID_SIZE; column ++) {
        if (current_mass[row][column] > RENDER_THRESHOLD) {
            // Draw color (light blue)
            SDL_SetRenderDrawColor(ren, 28, 163, 236, 255); 
            
            SDL_Rect r = { index_to_dim(column), index_to_dim(row), WATER_PIXEL_SIZE, WATER_PIXEL_SIZE };
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
