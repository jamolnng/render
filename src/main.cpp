#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <engine/command.hpp>
#include <flip/flip.hpp>

using engine::Command;
using engine::CommandType;
using engine::RectCommand;
using engine::TextCommand;

#define BUF_SIZE 1024 * 1024 * sizeof(engine::RectCommand)
char cmdbuf[BUF_SIZE];
std::size_t cmdidx = 0;

void draw_grid(int size_x,
               int size_y,
               int w,
               int h,
               float scale,
               const std::vector<sim::FlipFluid::Color>& colors)
{
  using R = engine::RectCommand;
  float ox = ((float)w / 2.0f - (float)size_x * scale / 2.0f);
  float oy = ((float)h / 2.0f - (float)size_y * scale / 2.0f);
  for (auto i = 0; i < size_x; i++) {
    for (auto j = 0; j < size_y; j++) {
      auto k = size_y - j - 1;
      auto r = colors[i * size_y + k].r;
      auto g = colors[i * size_y + k].g;
      auto b = colors[i * size_y + k].b;
      cmdidx = R::push({(float)i * scale + ox,
                        (float)j * scale + oy,
                        (float)(1.0f) * (scale - 1.0f),
                        (float)(1.0f) * (scale - 1.0f)},
                       {(float)r, (float)g, (float)b, 1},
                       cmdbuf,
                       cmdidx);
    }
  }
}

void set_pixel(
    SDL_Renderer* rend, int x, int y, Uint8 r, Uint8 g, Uint8 b, Uint8 a)
{
  SDL_SetRenderDrawColor(rend, r, g, b, a);
  SDL_RenderPoint(rend, x, y);
}

void draw_circle(SDL_Renderer* surface,
                 int n_cx,
                 int n_cy,
                 int radius,
                 Uint8 r,
                 Uint8 g,
                 Uint8 b,
                 Uint8 a)
{
  // if the first pixel in the screen is represented by (0,0) (which is in sdl)
  // remember that the beginning of the circle is not in the middle of the pixel
  // but to the left-top from it:

  double error = (double)-radius;
  double x = (double)radius - 0.5;
  double y = (double)0.5;
  double cx = n_cx - 0.5;
  double cy = n_cy - 0.5;

  while (x >= y) {
    set_pixel(surface, (int)(cx + x), (int)(cy + y), r, g, b, a);
    set_pixel(surface, (int)(cx + y), (int)(cy + x), r, g, b, a);

    if (x > 0.0f || x < 0.0f) {
      set_pixel(surface, (int)(cx - x), (int)(cy + y), r, g, b, a);
      set_pixel(surface, (int)(cx + y), (int)(cy - x), r, g, b, a);
    }

    if (y > 0.0f || y < 0.0f != 0) {
      set_pixel(surface, (int)(cx + x), (int)(cy - y), r, g, b, a);
      set_pixel(surface, (int)(cx - y), (int)(cy + x), r, g, b, a);
    }

    if ((x > 0.0f || x < 0.0f) != 0 && (y > 0.0f || y < 0.0f != 0)) {
      set_pixel(surface, (int)(cx - x), (int)(cy - y), r, g, b, a);
      set_pixel(surface, (int)(cx - y), (int)(cy - x), r, g, b, a);
    }

    error += y;
    ++y;
    error += y;

    if (error >= 0) {
      --x;
      error -= x;
      error -= x;
    }
    /*
    // sleep for debug
    SDL_RenderPresent(gRenderer);
    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
    */
  }
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed (%s)", SDL_GetError());
    return 1;
  }

  SDL_Window* window = NULL;
  SDL_Renderer* renderer = NULL;

  auto window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
      | SDL_WINDOW_OPENGL | SDL_WINDOW_ALWAYS_ON_TOP;

  if (!SDL_CreateWindowAndRenderer(
          "Flip Fluid Sim", 480, 480, window_flags, &window, &renderer))
  {
    SDL_Log("SDL_CreateWindowAndRenderer failed (%s)", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_SetRenderVSync(renderer, 1);

  auto surface = SDL_GetWindowSurface(window);

  TTF_Init();
  TTF_Font* Sans = TTF_OpenFont("/usr/share/fonts/noto/NotoSans-Bold.ttf", 20);
  TTF_SetFontHinting(Sans, TTF_HINTING_MONO);
  TTF_SetFontWrapAlignment(Sans, TTF_HORIZONTAL_ALIGN_RIGHT);

  std::srand(time(0));

  sim::FlipFluid f {(float)surface->w, (float)surface->h};
  float scale =
      std::min((surface->w - 15.0f) / f.fNumX, (surface->h - 15.0f) / f.fNumY);

  auto newtime = SDL_GetTicks();
  auto oldtime = newtime;
  int p = 0;
  float fps = 100;

  bool move = false;
  bool bordered = true;

  while (true) {
    auto starttime = SDL_GetTicks();
    surface = SDL_GetWindowSurface(window);

    scale = std::min((surface->w - 15.0f) / f.fNumX,
                     (surface->h - 15.0f) / f.fNumY);
    oldtime = newtime;
    int finished = 0;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        finished = 1;
        break;
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        move = true;
        float x, y;
        SDL_GetMouseState(&x, &y);
        x = std::clamp(x, 0.0f, (float)surface->w);
        y = std::clamp(y, 0.0f, (float)surface->h);
        f.setObstacle(x / f.simScale, (surface->h - y) / f.simScale, true);
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        move = false;
        f.scene.obstacleVelX = 0.0f;
        f.scene.obstacleVelY = 0.0f;
      }
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_B) {
          bordered = !bordered;
          SDL_SetWindowBordered(window, bordered);
        }
        if (event.key.key == SDLK_ESCAPE) {
          finished = 1;
          break;
        }
      }
    }
    if (finished) {
      break;
    }
    if (move) {
      float x, y;
      SDL_GetMouseState(&x, &y);
      x = std::clamp(x, 0.0f, (float)surface->w);
      y = std::clamp(y, 0.0f, (float)surface->h);
      f.setObstacle(x / f.simScale, (surface->h - y) / f.simScale, false);
    }

    f.simulate();

    SDL_SetRenderDrawColor(renderer, 31, 31, 31, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    cmdidx = 0;
    draw_grid(
        (int)f.fNumX, (int)f.fNumY, surface->w, surface->h, scale, f.cellColor);

    Command* cmd = (Command*)cmdbuf;
    Command* end = (Command*)&cmdbuf[cmdidx];
    while (cmd != end) {
      switch (cmd->type) {
        case CommandType::Rectangle: {
          RectCommand* rc = (RectCommand*)cmd;
          SDL_SetRenderDrawColorFloat(
              renderer, rc->c.x(), rc->c.y(), rc->c.z(), rc->c.w());
          SDL_FRect sr = {
              rc->bbox.x(), rc->bbox.y(), rc->bbox.z(), rc->bbox.w()};
          SDL_RenderFillRect(renderer, &sr);
        } break;
        case CommandType::Text: {
          TextCommand* tc = (TextCommand*)cmd;
          SDL_Color c = {(uint8_t)tc->c.x(),
                         (uint8_t)tc->c.y(),
                         (uint8_t)tc->c.z(),
                         (uint8_t)tc->c.w()};
          SDL_Surface* surfaceMessage = TTF_RenderText_Shaded(
              Sans, tc->text, tc->nchar, c, {0, 0, 0, 127});
          SDL_Rect Message_rect;
          SDL_GetSurfaceClipRect(surfaceMessage, &Message_rect);
          SDL_FRect mr {tc->bbox.x(),
                        tc->bbox.y(),
                        (float)Message_rect.w,
                        (float)Message_rect.h};
          SDL_Texture* Message =
              SDL_CreateTextureFromSurface(renderer, surfaceMessage);
          SDL_RenderTexture(renderer, Message, NULL, &mr);
          SDL_DestroySurface(surfaceMessage);
          SDL_DestroyTexture(Message);
        } break;
      }
      cmd = (Command*)(((char*)cmd) + cmd->size);
    }

    float xx = f.scene.obstacleX;
    float yy = f.scene.obstacleY;
    float sx = f.simWidth;
    float sy = f.simHeight;

    draw_circle(renderer,
                (xx * surface->w / sx - 1.0f),
                surface->h - (yy * surface->h / sy - 1.0f),
                f.scene.obstacleRadius * f.fInvSpacing * 10,
                255,
                0,
                0,
                255);

    SDL_RenderPresent(renderer);
    auto ur = 70;
    if (p++ >= ur) {
      newtime = SDL_GetTicks();
      p -= ur;
      fps = 1000.0f * (float)ur / (newtime - oldtime);
      std::string newt = std::string("Flip Fluid Sim (")
          + std::to_string((int)fps) + std::string(" fps)");
      SDL_SetWindowTitle(window, newt.c_str());
    }
    constexpr auto fpslimit = 60.0f;
    auto captime = SDL_GetTicks() - starttime;
    if (captime < 1000.0f / fpslimit) {
      SDL_Delay(1000.0f / fpslimit - captime);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
