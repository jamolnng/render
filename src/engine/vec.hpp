#pragma once

#include <cmath>

namespace engine
{
template<typename T>
class Vec2
{
public:
  using type = T;
  using base = Vec2<T>;

  Vec2(T x, T y)
      : _x(x)
      , _y(y)
  {
  }

  template<typename U>
  Vec2(const Vec2<U>& other)
      : _x(other._x)
      , _y(other._y)
  {
  }

  T x() const { return _x; };
  T y() const { return _y; };

  void set(T x, T y)
  {
    _x = x;
    _y = y;
  }
  void set_x(T x) { _x = x; }
  void set_y(T y) { _y = y; }

  template<typename U>
  auto operator<=>(const Vec2<U>& other) const
  {
    if (auto cmp = _x <=> other._x; cmp != 0) {
      return cmp;
    }
    return _y <=> other._y;
  }

  template<typename U>
  bool operator==(const Vec2<U>& other) const
  {
    return _x == other._x && _y == other._y;
  }

  template<typename U>
  inline Vec2<T>& operator+=(const Vec2<U>& other)
  {
    _x += other._x;
    _y += other._y;
    return *this;
  }

  template<typename U>
  Vec2<T> operator+(const Vec2<U>& other) const
  {
    return {_x + other._x, _y + other._y};
  }

  template<typename U>
  inline Vec2<T>& operator-=(const Vec2<U>& other)
  {
    _x -= other._x;
    _y -= other._y;
    return *this;
  }

  template<typename U>
  Vec2<T> operator-(const Vec2<U>& other) const
  {
    return {_x - other._x, _y - other._y};
  }

  template<typename U>
  Vec2<T>& operator=(const Vec2<U>& other)
  {
    _x = other._x;
    _y = other._y;
    return *this;
  }

  T length() const { return std::sqrt(_x * _x + _y * _y); };
  Vec2<T> norm()
  {
    auto l = length();
    return {_x / l, _y / l};
  }

private:
  T _x, _y;
};

template<typename T, typename U>
static Vec2<T> operator+(const Vec2<T>& a, const Vec2<U> b)
{
  return {a.x() + b.x(), a.y() + b.y()};
}

template<typename T, typename U>
static Vec2<T> operator-(const Vec2<T>& a, const Vec2<U> b)
{
  return {a.x() - b.x(), a.y() - b.y()};
}

template<typename T>
class Vec3
{
public:
  using type = T;
  using base = Vec3<T>;

  Vec3(T x, T y, T z)
      : _x(x)
      , _y(y)
      , _z(z)
  {
  }

  template<typename U>
  Vec3(const Vec3<U>& other)
      : _x(other._x)
      , _y(other._y)
      , _z(other._z)
  {
  }

  void set(T x, T y, T z)
  {
    _x = x;
    _y = y;
    _z = z;
  }
  T x() const { return _x; };
  T y() const { return _y; };
  T z() const { return _z; };

  void set_x(T x) { _x = x; }
  void set_y(T y) { _y = y; }
  void set_z(T z) { _z = z; }

  template<typename U>
  auto operator<=>(const Vec3<U>& other) const
  {
    if (auto cmp = _x <=> other._x; cmp != 0) {
      return cmp;
    }
    if (auto cmp = _y <=> other._y; cmp != 0) {
      return cmp;
    }
    return _z <=> other._z;
  }

  template<typename U>
  bool operator==(const Vec3<U>& other) const
  {
    return _x == other._x && _y == other._y && _z == other._z;
  }

  template<typename U>
  Vec3<T>& operator+=(const Vec3<U>& other)
  {
    _x += other._x;
    _y += other._y;
    _z += other._z;
    return *this;
  }

  template<typename U>
  Vec3<T> operator+(const Vec3<U>& other) const
  {
    return {_x + other._x, _y + other._y, _z + other._z};
  }

  template<typename U>
  Vec3<T>& operator-=(const Vec3<U>& other)
  {
    _x -= other._x;
    _y -= other._y;
    _z -= other._z;
    return *this;
  }

  template<typename U>
  Vec3<T> operator-(const Vec3<U>& other) const
  {
    return {_x - other._x, _y - other._y, _z - other._z};
  }

  template<typename U>
  Vec3<T>& operator=(const Vec3<U>& other)
  {
    _x = other._x;
    _y = other._y;
    _z = other._z;
    return *this;
  }

  T length() const { return std::sqrt(_x * _x + _y * _y + _z * _z); };
  Vec3<T> norm()
  {
    auto l = length();
    return {_x / l, _y / l, _z / l};
  }

private:
  T _x, _y, _z;
};

template<typename T, typename U>
static Vec3<T> operator+(const Vec3<T>& a, const Vec3<U> b)
{
  return {a.x() + b.x(), a.y() + b.y(), a.z() + b.z()};
}

template<typename T, typename U>
static Vec3<T> operator-(const Vec3<T>& a, const Vec3<U> b)
{
  return {a.x() - b.x(), a.y() - b.y(), a.z() - b.z()};
}

template<typename T>
class Vec4
{
public:
  using type = T;
  using base = Vec4<T>;

  Vec4(T x, T y, T z, T w)
      : _x(x)
      , _y(y)
      , _z(z)
      , _w(w)
  {
  }

  template<typename U>
  Vec4(const Vec4<U>& other)
      : _x(other._x)
      , _y(other._y)
      , _z(other._z)
      , _w(other._w)
  {
  }

  void set(T x, T y, T z, T w)
  {
    _x = x;
    _y = y;
    _z = z;
    _w = w;
  }
  T x() const { return _x; };
  T y() const { return _y; };
  T z() const { return _z; };
  T w() const { return _w; };

  void set_x(T x) { _x = x; }
  void set_y(T y) { _y = y; }
  void set_z(T z) { _z = z; }
  void set_w(T w) { _w = w; }

  template<typename U>
  auto operator<=>(const Vec4<U>& other) const
  {
    if (auto cmp = _x <=> other._x; cmp != 0) {
      return cmp;
    }
    if (auto cmp = _y <=> other._y; cmp != 0) {
      return cmp;
    }
    if (auto cmp = _z <=> other._z; cmp != 0) {
      return cmp;
    }
    return _w <=> other._w;
  }

  template<typename U>
  bool operator==(const Vec4<U>& other) const
  {
    return _x == other._x && _y == other._y && _z == other._z && _w == other._w;
  }

  template<typename U>
  Vec4<T>& operator+=(const Vec4<U>& other)
  {
    _x += other._x;
    _y += other._y;
    _z += other._z;
    _w += other._w;
    return *this;
  }

  template<typename U>
  Vec4<T> operator+(const Vec4<U>& other) const
  {
    return {_x + other._x, _y + other._y, _z + other._z, _w + other._w};
  }

  template<typename U>
  Vec4<T>& operator-=(const Vec4<U>& other)
  {
    _x -= other._x;
    _y -= other._y;
    _z -= other._z;
    _w -= other._w;
    return *this;
  }

  template<typename U>
  Vec4<T> operator-(const Vec4<U>& other) const
  {
    return {_x - other._x, _y - other._y, _z - other._z, _w - other._w};
  }

  template<typename U>
  Vec4<T>& operator=(const Vec4<U>& other)
  {
    _x = other._x;
    _y = other._y;
    _z = other._z;
    _w = other._w;
    return *this;
  }

  T length() const { return std::sqrt(_x * _x + _y * _y + _z * _z + _w * _w); };
  Vec4<T> norm()
  {
    auto l = length();
    return {_x / l, _y / l, _z / l, _w / l};
  }

private:
  T _x, _y, _z, _w;
};

template<typename T, typename U>
static Vec4<T> operator+(const Vec4<T>& a, const Vec4<U> b)
{
  return {a.x() + b.x(), a.y() + b.y(), a.z() + b.z(), a.w() + b.w()};
}

template<typename T, typename U>
static Vec4<T> operator-(const Vec4<T>& a, const Vec4<U> b)
{
  return {a.x() - b.x(), a.y() - b.y(), a.z() - b.z(), a.w() - b.w()};
}

// class Dimensions : public Vec2<std::size_t>
// {
// public:
//   using base::base;

//   inline void set(type width, type height) { base::set(width, height); }
//   inline void set_width(type width) { set_x(width); }
//   inline void set_height(type height) { set_y(height); }

//   inline type width() { return x(); };
//   inline type height() { return y(); };
// };

// class BoundingBox : public Vec4<std::size_t>
// {
// public:
//   using base::base;

//   inline void set(type x, type y, type width, type height)
//   {
//     base::set(x, y, width, height);
//   }
//   using base::set_x;
//   using base::set_y;
//   inline void set_width(type width) { set_z(width); }
//   inline void set_height(type height) { set_w(height); }

//   using base::x;
//   using base::y;
//   inline type width() { return z(); };
//   inline type height() { return w(); };
// };

// class Color : public Vec4<float>
// {
// public:
//   Color(type r, type g, type b, type a) : base(r, g, b, a) {}
//   using base::base;

//   inline void set(type r, type g, type b, type a) { base::set(r, b, g, a); }
//   inline void set_r(type r) { set_x(r); }
//   inline void set_g(type g) { set_y(g); }
//   inline void set_b(type b) { set_z(b); }
//   inline void set_a(type a) { set_w(a); }

//   inline type r() { return x(); }
//   inline type g() { return y(); }
//   inline type b() { return z(); }
//   inline type a() { return w(); }
// };
}  // namespace engine