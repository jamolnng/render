#pragma once

#include <cstdint>

#include <engine/vec.hpp>

namespace engine
{
enum PointerState
{
  PressedFrame,
  Pressed,
  ReleasedFrame,
  Released
};

struct PointerData
{
  Vec2<std::size_t> pos;
  PointerState state;
};
};