#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

namespace engine
{
class Arena
{
public:
  Arena(std::byte* base, std::size_t size);
  Arena(Arena&& arena);
  template<typename T, std::size_t S>
  Arena(std::array<T, S>& arr)
      : Arena(arr.data(), sizeof(arr))
  {
  }

  void init();
  void reset();

  template<typename T>
  inline T* aligned_alloc(std::size_t align = alignof(T))
  {
    return aligned_alloc(sizeof(T), align);
  }

  void* aligned_alloc(std::size_t size, std::size_t align);

  template<typename T>
  inline T* malloc()
  {
    return (T*)malloc(sizeof(T));
  }

  void* malloc(std::size_t size);

  template<typename T>
  inline T* calloc()
  {
    return (T*)calloc(sizeof(T));
  }

  void* calloc(std::size_t);

protected:
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

private:
  std::byte* _base;
  std::size_t _size;
  std::size_t _pos;
};
}  // namespace engine