#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <stdint.h>
#include <stdlib.h>

#define WIDTH 320
#define HEIGHT 200
#define FIRE_WIDTH WIDTH
#define FIRE_HEIGHT HEIGHT
#define FIRE_PALETTE_SIZE 29

static uint32_t framebuffer[WIDTH * HEIGHT];
static uint8_t fire_pixels[WIDTH * HEIGHT] = {0};
static const uint32_t fire_palette[FIRE_PALETTE_SIZE] = {
    0x070707FF, 0x1f0707FF, 0x2F0F07FF, 0x470F07FF, 0x571707FF,
    0x671F07FF, 0x771F97FF, 0x8F2707FF, 0x9F2F07FF, 0xAF3F07FF,
    0xBF4707FF, 0xC74707FF, 0xDF4F07FF, 0xDF5707FF, 0xCF8717FF,
    0xC78717FF, 0xC78F17FF, 0xC7971FFF, 0xBF9F1FFF, 0xBFA727FF,
    0xBFA727FF, 0xBFAF2FFF, 0xB7AF2FFF, 0xB7B72FFF, 0xB7B737FF,
    0xCFCF6FFF, 0xDFDF9FFF, 0xEFEFC7FF, 0xFFFFFFFF
  };
  
void put_pixel(int x, int y, uint32_t color) {
  framebuffer[(WIDTH * y) + x] = color;
}

void clear_framebuffer(uint32_t color) {
  for (int i = 0; i < WIDTH * HEIGHT; i++) {
    framebuffer[i] = color;
  }
}

int main(void) {

  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Texture *texture;
  SDL_Event event;

  uint8_t is_running = 1;
  uint32_t frame = 0;
  double target_frame = 1.0 / 60.0;

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return 0;
  }

  window = SDL_CreateWindow("SDL Frame Buffer", WIDTH * 4, HEIGHT * 4, 0);
  if (window == NULL) {
    SDL_Quit();
    return 0;
  }

  renderer = SDL_CreateRenderer(window, NULL);
  if (renderer == NULL) {
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
  }

  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32,
                              SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
  if (texture == NULL) {
    SDL_DestroyTexture(texture);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
  }

  SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
  // Set the screen's bottom with the latest fire_palette id
  for (int i = 0; i < FIRE_WIDTH; i++) {
    fire_pixels[(FIRE_HEIGHT - 1) * FIRE_WIDTH + i] = FIRE_PALETTE_SIZE - 1;
  }
  while (is_running) {
    uint64_t start = SDL_GetPerformanceCounter();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        is_running = 0;
      }
    }
    clear_framebuffer(0x00000000);
    // Fire updates
    for (uint32_t x = 0; x < FIRE_WIDTH; x++) {
      for (uint32_t y = 0; y < FIRE_HEIGHT; y++) {
        uint32_t src = FIRE_WIDTH * y + x;
        uint32_t rnd_idx = rand() % 4;
        int32_t dst = (uint32_t)src - FIRE_WIDTH;
        int32_t dst_x = (uint32_t)(src % FIRE_WIDTH) - (uint32_t)rnd_idx + 1;
        if (dst < 0 || dst_x < 0 || dst_x >= FIRE_WIDTH) {
          continue;
        } else {
          dst = dst - (uint32_t)(src % FIRE_WIDTH) + dst_x;
          uint8_t src_val = fire_pixels[src];
          uint8_t decay = rnd_idx & 1;
          fire_pixels[dst] = (src_val > decay) ? (src_val - decay) : 0;
        }
      }
    }
    // Fire render
    for (uint32_t y = 0; y < FIRE_HEIGHT; y++) {
      for (uint32_t x = 0; x < FIRE_WIDTH; x++) {
        uint8_t idx = fire_pixels[y * FIRE_WIDTH + x];
        put_pixel(x, y, fire_palette[idx]);
      }
    }
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));
    SDL_RenderTexture(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    uint64_t end = SDL_GetPerformanceCounter();
    double elapsed =
        (double)(end - start) / (double)SDL_GetPerformanceFrequency();
    if (elapsed <= target_frame) {
      SDL_Delay((target_frame - elapsed) * 1000.0);
    }
    frame++;
  }

  SDL_DestroyWindow(window);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyTexture(texture);
  SDL_Quit();

  return 0;
}
