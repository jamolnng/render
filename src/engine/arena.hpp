#pragma once

#include <array>
#include <cstdint>

namespace engine
{
class Arena
{
public:
  Arena(char* base, std::size_t size);
  Arena(Arena&& arena);
  template<typename T, std::size_t S>
  Arena(std::array<T, S>& arr)
      : Arena(arr.data(), sizeof(arr))
  {
  }

  void init();
  void reset();

  template<typename T>
  T* alloc()
  {
    return (T*)alloc(sizeof(T));
  }

  void* alloc(std::size_t size);

  template<typename T>
  void free(T* t)
  {
  }

protected:
  Arena(const Arena&) = delete;
  Arena& operator=(const Arena&) = delete;

private:
  char* _base;
  std::size_t _size;
  std::size_t _pos;
};
}  // namespace engine