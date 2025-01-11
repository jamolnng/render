#include <cstring>

#include "arena.hpp"

#include <engine/arena.hpp>

using namespace engine;

Arena::Arena(std::byte* base, std::size_t size)
    : _base(base)
    , _size(size)
    , _pos(0)
{
}

Arena::Arena(Arena&& arena)
    : _base(arena._base)
    , _size(arena._size)
    , _pos(arena._pos)
{
  arena._base = nullptr;
  arena._size = 0u;
}

void Arena::init() {}

void Arena::reset()
{
  _pos = 0;
}

void* Arena::aligned_alloc(std::ptrdiff_t size, std::ptrdiff_t align)
{
  if (size == 0) {
    return nullptr;
  }
  auto beg = _base + _pos;
  auto end = _base + _size;
  auto padding = -(uintptr_t)beg & (align - 1);
  auto avail = end - beg - padding;
  if (avail < 0) {
    // not enough memory
    return nullptr;
  }
  void* p = beg + padding;
  _pos += padding + size;
  return p;
}

void* Arena::malloc(std::ptrdiff_t size)
{
  return aligned_alloc(size, alignof(std::max_align_t));
}

void* Arena::calloc(std::ptrdiff_t size)
{
  auto* p = Arena::malloc(size);
  if (p != nullptr) {
    std::memset(p, 0, size);
  }
  return p;
}