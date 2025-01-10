#include "lib.hpp"

auto main() -> int
{
  auto const lib = library {};

  return lib.name == "render" ? 0 : 1;
}
