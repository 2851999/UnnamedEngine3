#pragma once

#include "maths/Vector.h"

/*****************************************************************************
 * Sphere class - Defines a sphere and methods for determining intersections
 *****************************************************************************/

class Sphere {
public:
    /* Centre of this sphere */
    Vector3f centre;

    /* Radius of this sphere */
    float radius = 0.0f;

    /* Constructors and destructor */
    Sphere() {}
    Sphere(Vector3f centre, float radius) : centre(centre), radius(radius) {}
    Sphere(float x, float y, float z, float radius) : centre(Vector3f(x, y, z)), radius(radius) {}
    virtual ~Sphere() {}

    /* Returns whether this sphere contains a point */
    inline bool contains(Vector3f& point) {
        return (centre - point).length() < radius;
    }

    /* Returns whether this sphere intersects with a point */
    inline bool intersects(Vector3f& point) {
        return (centre - point).length() <= radius;
    }

    /* Returns whether this sphere intersects with another sphere */
    inline bool intersects(Sphere& other) {
        return (centre - other.centre).length() <= (radius + other.radius);
    }
};