
#include "Quaternion.h"

#include "Matrix.h"

/*****************************************************************************
 * Quaternion
 *****************************************************************************/

Quaternion& Quaternion::initFromAxisAngle(const Vector3f& axis, float angle) {
    // Calculate some needed constants for transforming it into the quaternion
    // representation
    float a = utils_maths::toRadians(angle / 2.0f);
    float s = sinf(a);
    float c = cosf(a);
    // Assign the values
    setX(axis.getX() * s);
    setY(axis.getY() * s);
    setZ(axis.getZ() * s);
    setW(c);

    return *this;
}

Quaternion& Quaternion::initFromEulerAngles(const Vector3f& angles) {
    float h = utils_maths::toRadians(angles.getY());
    float a = utils_maths::toRadians(angles.getZ());
    float b = utils_maths::toRadians(angles.getX());

    float c1 = cosf(h / 2.0f);
    float s1 = sinf(h / 2.0f);
    float c2 = cosf(a / 2.0f);
    float s2 = sinf(a / 2.0f);
    float c3 = cosf(b / 2.0f);
    float s3 = sinf(b / 2.0f);
    float c1c2 = c1 * c2;
    float s1s2 = s1 * s2;

    setW(c1c2 * c3 - s1s2 * s3);
    setX(c1c2 * s3 + s1s2 * c3);
    setY(s1 * c2 * c3 + c1 * s2 * s3);
    setZ(c1 * s2 * c3 - s1 * c2 * s3);

    return *this;
}

Quaternion& Quaternion::initFromRotationMatrix(const Matrix4f& m) {
    float trace = m.get(0, 0) + m.get(1, 1) + m.get(2, 2);

    if (trace > 0) {
        float s = 0.5f / sqrtf(trace + 1.0f);

        setW(0.25f / s);
        setX((m.get(2, 1) - m.get(1, 2)) * s);
        setY((m.get(0, 2) - m.get(2, 0)) * s);
        setZ((m.get(1, 0) - m.get(1, 0)) * s);
    } else if (m.get(0, 0) > m.get(1, 1) && m.get(0, 0) > m.get(2, 2)) {
        float s = 2.0f * sqrtf(1.0f + m.get(0, 0) - m.get(1, 1) - m.get(2, 2));

        setW((m.get(2, 1) - m.get(1, 2)) / s);
        setX(0.25f * s);
        setY((m.get(0, 1) + m.get(1, 0)) / s);
        setZ((m.get(0, 2) + m.get(2, 0)) / s);
    } else if (m.get(1, 1) > m.get(2, 2)) {
        float s = 2.0f * sqrtf(1.0f + m.get(1, 1) - m.get(0, 0) - m.get(2, 2));

        setW((m.get(0, 2) - m.get(2, 0)) / s);
        setX((m.get(0, 1) + m.get(1, 0)) / s);
        setY(0.25f * s);
        setZ((m.get(1, 2) + m.get(2, 1)) / s);
    } else {
        float s = 2.0f * sqrtf(1.0f + m.get(2, 2) - m.get(1, 1) - m.get(0, 0));

        setW((m.get(1, 0) - m.get(0, 1)) / s);
        setX((m.get(0, 2) + m.get(2, 0)) / s);
        setY((m.get(2, 1) + m.get(2, 1)) / s);
        setZ(0.25f * s);
    }
    normalise();

    return *this;
}

Quaternion& Quaternion::initLookAt(const Vector3f& eye, const Vector3f& centre, const Vector3f& up) {
    return initFromRotationMatrix(Matrix4f().initLookAt(eye, centre, up));
}

Quaternion Quaternion::slerp(const Quaternion& quatA, const Quaternion& quatB, float factor) {
    // https://en.wikipedia.org/wiki/Slerp
    Quaternion v0 = quatA;
    Quaternion v1 = quatB;
    Quaternion result;

    float dot = v0.dot(v1);
    if (dot < 0.0f) {
        v0 = v0 * -1.0f;
        dot = -dot;
    }

    float s0 = 1.0f - factor;
    float s1 = factor;

    const float THRESHOLD = 0.9995f;
    if (dot < THRESHOLD) {
        float theta = acosf(dot);
        float invSin = 1.0f / sinf(theta);

        s0 = sinf(s0 * theta) * invSin;
        s1 = sinf(s1 * theta) * invSin;
    }

    result[0] = (s0 * v0[0]) + (s1 * v1[0]);
    result[1] = (s0 * v0[1]) + (s1 * v1[1]);
    result[2] = (s0 * v0[2]) + (s1 * v1[2]);
    result[3] = (s0 * v0[3]) + (s1 * v1[3]);

    return result.normalise();
}

Matrix4f Quaternion::toMatrix() const {
    // https://en.wikipedia.org/wiki/Quaternions_and_spatial_rotation
    Matrix4f mat;

    float i = getX();
    float j = getY();
    float k = getZ();
    float r = getW();

    float ii = i * i;
    float ij = i * j;
    float ik = i * k;
    float ir = i * r;
    float jj = j * j;
    float jk = j * k;
    float jr = j * r;
    float kk = k * k;
    float kr = k * r;

    // clang-format off
	mat.set(0, 0, 1 - 2 * (jj + kk)); mat.set(0, 1, 2 * (ij - kr));     mat.set(0, 2, 2 * (ik + jr));     mat.set(0, 3, 0);
	mat.set(1, 0, 2 * (ij + kr));     mat.set(1, 1, 1 - 2 * (ii + kk)); mat.set(1, 2, 2 * (jk - ir));     mat.set(1, 3, 0);
	mat.set(2, 0, 2 * (ik - jr));     mat.set(2, 1, 2 * (jk + ir));     mat.set(2, 2, 1 - 2 * (ii + jj)); mat.set(2, 3, 0);
	mat.set(3, 0, 0);                 mat.set(3, 1, 0);                 mat.set(3, 2, 0);                 mat.set(3, 3, 1);
    // clang-format on

    return mat;
}

Vector3f Quaternion::toEulerAngles() {
    float sqw = getW() * getW();
    float sqx = getX() * getX();
    float sqy = getY() * getY();
    float sqz = getZ() * getZ();

    float unit = sqx + sqy + sqz + sqw;
    float test = getX() * getY() + getZ() * getW();
    float h, a, b;

    if (test > 0.499f * unit) {
        h = 2 * atan2f(getX(), getW());
        a = utils_maths::PI / 2.0f;
        b = 0;
    } else if (test < -0.499f * unit) {
        h = -2 * atan2f(getX(), getW());
        a = -utils_maths::PI / 2.0f;
        b = 0;
    } else {
        h = atan2f(2 * getY() * getW() - 2.0f * getX() * getZ(), sqx - sqy - sqz + sqw);
        a = asinf(2 * test / unit);
        b = atan2f(2 * getX() * getW() - 2.0f * getY() * getZ(), -sqx + sqy - sqz + sqw);
    }

    return Vector3f(utils_maths::toDegrees(b), utils_maths::toDegrees(h), utils_maths::toDegrees(a));
}

Vector3f Quaternion::rotate(const Vector3f& vector, const Quaternion& quaternion) {
    // http://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
    Vector3f u(quaternion.getX(), quaternion.getY(), quaternion.getZ());
    float s = quaternion.getW();

    return (u * 2.0f * u.dot(vector) + vector * (s * s - u.dot(u)) + u.cross(vector) * 2.0f * s);
}