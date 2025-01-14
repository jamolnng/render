#include <algorithm>
#include <array>
#include <bit>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>

// sdl headers
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>

// engine header
#include <engine/command.hpp>
#include <flip/flip.hpp>

using engine::Command;
using engine::CommandType;
using engine::RectCommand;
// using engine::TextCommand;

#define BUF_SIZE (1024UL * 1024UL * sizeof(engine::RectCommand))
static std::array<char, BUF_SIZE> cmdbuf {};
static std::size_t cmdidx = 0;

namespace
{
void draw_grid(unsigned int size_x,
               unsigned int size_y,
               unsigned int width,
               unsigned int height,
               float scale,
               const std::vector<sim::FlipFluid::Color>& colors)
{
  const float offsetx = ((static_cast<float>(width) / 2.0F)
                         - (static_cast<float>(size_x) * scale / 2.0F));
  const float offsety = ((static_cast<float>(height) / 2.0F)
                         - (static_cast<float>(size_y) * scale / 2.0F));
  for (auto i = 0UL; i < size_x; i++) {
    for (auto j = 0UL; j < size_y; j++) {
      const auto fixedk = size_y - j - 1;
      const auto fixedidx = static_cast<std::size_t>((i * size_y) + fixedk);
      const auto cred = colors[fixedidx].r;
      const auto cgreen = colors[fixedidx].g;
      const auto cblue = colors[fixedidx].b;
      cmdidx =
          engine::RectCommand::push({(static_cast<float>(i) * scale) + offsetx,
                                     (static_cast<float>(j) * scale) + offsety,
                                     (1.0F) * (scale - 1.0F),
                                     (1.0F) * (scale - 1.0F)},
                                    {static_cast<float>(cred),
                                     static_cast<float>(cgreen),
                                     static_cast<float>(cblue),
                                     1},
                                    cmdbuf.data(),
                                    cmdidx);
    }
  }
}

void set_pixel(SDL_Renderer* rend,
               int posx,
               int posy,
               Uint8 cred,
               Uint8 cgreen,
               Uint8 cblue,
               Uint8 calpha)
{
  SDL_SetRenderDrawColor(rend, cred, cgreen, cblue, calpha);
  SDL_RenderPoint(rend, static_cast<float>(posx), static_cast<float>(posy));
}

void draw_circle(SDL_Renderer* surface,
                 int n_cx,
                 int n_cy,
                 int radius,
                 Uint8 cred,
                 Uint8 cgreen,
                 Uint8 cblue,
                 Uint8 calpha)
{
  // if the first pixel in the screen is represented by (0,0) (which is in sdl)
  // remember that the beginning of the circle is not in the middle of the pixel
  // but to the left-top from it:

  auto error = static_cast<double>(-radius);
  constexpr auto pixel_offset = 0.5;
  double posx = static_cast<double>(radius) - pixel_offset;
  double posy = pixel_offset;
  const double centerx = n_cx - pixel_offset;
  const double centery = n_cy - pixel_offset;

  while (posx >= posy) {
    set_pixel(surface,
              static_cast<int>(centerx + posx),
              static_cast<int>(centery + posy),
              cred,
              cgreen,
              cblue,
              calpha);
    set_pixel(surface,
              static_cast<int>(centerx + posy),
              static_cast<int>(centery + posx),
              cred,
              cgreen,
              cblue,
              calpha);

    if (posx > 0.0 || posx < 0.0) {
      set_pixel(surface,
                static_cast<int>(centerx - posx),
                static_cast<int>(centery + posy),
                cred,
                cgreen,
                cblue,
                calpha);
      set_pixel(surface,
                static_cast<int>(centerx + posy),
                static_cast<int>(centery - posx),
                cred,
                cgreen,
                cblue,
                calpha);
    }

    if (posy > 0.0 || posy < 0.0) {
      set_pixel(surface,
                static_cast<int>(centerx + posx),
                static_cast<int>(centery - posy),
                cred,
                cgreen,
                cblue,
                calpha);
      set_pixel(surface,
                static_cast<int>(centerx - posy),
                static_cast<int>(centery + posx),
                cred,
                cgreen,
                cblue,
                calpha);
    }

    if ((posx > 0.0 || posx < 0.0) && (posy > 0.0 || posy < 0.0)) {
      set_pixel(surface,
                static_cast<int>(centerx - posx),
                static_cast<int>(centery - posy),
                cred,
                cgreen,
                cblue,
                calpha);
      set_pixel(surface,
                static_cast<int>(centerx - posy),
                static_cast<int>(centery - posx),
                cred,
                cgreen,
                cblue,
                calpha);
    }

    error += posy;
    ++posy;
    error += posy;

    if (error >= 0) {
      --posx;
      error -= posx;
      error -= posx;
    }
    /*
    // sleep for debug
    SDL_RenderPresent(gRenderer);
    std::this_thread::sleep_for(std::chrono::milliseconds{ 100 });
    */
  }
}
}  // namespace

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int
{
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("SDL_Init failed (%s)", SDL_GetError());
    return 1;
  }

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;

  auto window_flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL
      | SDL_WINDOW_ALWAYS_ON_TOP;

  constexpr auto window_size = 480;
  if (!SDL_CreateWindowAndRenderer("Flip Fluid Sim",
                                   window_size,
                                   window_size,
                                   window_flags,
                                   &window,
                                   &renderer))
  {
    SDL_Log("SDL_CreateWindowAndRenderer failed (%s)", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_SetRenderVSync(renderer, 1);

  auto* surface = SDL_GetWindowSurface(window);

  // TTF_Init();
  // TTF_Font* Sans = TTF_OpenFont("/usr/share/fonts/noto/NotoSans-Bold.ttf",
  // 20); TTF_SetFontHinting(Sans, TTF_HINTING_MONO);
  // TTF_SetFontWrapAlignment(Sans, TTF_HORIZONTAL_ALIGN_RIGHT);

  sim::FlipFluid flip {static_cast<double>(surface->w),
                       static_cast<double>(surface->h)};

  auto newtime = SDL_GetTicks();
  decltype(newtime) oldtime {};
  int framecount = 0;
  float fps {};

  bool move = false;
  bool bordered = true;

  while (true) {
    auto starttime = SDL_GetTicks();
    surface = SDL_GetWindowSurface(window);

    constexpr auto padding = 15.0;
    auto scale =
        std::min((static_cast<double>(surface->w) - padding) / flip.fNumX,
                 (static_cast<double>(surface->h) - padding) / flip.fNumY);
    oldtime = newtime;
    bool finished = false;
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        finished = true;
        break;
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        move = true;
        float mposx {0.0F};
        float mposy {0.0F};
        SDL_GetMouseState(&mposx, &mposy);
        mposx = std::clamp(mposx, 0.0F, static_cast<float>(surface->w));
        mposy = std::clamp(mposy, 0.0F, static_cast<float>(surface->h));
        flip.setObstacle(
            static_cast<double>(mposx) / flip.simScale,
            (surface->h - static_cast<double>(mposy)) / flip.simScale,
            /*reset=*/true);
      }
      if (event.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        move = false;
        flip.scene.obstacleVelX = 0.0;
        flip.scene.obstacleVelY = 0.0;
      }
      if (event.type == SDL_EVENT_KEY_DOWN) {
        if (event.key.key == SDLK_B) {
          bordered = !bordered;
          SDL_SetWindowBordered(window, bordered);
        }
        if (event.key.key == SDLK_ESCAPE) {
          finished = true;
          break;
        }
      }
    }
    if (finished) {
      break;
    }
    if (move) {
      float mposx {0.0F};
      float mposy {0.0F};
      SDL_GetMouseState(&mposx, &mposy);
      mposx = std::clamp(mposx, 0.0F, static_cast<float>(surface->w));
      mposy = std::clamp(mposy, 0.0F, static_cast<float>(surface->h));
      flip.setObstacle(
          static_cast<double>(mposx) / flip.simScale,
          (surface->h - static_cast<double>(mposy)) / flip.simScale,
          /*reset=*/false);
    }

    flip.simulate();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);

    cmdidx = 0;
    draw_grid(static_cast<unsigned int>(flip.fNumX),
              static_cast<unsigned int>(flip.fNumY),
              static_cast<unsigned int>(surface->w),
              static_cast<unsigned int>(surface->h),
              static_cast<float>(scale),
              flip.cellColor);

    auto* cmd = std::bit_cast<Command*>(cmdbuf.data());
    auto* end = std::bit_cast<Command*>(&cmdbuf.at(cmdidx));
    while (cmd != end) {
      switch (cmd->type) {
        case CommandType::Rectangle: {
          auto* rectcmd = std::bit_cast<RectCommand*>(cmd);
          SDL_SetRenderDrawColorFloat(renderer,
                                      rectcmd->c.x(),
                                      rectcmd->c.y(),
                                      rectcmd->c.z(),
                                      rectcmd->c.w());
          const SDL_FRect sdl_bbox = {rectcmd->bbox.x(),
                                      rectcmd->bbox.y(),
                                      rectcmd->bbox.z(),
                                      rectcmd->bbox.w()};
          SDL_RenderFillRect(renderer, &sdl_bbox);
        } break;
        case CommandType::Text: {
          // TextCommand* tc = (TextCommand*)cmd;
          // SDL_Color c = {(uint8_t)tc->c.x(),
          //                (uint8_t)tc->c.y(),
          //                (uint8_t)tc->c.z(),
          //                (uint8_t)tc->c.w()};
          // SDL_Surface* surfaceMessage = TTF_RenderText_Shaded(
          //     Sans, tc->text, tc->nchar, c, {0, 0, 0, 127});
          // SDL_Rect Message_rect;
          // SDL_GetSurfaceClipRect(surfaceMessage, &Message_rect);
          // SDL_FRect mr {tc->bbox.x(),
          //               tc->bbox.y(),
          //               static_cast<float>(Message_rect.w),
          //               static_cast<float>(Message_rect.h)};
          // SDL_Texture* Message =
          //     SDL_CreateTextureFromSurface(renderer, surfaceMessage);
          // SDL_RenderTexture(renderer, Message, NULL, &mr);
          // SDL_DestroySurface(surfaceMessage);
          // SDL_DestroyTexture(Message);
        } break;
      }
      cmd = std::bit_cast<Command*>(std::bit_cast<char*>(cmd) + cmd->size);
    }

    const double oxx = flip.scene.obstacleX;
    const double oyy = flip.scene.obstacleY;
    const double ssx = flip.simWidth;
    const double ssy = flip.simHeight;

    const auto circle_scale = 10;

    draw_circle(renderer,
                static_cast<int>((oxx * surface->w / ssx) - 1.0),
                static_cast<int>(surface->h - ((oyy * surface->h / ssy) - 1.0)),
                static_cast<int>(flip.scene.obstacleRadius * flip.fInvSpacing
                                 * circle_scale),
                SDL_ALPHA_OPAQUE,
                0,
                0,
                SDL_ALPHA_OPAQUE);

    SDL_RenderPresent(renderer);
    constexpr auto delay_frames = 70;
    constexpr auto ms_per_s = 1000.0F;
    if (framecount++ >= delay_frames) {
      newtime = SDL_GetTicks();
      framecount -= delay_frames;
      fps = ms_per_s * static_cast<float>(delay_frames)
          / static_cast<float>(newtime - oldtime);
      const std::string newt = std::string("Flip Fluid Sim (")
          + std::to_string(static_cast<int>(fps)) + std::string(" fps)");
      SDL_SetWindowTitle(window, newt.c_str());
    }
    constexpr auto fpslimit = 60.0F;
    auto captime = SDL_GetTicks() - starttime;
    if (captime < static_cast<int>(ms_per_s / fpslimit)) {
      SDL_Delay(static_cast<int>(ms_per_s / fpslimit)
                - static_cast<Uint32>(captime));
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
  return 0;
}
