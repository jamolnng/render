#include <array>
#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <backend/SDL3/render.hpp>
#include <engine/engine.hpp>

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed (%s)", SDL_GetError());
    return 1;
  }

  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;

  auto window_flags = SDL_WINDOW_RESIZABLE
      | SDL_WINDOW_HIGH_PIXEL_DENSITY;  // | SDL_WINDOW_OPENGL;

  if (!SDL_CreateWindowAndRenderer(
          "Render", 640, 480, window_flags, &window, &renderer))
  {
    SDL_Log("SDL_CreateWindowAndRenderer failed (%s)", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  auto surface = SDL_GetWindowSurface(window);

  auto arena = engine::Arena {(char*)malloc(1024 * 1024), 1024 * 1024};
  auto engine = engine::Engine {arena,
                                {static_cast<std::size_t>(surface->w),
                                 static_cast<std::size_t>(surface->h)}};

  while (true) {
    int finished = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        finished = 1;
        break;
      }
    }
    if (finished) {
      break;
    }

    SDL_SetRenderDrawColor(renderer, 31, 31, 31, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    surface = SDL_GetWindowSurface(window);
    engine.begin({static_cast<std::size_t>(surface->w),
                  static_cast<std::size_t>(surface->h)});

    backend::SDL3_Render();

    SDL_RenderPresent(renderer);
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
