#pragma once

#include "config.h"
#include <algorithm>
#include <chrono>
#include <functional>
#include <sstream>

#define IN_BOUND(x) (0 <= (x) && (x) < WORLD_SIZE)

struct Vec3 {
    int x;
    int y;
    int z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(int x, int y, int z) : x(x), y(y), z(z) {}
    Vec3(int x) : x(x), y(x), z(x) {}

    Vec3 operator+(const Vec3 &v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator/(int d) const { return Vec3(x / d, y / d, z / d); }

    bool in_world() const { return IN_BOUND(x) && IN_BOUND(y) && IN_BOUND(z); }
    void invalidate() { x = WORLD_SIZE + 1; }
    std::string to_string() const {
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
    void clear() {
        for (size_t i = 0; i < N * N * N; i++) {
            buffer[i].~ValueType();
            new (&buffer[i]) ValueType();
        }
    }

    inline ValueType *get_buffer() { return buffer; }

  private:
    ValueType *buffer;
};

static size_t time_function(std::function<void()> f) {
    auto start = std::chrono::steady_clock::now();
    { f(); }
    auto end = std::chrono::steady_clock::now();
    size_t ms_elapsed = std::chrono::duration<double, std::milli>(end - start).count();
    return ms_elapsed;
}
