#ifndef VECTOR2_H
#define VECTOR2_H

#include <utility>
#include <cstdint>
#include <cmath>

template<class T>
class Vector2
{
 public:
  Vector2(T x = 0, T y = 0) : v{x,y} {}

  T &operator[](unsigned index) { return v[index]; }
  T operator[](unsigned index) const { return v[index]; }

  bool operator==(const Vector2<T> &other) { return v[0]==other[0] && v[1]==other[1]; }
  bool operator!=(const Vector2<T> &other) { return v[0]!=other[0] || v[1]!=other[1]; }

  Vector2<T> operator*(const Vector2<T> &other) const { return Vector2<T>(v[0]*other[0], v[1]*other[1]); }

  Vector2<T> operator*(float scale) const { return Vector2<T>(v[0]*scale, v[1]*scale); }
  Vector2<T> operator/(float scale) const { return Vector2<T>(v[0]/scale, v[1]/scale); }
  Vector2<T> operator+(const Vector2<T> &other) const { return Vector2<T>(v[0]+other[0], v[1]+other[1]); }
  Vector2<T> operator-(const Vector2<T> &other) const { return Vector2<T>(v[0]-other[0], v[1]-other[1]); }
  Vector2<T> operator-() const { return Vector2<T>(-v[0], -v[1]); }

  const Vector2<T> &operator+=(const Vector2<T> &rhs) { v[0] += rhs.v[0]; v[1] += rhs.v[1]; return *this; }

  float lengthSquared() const { return v[0]*v[0] + v[1]*v[1]; }
  float length() const { return sqrt(lengthSquared()); }

  T cross(const Vector2<T> &o) const { return v[0] * o[1] - v[1] * o[0]; }
  T dot(const Vector2<T> &o) const { return v[0] * o[0] + v[1] * o[1]; }
  Vector2<T> reflect(const Vector2<T> &o) const { return *this - (o*2*dot(o)); }

  Vector2<T> normalize() const { return lengthSquared() > 0.001 ? (*this)/length() : Vector2<T>(0,0); }

 private:
  T v[2];
};

typedef Vector2<float> Vec2f;
typedef Vector2<uint16_t> Vec2U16;

#endif
