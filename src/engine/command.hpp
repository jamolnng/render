#pragma once

#include <cstdint>

#include <engine/vec.hpp>

namespace engine
{
enum CommandType
{
  None,
  Rectangle,
  Border,
  Text,
  Image,
};

struct Command
{
  uint32_t id;
  uint32_t size;
  BoundingBox bbox;
  CommandType type;
};

struct RectCommand : public Command
{
  Color color;
};

struct CommandArray
{
  uint32_t capacity;
  uint32_t length;
  Command* commands;
};
}  // namespace engine