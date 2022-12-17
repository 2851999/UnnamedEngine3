#pragma once

#include "../maths/Vector.h"

/*****************************************************************************
 * Colour class - Handles colours
 *****************************************************************************/

class Colour : public Vector4f {
public:
    /* Predetermined colours */
    static const Colour NONE;
    static const Colour BLACK;
    static const Colour GREY;
    static const Colour LIGHT_GREY;
    static const Colour RED;
    static const Colour ORANGE;
    static const Colour BROWN;
    static const Colour YELLOW;
    static const Colour PINK;
    static const Colour PURPLE;
    static const Colour GREEN;
    static const Colour BLUE;
    static const Colour LIGHT_BLUE;
    static const Colour WHITE;

    /* Various constructors */
    Colour() : Vector4f() {}
    Colour(const std::initializer_list<float>& values) : Vector4f(values) {}
    Colour(const float value) : Vector4f(value) {}
    Colour(const Vector2<float>& base, const float z, const float w) : Vector4f(base, z, w) {}
    Colour(const Vector3<float>& base, const float w) : Vector4f(base, w) {}

    Colour(const Colour colour, const float a) : Vector4f(colour.getR(), colour.getG(), colour.getB(), a) {}
    Colour(const float grey, const float a = 1.0f) : Vector4f(grey, grey, grey, a) {}
    Colour(const float r, const float g, const float b, const float a = 1.0f) : Vector4f(r, g, b, a) {}

    Colour(const Vector<float, 4>& base) : Vector4f(base) {}
    Colour(const Vector4<float>& base) : Vector4f(base) {}
    Colour(const Vector4f& base) : Vector4f(base) {}

    /* Various setters and getters */
    void setR(float r) { setX(r); }
    void setG(float g) { setY(g); }
    void setB(float b) { setZ(b); }
    void setA(float a) { setW(a); }

    float getR() const { return getX(); }
    float getG() const { return getY(); }
    float getB() const { return getZ(); }
    float getA() const { return getW(); }
};