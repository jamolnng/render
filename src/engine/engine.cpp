#include <utility>

#include <engine/engine.hpp>

using namespace engine;

Engine::Engine(Arena& arena, Dimensions viewbox)
    : _arena(std::move(arena))
    , _viewbox(viewbox)

{
}

void Engine::set_viewbox(Dimensions viewbox)
{
  _viewbox = viewbox;
}

void Engine::begin(Dimensions viewbox)
{
  set_viewbox(viewbox);
  begin();
}

void Engine::begin() {}