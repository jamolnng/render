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

  static std::size_t push(Rect r, Color c, char* buf, std::size_t idx)
  {
    RectCommand* rc = (RectCommand*)&buf[idx];
    rc->type = CommandType::Rectangle;
    rc->size = sizeof(*rc);
    rc->bbox = r;
    rc->c = c;
    return idx + rc->size;
  }
};

struct TextCommand : public Command
{
  int font;
  Color c;
  std::size_t nchar;
  char text[1];

  static std::size_t push(Point p,
                          int font,
                          Color c,
                          char* text,
                          std::size_t nchar,
                          char* buf,
                          std::size_t idx)
  {
    TextCommand* rc = (TextCommand*)&buf[idx];
    rc->type = CommandType::Text;
    rc->size = sizeof(*rc) + nchar;
    rc->bbox = {p.x(), p.y(), 0, 0};  // TODO: generate from text
    rc->c = c;
    rc->font = font;
    rc->nchar = nchar;
    strncpy(rc->text, text, nchar);
    return idx + rc->size;
  }
};
}  // namespace engine