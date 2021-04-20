#pragma once

#include "config.h"
#include <algorithm>

#define IN_BOUND(x) (0 <= (x) && (x) < WORLD_SIZE)

struct Vec3 {
    int x;
    int y;
    int z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(int x, int y, int z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3 &v) const { return Vec3(x + v.x, y + v.y, z + v.z); }

    bool in_world() const { return IN_BOUND(x) && IN_BOUND(y) && IN_BOUND(z); }
    void invalidate() { x = WORLD_SIZE + 1; }
    std::string to_string() {
        std::stringstream ss;
        ss << "<" << x << ", " << y << ", " << z << ">";
        return ss.str();
    }
};

template <typename ValueType, unsigned int N> class Tensor {
  public:
    Tensor() { buffer = new ValueType[N * N * N]; }
    ~Tensor() { delete[] buffer; }

    inline ValueType &operator()(int x, int y, int z) { return at(x, y, z); }
    inline ValueType &operator()(const Vec3 &v) { return at(v.x, v.y, v.z); }

    inline ValueType &at(int x, int y, int z) { return buffer[x * N * N + y * N + z]; }

    void clear(ValueType v) { std::fill((ValueType *)buffer, (ValueType *)buffer + N * N * N, v); }

    inline ValueType *get_buffer() { return buffer; }

  private:
    ValueType *buffer;
};
