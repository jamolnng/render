#pragma once

#include <cmath>
#include <vector>

namespace sim
{
class FlipFluid
{
public:
  enum CellType
  {
    FLUID,
    AIR,
    SOLID
  };

  struct Color
  {
    float r, g, b;
  };

  struct Cell
  {
    float u, v, prev_u, prev_v;
    float du, dv;
    float p, s;
    Color color;
    CellType type;
  };

  struct Particle
  {
    float u, v;
    float du, dv;
    float density;
  };

  FlipFluid(float width, float height, float spacing)
  {
    auto sim_h = 3.0;
    auto sim_scale = height / sim_h;
    auto sim_w = width / sim_scale;

    auto res = 100.0f;

    auto tank_w = 1.0f * sim_w;
    auto tank_h = 1.0f * sim_h;
    auto res_h = tank_h / res;

    auto rel_water_h = 0.8;
    auto rel_water_w = 0.6;

    auto r = 0.3 * res_h;
    auto dx = 2.0f * r;
    auto dy = std::sqrt(3.0f) / 2.0f * dx;
    auto nx = std::floor((rel_water_w * tank_w - 2.0 * res_h - 2.0 * r) / dx);
    auto ny = std::floor((rel_water_h * tank_h - 2.0 * res_h - 2.0 * r) / dy);
    auto max_particles = nx * ny;

    //

    _density = 1000.0f;
    int cell_num_x = floor(width / spacing) + 1.0f;
    int cell_num_y = floor(height / spacing) + 1.0f;
    auto h = std::max(width / cell_num_x, height / cell_num_y);
    _inv_spacing = 1.0f / h;

    _cells = {(std::size_t)cell_num_x, {(std::size_t)cell_num_y, Cell {}}};

    _max_particles = 10;

    _particles = {(std::size_t)_max_particles, Particle {}};

    _particle_rest_density = 0.0f;
    _particle_radius = 1.0f;

    _p_inv_spacing = 1.0f / (2.2f * _particle_radius);
    int p_num_x = std::floor(width * _p_inv_spacing) + 1;
    int p_num_y = std::floor(height * _p_inv_spacing) + 1;
    auto p_n_cells = static_cast<int>(p_num_x * p_num_y);

    _n_cell_particles = {p_n_cells, 0};
    _first_cell_particles = {p_n_cells + 1, 0};
    _particle_ids = {_max_particles, 0};

    //

    _n_particles = max_particles;
    auto p = 0;
    for (auto i = 0; i < nx; i++) {
      for (auto j = 0; j < ny; j++) {
        auto& p = _particles[i + j * nx];
        p.u = res_h + r + dx * i + (j % 2 == 0 ? 0.0f : r);
        p.v = res_h + r + dy * j;
      }
    }

    auto n = cell_num_y;
    for (auto i = 0; i < cell_num_x; i++) {
      for (auto j = 0; j < cell_num_y; j++) {
        auto s = 1.0f;  // fluid
        if (i == 0 || i == cell_num_x - 1 || j == 0) {
          s = 0.0f;  // solid
        }
        auto& c = _cells[i][j];
        c.s = s;
      }
    }
  }

private:
  std::vector<std::vector<Cell>> _cells;
  std::vector<Particle> _particles;
  std::vector<int32_t> _n_cell_particles;
  std::vector<int32_t> _first_cell_particles;
  std::vector<int32_t> _particle_ids;

  int32_t _max_particles;
  int32_t _n_particles;
  float _particle_radius;
  float _inv_spacing;
  float _p_inv_spacing;
  float _density;
  float _particle_rest_density;
};
}  // namespace sim