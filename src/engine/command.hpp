#pragma once

#include <cstdint>
#include <cstring>

#include <engine/vec.hpp>

namespace engine
{
using Rect = Vec4<float>;
using Point = Vec2<float>;
using Color = Vec4<float>;

enum CommandType
{
  Rectangle,
  Text
};

static const char* type2str(CommandType type)
{
  switch (type) {
    case Rectangle:
      return "Rectangle";
    case Text:
      return "Text";
  }
  return "UNKNOWN";
}

struct Command
{
  CommandType type;
  uint32_t size;
  Rect bbox;
};

struct RectCommand : public Command
{
  Color c;

  static auto push(Rect r, Color c, char* buf, std::size_t idx) -> std::size_t
  {
    new (&buf[idx]) RectCommand {{.type = CommandType::Rectangle,
                                  .size = sizeof(RectCommand),
                                  .bbox = r},
                                 c};
    return idx + sizeof(RectCommand);
  }
};

struct TextCommand : public Command
{
  int font;
  Color c;
  std::size_t nchar;
  char text[1];

  static auto push(Point p,
                          int font,
                          Color c,
                          const char* text,
                          std::size_t nchar,
                          char* buf,
                          std::size_t idx) -> std::size_t
  {
    auto* rc = new (&buf[idx]) TextCommand {{.type = CommandType::Text,
                                  .size = sizeof(TextCommand),
                                  .bbox = {p.x(), p.y(), 0, 0}},
                                 font,
                                 c,
                                 nchar};
    strncpy(rc->text, text, nchar);
    return idx + sizeof(TextCommand);
  }
};
}  // namespace engine