#pragma once

#include <cfloat>
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
    double r, g, b;
  };

  struct Cell
  {
    double u, v, prev_u, prev_v;
    double du, dv;
    double p, s;
    Color color;
    CellType type;
  };

  struct Particle
  {
    double u, v;
    double du, dv;
    double density;
  };

  struct Scene
  {
    double gravity {-9.81};
    double dt {1.0 / 120.0};
    double flipRatio {0.9f};
    int numPressureIters {100};
    int numParticleIters {2};
    int frameNr {0};
    double overRelaxation {1.9};
    bool compensateDraft {true};
    bool separateParticles {true};
    double obstacleX {0.0};
    double obstacleY {0.0};
    double obstacleRadius {0.15f};
    bool paused {true};
    bool showObstacle {true};
    double obstacleVelX {0.0};
    double obstacleVelY {0.0};
    bool showParticles {true};
    bool showGrid {false};
  };

  void init_fluid(double density,
                  double width,
                  double height,
                  double spacing,
                  double particleRadius,
                  double maxParticles)
  {
    // fluid

    this->density = density;
    this->fNumX = std::floor(width / spacing) + 1.0;
    this->fNumY = std::floor(height / spacing) + 1.0;
    this->h = std::max(width / this->fNumX, height / this->fNumY);
    this->fInvSpacing = 1.0 / this->h;
    this->fNumCells = this->fNumX * this->fNumY;

    this->u = std::vector<double>(this->fNumCells, 0.0);
    this->v = std::vector<double>(this->fNumCells, 0.0);
    this->du = std::vector<double>(this->fNumCells, 0.0);
    this->dv = std::vector<double>(this->fNumCells, 0.0);
    this->prevU = std::vector<double>(this->fNumCells, 0.0);
    this->prevV = std::vector<double>(this->fNumCells, 0.0);
    this->p = std::vector<double>(this->fNumCells, 0.0);
    this->s = std::vector<double>(this->fNumCells, 0.0);
    this->cellType = std::vector<CellType>(this->fNumCells, CellType::FLUID);
    this->cellColor = std::vector<Color>(this->fNumCells, Color {});

    // paraticles

    this->maxParticles = maxParticles;

    this->particlePos = std::vector<double>(2 * maxParticles, 0.0);
    this->particleColor = std::vector<Color>(maxParticles, Color {0, 0, 1.0});

    this->particleVel = std::vector<double>(2 * maxParticles, 0.0);
    this->particleDensity = std::vector<double>(fNumCells, 0.0);
    this->particleRestDensity = 0.0;

    this->particleRadius = particleRadius;
    this->pInvSpacing = 1.0 / (2.2 * particleRadius);
    this->pNumX = std::floor(width * pInvSpacing) + 1.0;
    this->pNumY = std::floor(height * pInvSpacing) + 1.0;
    this->pNumCells = pNumX * pNumY;

    this->numCellParticles = std::vector<int>(pNumCells, 0);
    this->firstCellParticle = std::vector<int>(pNumCells + 1, 0);
    this->cellParticleIds = std::vector<int>(maxParticles);

    this->numParticles = 0;
  }

  void setObstacle(double x, double y, bool reset)
  {
    double vx = 0.0;
    double vy = 0.0;

    if (!reset) {
      vx = (x - scene.obstacleX) / scene.dt;
      vy = (y - scene.obstacleY) / scene.dt;
    }

    scene.obstacleX = x;
    scene.obstacleY = y;

    const double r = scene.obstacleRadius;
    const int n = fNumY;
    const double cd = std::sqrt(2.0) * this->h;

    for (auto i = 1u; i < fNumX - 2; i++) {
      for (auto j = 1u; j < fNumY - 2; j++) {
        this->s[i * n + j] = 1.0;
        double dx = (i + 0.5f) * this->h - x;
        double dy = (j + 0.5f) * this->h - y;

        if (dx * dx + dy * dy < r * r) {
          this->s[i * n + j] = 0.0;
          this->u[i * n + j] = vx;
          this->u[(i + 1) * n + j] = vx;
          this->v[i * n + j] = vy;
          this->v[i * n + j + 1] = vy;
        }
      }
    }

    scene.showObstacle = true;
    scene.obstacleVelX = vx;
    scene.obstacleVelY = vy;
  }

  void integrateParticles(double dt, double gravity)
  {
    for (auto i = 0UL; i < numParticles; i++) {
      particleVel[(2 * i) + 1] += dt * gravity;
      particlePos[(2 * i)] += particleVel[2 * i] * dt;
      particlePos[(2 * i) + 1] += particleVel[(2 * i) + 1] * dt;
    }
  }

  void pushParticlesApart(int numIters)
  {
    double colorDiffusionCoeff = 0.001f;

    std::fill(numCellParticles.begin(), numCellParticles.end(), 0);

    for (auto i = 0; i < numParticles; i++) {
      double x = particlePos[(2 * i)];
      double y = particlePos[(2 * i) + 1];

      double xi = std::clamp(std::floor(x * pInvSpacing), 0.0, pNumX - 1.0);
      double yi = std::clamp(std::floor(y * pInvSpacing), 0.0, pNumY - 1.0);
      double cellNr = xi * pNumY + yi;
      numCellParticles[(int)cellNr]++;
    }

    int first = 0;

    for (auto i = 0; i < pNumCells; i++) {
      first += numCellParticles[i];
      firstCellParticle[i] = first;
    }
    firstCellParticle[(std::size_t)pNumCells] = first;

    for (auto i = 0; i < numParticles; i++) {
      double x = particlePos[(2 * i)];
      double y = particlePos[(2 * i) + 1];

      double xi = std::clamp(std::floor(x * pInvSpacing), 0.0, pNumX - 1.0);
      double yi = std::clamp(std::floor(y * pInvSpacing), 0.0, pNumY - 1.0);
      double cellNr = xi * pNumY + yi;
      firstCellParticle[(int)cellNr]--;
      cellParticleIds[firstCellParticle[(int)cellNr]] = i;
    }

    double minDist = 2.0 * particleRadius;
    double minDist2 = minDist * minDist;

    for (auto iter = 0; iter < numIters; iter++) {
      for (auto i = 0; i < numParticles; i++) {
        double px = particlePos[(2 * i)];
        double py = particlePos[(2 * i) + 1];

        double pxi = std::floor(px * pInvSpacing);
        double pyi = std::floor(py * pInvSpacing);
        double x0 = std::max(pxi - 1.0, 0.0);
        double y0 = std::max(pyi - 1.0, 0.0);
        double x1 = std::min(pxi + 1.0, pNumX - 1.0);
        double y1 = std::min(pyi + 1.0, pNumY - 1.0);

        for (auto xi = x0; xi <= x1; xi++) {
          for (auto yi = y0; yi <= y1; yi++) {
            double cellNr = xi * pNumY + yi;
            int first = firstCellParticle[(int)cellNr];
            int last = firstCellParticle[(int)cellNr + 1];
            for (auto j = first; j < last; j++) {
              int id = cellParticleIds[j];
              if (id == i) {
                continue;
              }
              double qx = particlePos[(2 * id)];
              double qy = particlePos[(2 * id) + 1];

              double dx = qx - px;
              double dy = qy - py;
              double d2 = dx * dx + dy * dy;
              if (d2 > minDist2 || std::abs(d2) < DBL_MIN) {
                continue;
              }
              double d = std::sqrt(d2);
              double s = 0.5 * (minDist - d) / d;
              dx *= s;
              dy *= s;
              particlePos[(2 * i)] -= dx;
              particlePos[(2 * i) + 1] -= dy;
              particlePos[(2 * id)] += dx;
              particlePos[(2 * id) + 1] += dy;

              // diffuse colors
              // for (var k = 0; k < 3; k++) {
              //   var color0 = this.particleColor[3 * i + k];
              //   var color1 = this.particleColor[3 * id + k];
              //   var color = (color0 + color1) * 0.5;
              //   this.particleColor[3 * i + k] =
              //       color0 + (color - color0) * colorDiffusionCoeff;
              //   this.particleColor[3 * id + k] =
              //       color1 + (color - color1) * colorDiffusionCoeff;
              // }
            }
          }
        }
      }
    }
  }

  void handleParticleCollisions(double obstacleX,
                                double obstacleY,
                                double obstacleRadius)
  {
    double h = 1.0 / fInvSpacing;
    double r = particleRadius;
    double orr = obstacleRadius;
    double or2 = orr * orr;
    double minDist = obstacleRadius + r;
    double minDist2 = minDist * minDist;

    double minX = h + r;
    double maxX = (this->fNumX - 1) * h - r;
    double minY = h + r;
    double maxY = (this->fNumY - 1) * h - r;

    for (auto i = 0; i < numParticles; i++) {
      double x = particlePos[(2 * i)];
      double y = particlePos[(2 * i) + 1];

      double dx = x - obstacleX;
      double dy = y - obstacleY;
      double d2 = dx * dx + dy * dy;

      // obstacle collision
      if (d2 < minDist2) {
        this->particleVel[(2 * i)] = scene.obstacleVelX;
        this->particleVel[(2 * i) + 1] = scene.obstacleVelY;
      }

      // wall collision
      if (x < minX) {
        x = minX;
        particleVel[2 * i] = 0.0;
      }
      if (x > maxX) {
        x = maxX;
        particleVel[2 * i] = 0.0;
      }
      if (y < minY) {
        y = minY;
        particleVel[(2 * i) + 1] = 0.0;
      }
      if (y > maxY) {
        y = maxY;
        particleVel[(2 * i) + 1] = 0.0;
      }

      particlePos[(2 * i)] = x;
      particlePos[(2 * i) + 1] = y;
    }
  }

  void updateParticleDensity()
  {
    double n = fNumY;
    double h = this->h;
    double h1 = fInvSpacing;
    double h2 = 0.5 * h;

    std::fill(particleDensity.begin(), particleDensity.end(), 0.0);

    for (auto i = 0; i < numParticles; i++) {
      double x = particlePos[(2 * i)];
      double y = particlePos[(2 * i) + 1];

      x = std::clamp(x, h, (this->fNumX - 1.0) * h);
      y = std::clamp(y, h, (this->fNumY - 1.0) * h);

      double x0 = std::floor((x - h2) * h1);
      double tx = ((x - h2) - x0 * h) * h1;
      double x1 = std::min(x0 + 1.0, fNumX - 2.0);

      double y0 = std::floor((y - h2) * h1);
      double ty = ((y - h2) - y0 * h) * h1;
      double y1 = std::min(y0 + 1.0, fNumY - 2.0);

      double sx = 1.0 - tx;
      double sy = 1.0 - ty;

      if (x0 < fNumX && y0 < fNumY) {
        particleDensity[(int)((x0 * n) + y0)] += (sx * sy);
      }
      if (x1 < fNumX && y0 < fNumY) {
        particleDensity[(int)((x1 * n) + y0)] += (tx * sy);
      }
      if (x1 < fNumX && y1 < fNumY) {
        particleDensity[(int)((x1 * n) + y1)] += (tx * ty);
      }
      if (x0 < fNumX && y1 < fNumY) {
        particleDensity[(int)((x0 * n) + y1)] += (sx * ty);
      }
    }

    if (std::abs(particleRestDensity) < DBL_MIN) {
      double sum = 0.0;
      double numFluidCells = 0.0;

      for (auto i = 0u; i < fNumCells; i++) {
        if (cellType[i] == CellType::FLUID) {
          sum += particleDensity[i];
          numFluidCells++;
        }
        if (numFluidCells > 0) {
          particleRestDensity = sum / numFluidCells;
        }
      }
    }
  }

  void transferVelocities(bool toGrid, double flipRatio)
  {
    double n = fNumY;
    double h = this->h;
    double h1 = fInvSpacing;
    double h2 = 0.5 * h;

    if (toGrid) {
      prevU = u;
      prevV = v;

      std::fill(du.begin(), du.end(), 0.0);
      std::fill(dv.begin(), dv.end(), 0.0);
      std::fill(u.begin(), u.end(), 0.0);
      std::fill(v.begin(), v.end(), 0.0);

      for (auto i = 0; i < fNumCells; i++) {
        cellType[i] =
            std::abs(this->s[i]) < DBL_MIN ? CellType::SOLID : CellType::AIR;
      }

      for (auto i = 0; i < numParticles; i++) {
        double x = particlePos[(2 * i)];
        double y = particlePos[(2 * i) + 1];
        double xi = std::clamp(std::floor(x * h1), 0.0, fNumX - 1.0);
        double yi = std::clamp(std::floor(y * h1), 0.0, fNumY - 1.0);
        int cellNr = (int)(xi * n + yi);
        if (cellType[cellNr] == CellType::AIR) {
          cellType[cellNr] = CellType::FLUID;
        }
      }
    }

    for (auto component = 0; component < 2; component++) {
      double dx = component == 0 ? 0.0 : h2;
      double dy = component == 0 ? h2 : 0.0;

      auto& f = component == 0 ? u : v;
      auto& prevF = component == 0 ? prevU : prevV;
      auto& d = component == 0 ? du : dv;

      for (auto i = 0; i < numParticles; i++) {
        double x = particlePos[(2 * i)];
        double y = particlePos[(2 * i) + 1];
        x = std::clamp(x, h, (fNumX - 1.0) * h);
        y = std::clamp(y, h, (fNumY - 1.0) * h);

        double x0 = std::min(std::floor((x - dx) * h1), fNumX - 2.0);
        double tx = ((x - dx) - x0 * h) * h1;
        double x1 = std::min(x0 + 1.0, fNumX - 2.0);

        double y0 = std::min(std::floor((y - dy) * h1), fNumY - 2.0);
        double ty = ((y - dy) - y0 * h) * h1;
        double y1 = std::min(y0 + 1.0, fNumY - 2.0);

        double sx = 1.0 - tx;
        double sy = 1.0 - ty;

        double d0 = sx * sy;
        double d1 = tx * sy;
        double d2 = tx * ty;
        double d3 = sx * ty;

        double nr0 = x0 * n + y0;
        double nr1 = x1 * n + y0;
        double nr2 = x1 * n + y1;
        double nr3 = x0 * n + y1;

        if (toGrid) {
          double pv = particleVel[(2 * i) + component];
          f[(int)nr0] += pv * d0;
          d[(int)nr0] += d0;

          f[(int)nr1] += pv * d1;
          d[(int)nr1] += d1;

          f[(int)nr2] += pv * d2;
          d[(int)nr2] += d2;

          f[(int)nr3] += pv * d3;
          d[(int)nr3] += d3;
        } else {
          int offset = component == 0 ? n : 1;
          auto vcell = [this, offset](int idx)
          {
            return (cellType[idx] != CellType::AIR
                    || cellType[idx - offset] != CellType::AIR)
                ? 1.0
                : 0.0;
          };
          double valid0 = vcell((int)nr0);
          double valid1 = vcell((int)nr1);
          double valid2 = vcell((int)nr2);
          double valid3 = vcell((int)nr3);

          double v = particleVel[(2 * i) + component];
          double d = valid0 * d0 + valid1 * d1 + valid2 * d2 + valid3 * d3;

          if (d > 0.0) {
            double picV =
                (valid0 * d0 * f[(int)nr0] + valid1 * d1 * f[(int)nr1]
                 + valid2 * d2 * f[(int)nr2] + valid3 * d3 * f[(int)nr3])
                / d;

            double corr = (valid0 * d0 * (f[(int)nr0] - prevF[(int)nr0])
                           + valid1 * d1 * (f[(int)nr1] - prevF[(int)nr1])
                           + valid2 * d2 * (f[(int)nr2] - prevF[(int)nr2])
                           + valid3 * d3 * (f[(int)nr3] - prevF[(int)nr3]))
                / d;
            double flipV = v + corr;

            particleVel[(2 * i) + component] =
                (1.0 - flipRatio) * picV + flipRatio * flipV;
          }
        }
      }
      if (toGrid) {
        for (auto i = 0; i < f.size(); i++) {
          if (d[i] > 0.0) {
            f[i] /= d[i];
          }
        }

        // restore solid cells

        for (auto i = 0; i < fNumX; i++) {
          for (auto j = 0; j < fNumY; j++) {
            bool solid = cellType[i * n + j] == CellType::SOLID;
            if (solid
                || (i > 0 && cellType[(i - 1) * n + j] == CellType::SOLID))
            {
              u[i * n + j] = prevU[i * n + j];
            }
            if (solid || (j > 0 && cellType[i * n + j - 1] == CellType::SOLID))
            {
              v[i * n + j] = prevV[i * n + j];
            }
          }
        }
      }
    }
  }

  void solveIncompressibility(int numIters,
                              double dt,
                              double overRelaxation,
                              bool compensateDrift = true)
  {
    std::fill(p.begin(), p.end(), 0.0);
    prevU = u;
    prevV = v;

    double n = fNumY;
    double cp = density * h / dt;

    for (auto i = 0; i < fNumCells; i++) {
      double u = this->u[i];
      double v = this->v[i];
    }

    for (auto iter = 0; iter < numIters; iter++) {
      for (auto i = 1; i < fNumX - 1.0; i++) {
        for (auto j = 1; j < fNumY - 1.0; j++) {
          if (cellType[(i * n) + j] != CellType::FLUID) {
            continue;
          }

          auto center = i * n + j;
          auto left = (i - 1) * n + j;
          auto right = (i + 1) * n + j;
          auto bottom = i * n + j - 1;
          auto top = i * n + j + 1;

          // auto s = this.s[center];
          auto sx0 = this->s[left];
          auto sx1 = this->s[right];
          auto sy0 = this->s[bottom];
          auto sy1 = this->s[top];
          auto s = sx0 + sx1 + sy0 + sy1;
          if (std::abs(s) < DBL_MIN) {
            continue;
          }

          auto div =
              this->u[right] - this->u[center] + this->v[top] - this->v[center];

          if (particleRestDensity > 0.0 && compensateDrift) {
            double k = 1.0;
            double compression =
                particleDensity[i * n + j] - particleRestDensity;
            if (compression > 0.0) {
              div = div - k * compression;
            }

            double p = -div / s;
            p *= overRelaxation;
            this->p[center] += cp * p;
            
            this->u[center] -= sx0 * p;
            this->u[right] += sx1 * p;
            this->v[center] -= sy0 * p;
            this->v[top] += sy1 * p;
          }
        }
      }
    }
  }

  void setSciColor(int cellNr, double val, double minVal, double maxVal)
  {
    val = std::min(std::max(val, minVal), maxVal - 0.0001f);
    double d = maxVal - minVal;
    val = std::abs(d) < 0.000001 ? 0.5 : (val - minVal) / d;
    double m = 0.25;
    double num = std::floor(val / m);
    double s = (val - num * m) / m;
    double r, g, b;

    switch ((int)num) {
      case 0:
        r = 0.0;
        g = s;
        b = 1.0;
        break;
      case 1:
        r = 0.0;
        g = 1.0;
        b = 1.0 - s;
        break;
      case 2:
        r = s;
        g = 1.0;
        b = 0.0;
        break;
      case 3:
        r = 1.0;
        g = 1.0 - s;
        b = 0.0;
        break;
    }

    cellColor[cellNr] = {r, g, b};
  }

  void updateCellColors()
  {
    std::fill(cellColor.begin(), cellColor.end(), Color {0.0, 0.0, 0.0});

    for (auto i = 0; i < fNumCells; i++) {
      if (cellType[i] == CellType::SOLID) {
        cellColor[i] = {0.5, 0.5, 0.5};
      } else if (cellType[i] == CellType::FLUID) {
        // cellColor[i] = {0.0, 0.0, 1.0};
        double d = particleDensity[i];
        if (particleRestDensity > 0.0) {
          d /= particleRestDensity;
        }
        setSciColor(i, d, 0, 2.0);
      }
    }
  }

  void simulate(double dt,
                double gravity,
                double flipRatio,
                int numPressureIters,
                int numParticleIters,
                double overRelaxation,
                bool compensateDrift,
                bool separateParticles,
                double obstacleX,
                double obstacleY,
                double obstacleRadius)
  {
    int numSubSteps = 1;
    double sdt = dt / (double)numSubSteps;

    for (auto step = 0; step < numSubSteps; step++) {
      integrateParticles(sdt, gravity);
      if (separateParticles) {
        pushParticlesApart(numParticleIters);
      }
      handleParticleCollisions(obstacleX, obstacleY, obstacleRadius);
      transferVelocities(true, 0.0);
      updateParticleDensity();
      solveIncompressibility(
          numPressureIters, sdt, overRelaxation, compensateDrift);
      transferVelocities(false, flipRatio);
    }

    // updateParticleColors();
    updateCellColors();
  }

  void simulate()
  {
    simulate(scene.dt,
             scene.gravity,
             scene.flipRatio,
             scene.numPressureIters,
             scene.numParticleIters,
             scene.overRelaxation,
             scene.compensateDraft,
             scene.separateParticles,
             scene.obstacleX,
             scene.obstacleY,
             scene.obstacleRadius);
  }

  FlipFluid(double width, double height)
  {
    // scene.obstacleRadius = 1.0;
    scene.obstacleRadius = 0.15f;
    scene.overRelaxation = 1.9f;

    scene.dt = 1.0 / 60.0;
    scene.numPressureIters = 50;
    scene.numParticleIters = 2;

    simHeight = 3.0;
    simScale = height / simHeight;
    simWidth = width / simScale;

    int res = 100;

    double tankHeight = 1.0 * simHeight;
    double tankWidth = 1.0 * simWidth;

    double res_h = tankHeight / (double)res;
    double density = 1000.0;

    double relWaterHeight = 0.8f;
    double relWaterWidth = 0.6f;

    auto r = 0.3 * res_h;
    auto dx = 2.0 * r;
    auto dy = std::sqrt(3.0) / 2.0 * dx;

    auto nx =
        std::floor((relWaterWidth * tankWidth - 2.0 * res_h - 2.0 * r) / dx);
    auto ny =
        std::floor((relWaterHeight * tankHeight - 2.0 * res_h - 2.0 * r) / dy);
    auto maxParticles = nx * ny;

    // ##################

    init_fluid(density, tankWidth, tankHeight, res_h, r, maxParticles);

    numParticles = nx * ny;

    // ##################

    // create particles

    auto p = 0;
    for (auto i = 0; i < nx; i++) {
      for (auto j = 0; j < ny; j++) {
        particlePos[p++] = res_h + r + dx * i + (j % 2 == 0 ? 0.0 : r);
        particlePos[p++] = res_h + r + dy * j;
      }
    }

    // setup grid collisions

    auto n = fNumY;
    for (auto i = 0; i < fNumX; i++) {
      for (auto j = 0; j < fNumY; j++) {
        auto s = 1.0;  // fluid
        if (i == 0 || i == (int)fNumX - 1 || j == 0) {
          s = 0.0;  // solid
        }
        this->s[i * n + j] = s;
      }
    }

    setObstacle(3.0, 2.0, true);
  }

  Scene scene;

  double simScale;
  double simWidth, simHeight;

  double density;
  double fNumX, fNumY;
  double h;
  double fInvSpacing;
  double fNumCells;

  std::vector<double> u;
  std::vector<double> v;
  std::vector<double> du;
  std::vector<double> dv;
  std::vector<double> prevU;
  std::vector<double> prevV;
  std::vector<double> p;
  std::vector<double> s;
  std::vector<CellType> cellType;
  std::vector<Color> cellColor;

  int maxParticles;
  std::vector<double> particlePos;
  std::vector<Color> particleColor;

  std::vector<double> particleVel;
  std::vector<double> particleDensity;

  double particleRestDensity;

  double particleRadius;
  double pInvSpacing;
  double pNumX, pNumY;
  double pNumCells;

  std::vector<int> numCellParticles;
  std::vector<int> firstCellParticle;
  std::vector<int> cellParticleIds;

  int numParticles;
};
}  // namespace sim