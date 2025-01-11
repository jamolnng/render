#pragma once

#include <cmath>
#include <cstdint>

#include <engine/arena.hpp>
#include <engine/command.hpp>
#include <engine/vec.hpp>

namespace engine
{
using Dimensions = Vec2<std::size_t>;

class Engine
{
public:
  Engine(Arena& arena, Dimensions viewbox);

  void set_viewbox(Dimensions dims);
  void begin(Dimensions dims);
  void begin();
  // CommandArray end();

protected:
  Engine(const Engine&) = delete;
  Engine& operator=(const Engine&) = delete;

private:
  Arena _arena;
  Dimensions _viewbox;
};
}  // namespace engine