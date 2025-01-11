#include <array>
#include <chrono>
#include <iostream>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <engine/command.hpp>

using engine::Command;
using engine::CommandType;
using engine::RectCommand;
using engine::TextCommand;

#define BUF_SIZE 1024 * 1024
char cmdbuf[BUF_SIZE];
std::size_t cmdidx = 0;

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

  SDL_SetRenderVSync(renderer, 1);

  auto surface = SDL_GetWindowSurface(window);

  char str[] = "test";
  char str2[] = "test";

  float aa = surface->w / 2.0f;
  float bb = surface->h / 2.0f;
  cmdidx = RectCommand::push({0, 0, aa, bb}, {1, 0, 0, 1}, cmdbuf, cmdidx);
  cmdidx = RectCommand::push({aa, 0, aa, bb}, {0, 1, 0, 1}, cmdbuf, cmdidx);
  cmdidx = RectCommand::push({0, bb, aa, bb}, {0, 0, 1, 1}, cmdbuf, cmdidx);
  cmdidx = RectCommand::push({aa, bb, aa, bb}, {1, 1, 1, 1}, cmdbuf, cmdidx);
  cmdidx = TextCommand::push(
      {11, 11}, 0, {1, 1, 1, 1}, str, strlen(str), cmdbuf, cmdidx);
  cmdidx = TextCommand::push(
      {11, 11}, 0, {1, 1, 1, 1}, str2, strlen(str2), cmdbuf, cmdidx);

  TTF_Init();
  TTF_Font* Sans =
      TTF_OpenFont("/usr/share/fonts/noto/NotoSans-Regular.ttf", 32);
  TTF_SetFontHinting(Sans, TTF_HINTING_MONO);
  TTF_SetFontWrapAlignment(Sans, TTF_HORIZONTAL_ALIGN_RIGHT);
  std::cout << SDL_GetError() << std::endl;

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

    aa = surface->w / 2.0f;
    bb = surface->h / 2.0f;
    cmdidx = 0;
    cmdidx = RectCommand::push({0, 0, aa, bb}, {1, 0, 0, 1}, cmdbuf, cmdidx);
    cmdidx = RectCommand::push({aa, 0, aa, bb}, {0, 1, 0, 1}, cmdbuf, cmdidx);
    cmdidx = RectCommand::push({0, bb, aa, bb}, {0, 0, 1, 1}, cmdbuf, cmdidx);
    cmdidx = RectCommand::push({aa, bb, aa, bb}, {1, 1, 1, 1}, cmdbuf, cmdidx);
    cmdidx = TextCommand::push({11.0f, 0.0f},
                               0,
                               {255, 255, 255, 255},
                               str,
                               strlen(str),
                               cmdbuf,
                               cmdidx);
    cmdidx = TextCommand::push({aa + 11.0f, 0.0f},
                               0,
                               {255, 255, 255, 255},
                               str,
                               strlen(str),
                               cmdbuf,
                               cmdidx);
    cmdidx = TextCommand::push({11.0f, bb},
                               0,
                               {255, 255, 255, 255},
                               str2,
                               strlen(str2),
                               cmdbuf,
                               cmdidx);
    cmdidx = TextCommand::push({aa + 11.0f, bb},
                               0,
                               {0, 0, 0, 255},
                               str2,
                               strlen(str2),
                               cmdbuf,
                               cmdidx);

    Command* cmd = (Command*)cmdbuf;
    Command* end = (Command*)&cmdbuf[cmdidx];
    while (cmd != end) {
      switch (cmd->type) {
        case CommandType::Rectangle: {
          RectCommand* rc = (RectCommand*)cmd;
          SDL_SetRenderDrawColorFloat(
              renderer, rc->c.x(), rc->c.y(), rc->c.z(), rc->c.w());
          SDL_FRect sr = {(float)rc->bbox.x(),
                          (float)rc->bbox.y(),
                          (float)rc->bbox.z(),
                          (float)rc->bbox.w()};
          SDL_RenderFillRect(renderer, &sr);
        } break;
        case CommandType::Text: {
          TextCommand* tc = (TextCommand*)cmd;
          SDL_Color c = {(uint8_t)tc->c.x(),
                         (uint8_t)tc->c.y(),
                         (uint8_t)tc->c.z(),
                         (uint8_t)tc->c.w()};
          SDL_Surface* surfaceMessage =
              TTF_RenderText_Blended(Sans, tc->text, tc->nchar, c);
          // int bw, bh;
          // TTF_GetStringSize(Sans, tc->text, tc->nchar, &bw, &bh);
          SDL_Rect Message_rect;  // = {tc->bbox.x, tc->bbox.y, bw, bh};
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
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
