#include "arena.hpp"

#include <engine/arena.hpp>

using namespace engine;

Arena::Arena(char* base, std::size_t size)
    : _base(base)
    , _size(size)
{
}

Arena::Arena(Arena&& arena)
    : _base(arena._base)
    , _size(arena._size)
{
  arena._base = nullptr;
  arena._size = 0u;
}

void Arena::init() {}

void Arena::reset()
{
  _pos = 0;
}

void* Arena::alloc(std::size_t size)
{
  return nullptr;
}