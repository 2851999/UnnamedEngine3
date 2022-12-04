#pragma once

#include <stdexcept>

#include "../../utils/StringUtils.h"
#include "Utils.h"

/*****************************************************************************
 * Generalised version of a Vector
 *****************************************************************************/

/* T - type of value, N - dimension of vector */

template <typename T, unsigned int N>
class Vector {
protected:
    /* Always initialise member variables */
    T values[N] = {};

public:
    /* Constructor */
    Vector() {}

    /* Constructor taking an initial value */
    Vector(const std::initializer_list<T>& values) {
        // Ensure correct size
        if (values.size() != N)
            throw std::invalid_argument("Incorrect number of elements");
        std::copy(values.begin(), values.end(), this->values);
    }

    /* Operations for obtaining/assigning values */
    inline T& operator[](unsigned int idx) { return this->values[idx]; }
    inline const T& operator[](unsigned int idx) const { return this->values[idx]; }

    /* Adds another vector to this one and returns the result */
    inline Vector<T, N> operator+(const Vector<T, N>& other) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] + other[i];
        return result;
    }

    /* Subtracts another vector from this one and returns the result */
    inline Vector<T, N> operator-(const Vector<T, N>& other) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] - other[i];
        return result;
    }

    /* Returns the result of element-wise multiplication of this vector by another*/
    inline Vector<T, N> operator*(const Vector<T, N>& other) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] * other[i];
        return result;
    }

    /* Returns the result of element-wise division of this vector by another*/
    inline Vector<T, N> operator/(const Vector<T, N>& other) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] / other[i];
        return result;
    }

    /* Adds another vector to this one */
    inline Vector<T, N>& operator+=(const Vector<T, N>& other) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] + other[i];
        return *this;
    }

    /* Subtracts another vector from this one */
    inline Vector<T, N>& operator-=(const Vector<T, N>& other) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] - other[i];
        return *this;
    }

    /* Element-wise multiplication of this vector by another one */
    inline Vector<T, N>& operator*=(const Vector<T, N>& other) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] * other[i];
        return *this;
    }

    /* Element-wise division of this vector by another one */
    inline Vector<T, N>& operator/=(const Vector<T, N>& other) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] * other[i];
        return *this;
    }

    /* Multiplies this vector by a scalar and returns the result */
    inline Vector<T, N>& operator*(const T& scalar) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] * scalar;
        return result;
    }

    /* Divides this vector by a scalar and returns the result */
    inline Vector<T, N>& operator/(const T& scalar) const {
        Vector<T, N> result;
        for (unsigned int i = 0; i < N; ++i)
            result[i] = this->values[i] / scalar;
        return result;
    }

    /* Multiplies this vector by a scalar */
    inline Vector<T, N>& operator*=(const T& scalar) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] * scalar;
        return *this;
    }

    /* Divides this vector by a scalar */
    inline Vector<T, N>& operator/=(const T& scalar) {
        for (unsigned int i = 0; i < N; ++i)
            this->values[i] = this->values[i] * scalar;
        return *this;
    }

    /* Compares this vector to another one and returns whether they are equal
       or not */
    inline bool operator==(const Vector<T, N>& other) {
        // Return false if any element isn't equal
        for (unsigned int i = 0; i < N; ++i) {
            if (other[i] != values[i])
                return false;
        }
        return true;
    }

    /* Returns the length of this vector */
    inline T length() const {
        T total = 0;
        // Add on square of each value to the total
        for (unsigned int i = 0; i < N; ++i)
            total += (this->values[i] * this->values[i]);
        // Return length
        return sqrt(total);
    }

    /* Other comparison operators */
    inline bool operator!=(const Vector<T, N>& other) const { return ((*this) == other); }
    inline bool operator<(const Vector<T, N>& other) const { return this->length() < other.length(); }
    inline bool operator<=(const Vector<T, N>& other) const { return this->length() <= other.length(); }
    inline bool operator>(const Vector<T, N>& other) const { return this->length() > other.length(); }
    inline bool operator>=(const Vector<T, N>& other) const { return this->length() >= other.length(); }

    /* Returns the dot product of this and another vector */
    inline T dot(const Vector<T, N>& other) const {
        T result = 0;
        for (unsigned int i = 0; i < N; ++i)
            result += this->values[i] * other[i];
        return result;
    }

    /* Normalises this vector */
    inline Vector<T, N>& normalise() {
        T length = this->length();
        for (unsigned int i = 0; i < N; ++i)
            values[i] /= length;
        return (*this);
    }

    /* Returns this vector but normalised (without changing the result of this one) */
    inline Vector<T, N> normalised() const {
        Vector<T, N> result = (*this);
        return result.normalise();
    }

    /* Linear interpolation between two vectors */
    inline static Vector<T, N> lerp(const Vector<T, N>& vectorA, const Vector<T, N>& vectorB, T factor) { return (vectorA + ((vectorB - vectorA) * factor)); }

    /* Converts this vector to a string format */
    std::string toString() {
        std::string result = "(";
        for (unsigned int i = 0; i < N; ++i) {
            result += utils_string::str(values[i]);
            if (i != N - 1)
                result += ",";
        }
        return result + ")";
    }

    /* Returns the number of elements in this vector */
    inline int getNumElements() { return N * N; }

    /* Returns the size of this vector in bytes */
    inline int getSize() { return N * sizeof(T); }
};

/*****************************************************************************
 * Slightly more specific versions of Vector
 *****************************************************************************/

template <unsigned int N>
class VectorFloat {
    /* Spherical linear interpolation between two vectors
       This function is very specific to using floats */
    inline static Vector<float, N> slerp(const Vector<float, N>& vectorA, const Vector<float, N>& vectorB, float factor) {
        // https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
        float dot = vectorA.dot(vectorB);
        dot = utils_maths::clamp(dot, -1.0f, 1.0f);
        float theta = acosf(dot) * factor;
        Vector<float, N> relative = vectorB - vectorA * dot;
        relative.normalise();
        return ((vectorA * cosf(theta)) + (relative * sinf(theta)));
    }
};

/* 2-dimensional vector */
template <typename T>
class Vector2 : public Vector<T, 2> {
public:
    /* Various constructors */
    Vector2() {}
    Vector2(const std::initializer_list<T>& values) : Vector<T, 2>(values) {}
    Vector2(const T value) : Vector<T, 2>({value, value}) {}
    Vector2(const T x, const T y) : Vector<T, 2>({x, y}) {}
    Vector2(const Vector<T, 3>& base) : Vector<T, 2>({base[0], base[1]}) {}

    Vector2(const Vector<T, 2>& base) : Vector<T, 2>(base) {}

    /* Setters and getters */
    inline void setX(T x) { this->values[0] = x; }
    inline void setY(T y) { this->values[1] = y; }

    inline T getX() const { return this->values[0]; }
    inline T getY() const { return this->values[1]; }
};

/* 3-dimensional vector */
template <typename T>
class Vector3 : public Vector<T, 3> {
public:
    /* Various constructors */
    Vector3() {}
    Vector3(const std::initializer_list<T>& values) : Vector<T, 3>(values) {}
    Vector3(const T value) : Vector<T, 3>({value, value, value}) {}
    Vector3(const T x, const T y, const T z) : Vector<T, 3>({x, y, z}) {}
    Vector3(const Vector<T, 2>& base, const T z) : Vector<T, 3>({base[0], base[1], z}) {}

    Vector3(const Vector<T, 3>& base) : Vector<T, 3>(base) {}

    /* Performs a cross product with another vector and returns the result */
    inline Vector3<T> cross(const Vector3<T>& other) const {
        return Vector3<T>(
            this->getY() * other.getZ() - getZ() * other.getY(),
            this->getZ() * other.getX() - getX() * other.getZ(),
            this->getX() * other.getY() - getY() * other.getX());
    }

    /* Setters and getters */
    inline void setX(T x) { this->values[0] = x; }
    inline void setY(T y) { this->values[1] = y; }
    inline void setZ(T z) { this->values[3] = z; }

    inline T getX() const { return this->values[0]; }
    inline T getY() const { return this->values[1]; }
    inline T getZ() const { return this->values[2]; }
};

/* 4-dimensional vector */
template <typename T>
class Vector4 : public Vector<T, 4> {
public:
    /* Various constructors */
    Vector4() {}
    Vector4(const std::initializer_list<T>& values) : Vector<T, 4>(values) {}
    Vector4(const T value) : Vector<T, 4>({value, value, value, value}) {}
    Vector4(const T x, const T y, const T z, const T w) : Vector<T, 4>({x, y, z, w}) {}
    Vector4(const Vector<T, 2>& base, const T z, const T w) : Vector<T, 4>({base[0], base[1], z, w}) {}
    Vector4(const Vector<T, 3>& base, const T w) : Vector<T, 4>({base[0], base[1], base[2], w}) {}

    Vector4(const Vector<T, 4>& base) : Vector<T, 4>(base) {}

    /* Setters and getters */
    inline void setX(T x) { this->values[0] = x; }
    inline void setY(T y) { this->values[1] = y; }
    inline void setZ(T z) { this->values[2] = z; }
    inline void setW(T w) { this->values[3] = w; }

    inline T getX() const { return this->values[0]; }
    inline T getY() const { return this->values[1]; }
    inline T getZ() const { return this->values[2]; }
    inline T getW() const { return this->values[3]; }
};

/*****************************************************************************
 * Specific versions of Vector
 *****************************************************************************/

using Vector2i = Vector2<int>;

class Vector2f : public Vector2<float>, public VectorFloat<2> {
public:
    /* Various constructors */
    Vector2f() {}
    Vector2f(const std::initializer_list<float>& values) : Vector2<float>(values) {}
    Vector2f(const float value) : Vector2<float>(value) {}
    Vector2f(const float x, const float y) : Vector2<float>(x, y) {}
    Vector2f(const Vector3<float>& base) : Vector2<float>({base.getX(), base.getY()}) {}
    Vector2f(const Vector4<float>& base) : Vector2<float>({base.getX(), base.getY(), base.getZ()}) {}

    Vector2f(const Vector<float, 2>& base) : Vector2<float>(base) {}
    Vector2f(const Vector2<float>& base) : Vector2<float>(base) {}
};

using Vector2d = Vector2<double>;

using Vector3i = Vector3<int>;

class Vector3f : public Vector3<float>, public VectorFloat<3> {
public:
    /* Various constructors */
    Vector3f() {}
    Vector3f(const std::initializer_list<float>& values) : Vector3<float>(values) {}
    Vector3f(const float value) : Vector3<float>(value) {}
    Vector3f(const float x, const float y, const float z) : Vector3<float>(x, y, z) {}
    Vector3f(const Vector2<float>& base, const float z) : Vector3<float>(base, z) {}
    Vector3f(const Vector4<float>& base) : Vector3<float>({base.getX(), base.getY(), base.getZ()}) {}

    Vector3f(const Vector<float, 3>& base) : Vector3<float>(base) {}
    Vector3f(const Vector3<float>& base) : Vector3<float>(base) {}
};

using Vector3d = Vector3<double>;

using Vector4i = Vector4<int>;

class Vector4f : public Vector4<float>, public VectorFloat<4> {
public:
    /* Various constructors */
    Vector4f() {}
    Vector4f(const std::initializer_list<float>& values) : Vector4<float>(values) {}
    Vector4f(const float value) : Vector4<float>(value) {}
    Vector4f(const float x, const float y, const float z, const float w) : Vector4<float>(x, y, z, w) {}
    Vector4f(const Vector2<float>& base, const float z, const float w) : Vector4<float>(base, z, w) {}
    Vector4f(const Vector3<float>& base, const float w) : Vector4<float>(base, w) {}

    Vector4f(const Vector<float, 4>& base) : Vector4<float>(base) {}
    Vector4f(const Vector4<float>& base) : Vector4<float>(base) {}
};

using Vector4d = Vector4<double>;