#pragma once

#include <glm/glm.hpp>

class Ray {
  public:
    Ray(glm::vec3 pos, glm::vec3 dir) : pos(pos), dir(glm::normalize(dir)) {}

    glm::vec3 pos, dir;
};

class BBox {
  public:
    BBox(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}

    float hit(const Ray &ray) const {
        glm::vec3 invdir = 1.f / ray.dir;

        float tmin, tmax;
        if (invdir.x >= 0) {
            tmin = (min.x - ray.pos.x) * invdir.x;
            tmax = (max.x - ray.pos.x) * invdir.x;
        } else {
            tmin = (max.x - ray.pos.x) * invdir.x;
            tmax = (min.x - ray.pos.x) * invdir.x;
        }
        assert(tmin <= tmax);

        float tymin, tymax;

        if (invdir.y >= 0) {
            tymin = (min.y - ray.pos.y) * invdir.y;
            tymax = (max.y - ray.pos.y) * invdir.y;
        } else {
            tymin = (max.y - ray.pos.y) * invdir.y;
            tymax = (min.y - ray.pos.y) * invdir.y;
        }
        assert(tymin <= tymax);

        if ((tmin > tymax) || (tymin > tmax))
            return -1;

        if (tymin > tmin)
            tmin = tymin;

        if (tymax < tmax)
            tmax = tymax;

        float tzmin, tzmax;
        if (invdir.z >= 0) {
            tzmin = (min.z - ray.pos.z) * invdir.z;
            tzmax = (max.z - ray.pos.z) * invdir.z;
        } else {
            tzmin = (max.z - ray.pos.z) * invdir.z;
            tzmax = (min.z - ray.pos.z) * invdir.z;
        }

        assert(tzmin <= tzmax);

        if ((tmin > tzmax) || (tzmin > tmax))
            return -1;

        if (tzmin > tmin)
            tmin = tzmin;

        if (tzmax < tmax)
            tmax = tzmax;

        if (tmin >= 0) {
            return tmin;
        }
        if (tmax >= 0) {
            return tmax;
        }

        return -1;
    }

    glm::vec3 min, max;
};
