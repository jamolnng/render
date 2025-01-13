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
  float ox = (float)w / 2.0f - (float)size_x * scale / 2.0f;
  float oy = (float)h / 2.0f - (float)size_y * scale / 2.0f;
  for (auto i = 0; i < size_x; i++) {
    for (auto j = 0; j < size_y; j++) {
      // float r = (float)i / size_x;
      // float g = (float)j / size_y;
      // float b = 1.0f;
      auto r = colors[i * size_y + j].r;
      auto g = colors[i * size_y + j].g;
      auto b = colors[i * size_y + j].b;
      cmdidx = R::push({(float)i * scale + ox,
                        (float)j * scale + oy,
                        (float)(1.0f) * scale,
                        (float)(1.0f) * scale},
                       {r, g, b, 1},
                       cmdbuf,
                       cmdidx);
    }
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

  auto window_flags = SDL_WINDOW_RESIZABLE
      | SDL_WINDOW_HIGH_PIXEL_DENSITY;  // | SDL_WINDOW_OPENGL;

  if (!SDL_CreateWindowAndRenderer(
          "Flip Fluid Sim", 640, 640, window_flags, &window, &renderer))
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

  std::size_t gs = 100;
  float scale = (std::min(surface->w, surface->h) / (float)gs);
  auto rr = []() { return (float)std::rand() / RAND_MAX; };

  std::vector<sim::FlipFluid::Color> cs {gs * gs, sim::FlipFluid::Color {}};
  for (auto& c : cs) {
    c.r = rr();
    c.g = rr();
    c.b = rr();
  }

  cmdidx = 0;
  draw_grid(gs, gs, surface->w, surface->h, scale, cs);

  auto newtime = SDL_GetTicks();
  auto oldtime = newtime;
  int p = 0;
  float fps = 100;

  while (true) {
    oldtime = newtime;
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

    cmdidx = 0;
    scale = (std::min(surface->w, surface->h) / (float)gs);
    draw_grid(gs, gs, surface->w, surface->h, scale, cs);
    // auto fps_s = std::to_string((int)fps);
    // int textw, texth;
    // TTF_GetStringSize(Sans, fps_s.c_str(), fps_s.size(), &textw, &texth);
    // cmdidx = TextCommand::push({surface->w - textw, 0.0f},
    //                            0,
    //                            {0, 255, 0, 255},
    //                            fps_s.c_str(),
    //                            strlen(fps_s.c_str()),
    //                            cmdbuf,
    //                            cmdidx);

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

    SDL_RenderPresent(renderer);

    auto ur = 70;
    if (p++ >= ur) {
      p -= ur;
      newtime = SDL_GetTicks();
      fps = 1000.0f * (float)ur / (newtime - oldtime);
      std::string newt = std::string("Flip Fluid Sim (")
          + std::to_string((int)fps) + std::string(" fps)");
      SDL_SetWindowTitle(window, newt.c_str());
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
