#pragma once

#include "Vector.h"

class Matrix4f;

/*****************************************************************************
 * Quaternion - Used to represent quaternions
 *****************************************************************************/

class Quaternion : public Vector4f {
public:
    /* Various constructors */
    Quaternion() {}
    Quaternion(const std::initializer_list<float>& values) : Vector4f(values) {}
    Quaternion(const float value) : Vector4f(value) {}
    Quaternion(const float x, const float y, const float z, const float w) : Vector4f(x, y, z, w) {}
    Quaternion(const Vector2<float>& base, const float z, const float w) : Vector4f(base, z, w) {}
    Quaternion(const Vector3<float>& base, const float w) : Vector4f(base, w) {}

    Quaternion(const Vector<float, 4>& base) : Vector4f(base) {}
    Quaternion(const Vector4<float>& base) : Vector4f(base) {}

    /* Multiplies this quaternion by another and returns the result */
    inline Quaternion operator*(const Quaternion& other) const {
        Quaternion result;

        result[0] = (getX() * other.getW()) + (getW() * other.getX()) + (getY() * other.getZ()) - (getZ() * other.getY());
        result[1] = (getY() * other.getW()) + (getW() * other.getY()) + (getZ() * other.getX()) - (getX() * other.getZ());
        result[2] = (getZ() * other.getW()) + (getW() * other.getZ()) + (getX() * other.getY()) - (getY() * other.getX());
        result[3] = (getW() * other.getW()) - (getX() * other.getX()) - (getY() * other.getY()) - (getZ() * other.getZ());

        return result;
    }

    /* Multiplies this quaternion by another */
    inline Quaternion operator*=(const Quaternion& other) {
        Quaternion result = (*this) * other;

        setX(result.getX());
        setY(result.getY());
        setZ(result.getZ());
        setW(result.getW());

        return *this;
    }

    /* Intitialises this quaternion using a rotation about an axis (angle in degrees) */
    Quaternion& initFromAxisAngle(const Vector3f& axis, float angle);

    /* Intitialises this quaternion using euler angles (in degrees) */
    Quaternion& initFromEulerAngles(const Vector3f& angles);

    /* Initialises this quaternion from a rotation matrix */
    Quaternion& initFromRotationMatrix(const Matrix4f& matrix);

    /* Initialises this quaternion to look at a particular point */
    Quaternion& initLookAt(const Vector3f& eye, const Vector3f& centre, const Vector3f& up);

    /* Returns the conjugate of this quaternion */
    inline Quaternion conjugate() const { return Quaternion(-getX(), -getY(), -getZ(), getW()); }

    /* Converts this quaternion to a rotation matrix */
    Matrix4f toMatrix() const;

    /* Returns the euler angles representing this quaternion's rotation (in degrees) */
    Vector3f toEulerAngles();

    /* Returns various unit vectors after being rotated by this quaternion */
    inline Vector3f getForward() const { return rotate(Vector3f(0.0f, 0.0f, -1.0f), (*this)); }
    inline Vector3f getBackward() const { return rotate(Vector3f(0.0f, 0.0f, 1.0f), (*this)); }
    inline Vector3f getUp() const { return rotate(Vector3f(0.0f, 1.0f, 0.0f), (*this)); }
    inline Vector3f getDown() const { return rotate(Vector3f(0.0f, -1.0f, 0.0f), (*this)); }
    inline Vector3f getLeft() const { return rotate(Vector3f(-1.0f, 0.0f, 0.0f), (*this)); }
    inline Vector3f getRight() const { return rotate(Vector3f(1.0f, 0.0f, 0.0f), (*this)); }

    /* Spherical linear interpolation between two quaternions */
    static Quaternion slerp(const Quaternion& quatA, const Quaternion& quatB, float factor);

    /* Rotates a vector using a quaternion */
    static Vector3f rotate(const Vector3f& vector, const Quaternion& quaternion);
};