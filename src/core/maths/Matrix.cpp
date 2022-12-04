#include "Matrix.h"

/*****************************************************************************
 * Matrix3f
 *****************************************************************************/

Matrix3f Matrix3f::inverse() const {
    Matrix3f result;

    // Assign each value letters to make the calculation easier to follow
    // clang-format off
	float a = get(0, 0); float b = get(0, 1); float c = get(0, 2);
	float d = get(1, 0); float e = get(1, 1); float f = get(1, 2);
	float g = get(2, 0); float h = get(2, 1); float i = get(2, 2);
    // clang-format on

    // Now calculate the matrix of cofactors, using capital letters in the
    // same order as the lower case ones
    float A = (e * i - f * h);
    float B = (f * g - d * i);
    float C = (d * h - e * g);
    float D = (h * c - i * b);
    float E = (i * a - g * c);
    float F = (g * b - h * a);
    float G = (b * f - c * e);
    float H = (c * d - a * f);
    float I = (a * e - b * d);

    // From this, the inverse of the determinant is calculated so it
    // can be multiplied by the transpose of the matrix of cofactors
    // to get the inverse
    float det = (a * A + b * B + c * C);

    // Prevent a divide by 0
    if (det != 0.0f) {
        float invDet = 1.0f / (a * A + b * B + c * C);

        // Finally assign the result
        // clang-format off
		result.set(0, 0, invDet * A); result.set(0, 1, invDet * D); result.set(0, 2, invDet * G);
		result.set(1, 0, invDet * B); result.set(1, 1, invDet * E); result.set(1, 2, invDet * H);
		result.set(2, 0, invDet * C); result.set(2, 1, invDet * F); result.set(2, 2, invDet * I);
        // clang-format on
    }

    return result;
}

/*****************************************************************************
 * Matrix4f
 *****************************************************************************/

const Matrix4f& Matrix4f::initFromVectors(const Vector3f& forward, const Vector3f& up, const Vector3f& right) {
    // clang-format off
	set(0, 0, right.getX());   set(0, 1, right.getY());    set(0, 2, right.getZ());   set(0, 3, 0);
	set(1, 0, up.getX());      set(1, 1, up.getY());       set(1, 2, up.getZ());      set(1, 3, 0);
	set(2, 0, forward.getX()); set(2, 1, forward.getY());  set(2, 2, forward.getZ()); set(2, 3, 0);
	set(3, 0, 0);              set(3, 1, 0);               set(3, 2, 0);              set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initOrthographic(float left, float right, float bottom, float top, float zNear, float zFar) {
    // Using Vulkan depth values are between 0 and 1 (Unlike OpenGL which is -1 to 1)
    // clang-format off
	set(0, 0, 2.0f / (right - left)); set(0, 1, 0);                     set(0, 2, 0);                      set(0, 3, -(right + left) / (right - left));
	set(1, 0, 0);                     set(1, 1, 2.0f / (top - bottom)); set(1, 2, 0);                      set(1, 3, -(top + bottom) / (top - bottom));
	set(2, 0, 0);                     set(2, 1, 0);                     set(2, 2, -1.0f / (zFar - zNear)); set(2, 3, -zNear / (zFar - zNear));
	set(3, 0, 0);                     set(3, 1, 0);                     set(3, 2, 0);                      set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initPerspective(float fovY, float aspect, float zNear, float zFar) {
    float scale = tanf(utils_maths::toRadians(fovY / 2));

    // Using Vulkan depth values are between 0 and 1 (Unlike OpenGL which is -1 to 1)
    // clang-format off
	set(0, 0, 1.0f / (aspect * scale)); set(0, 1, 0);            set(0, 2, 0);                     set(0, 3, 0);
	set(1, 0, 0);                       set(1, 1, 1.0f / scale); set(1, 2, 0);                     set(1, 3, 0);
	set(2, 0, 0);                       set(2, 1, 0);            set(2, 2, zFar / (zNear - zFar)); set(2, 3, -(zFar * zNear) / (zFar - zNear));
	set(3, 0, 0);                       set(3, 1, 0);            set(3, 2, -1);                    set(3, 3, 0);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initLookAt(const Vector3f& eye, const Vector3f& centre, const Vector3f& up) {
    // http://stackoverflow.com/questions/21152556/an-inconsistency-in-my-understanding-of-the-glm-lookat-function

    Vector3f forward = (centre - eye).normalise();
    Vector3f u = up.normalised();
    Vector3f side = forward.cross(u).normalise();
    u = side.cross(forward);

    // clang-format off
	set(0, 0, side.getX());     set(0, 1, side.getY());     set(0, 2, side.getZ());     set(0, 3, -side.dot(eye));
	set(1, 0, u.getX());        set(1, 1, u.getY());        set(1, 2, u.getZ());        set(1, 3, -u.dot(eye));
	set(2, 0, -forward.getX()); set(2, 1, -forward.getY()); set(2, 2, -forward.getZ()); set(2, 3, forward.dot(eye));
	set(3, 0, 0);               set(3, 1, 0);               set(3, 2, 0);               set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initTranslation(const Vector2f& translation) {
    // clang-format off
	set(0, 0, 1); set(0, 1, 0); set(0, 2, 0); set(0, 3, translation.getX());
	set(1, 0, 0); set(1, 1, 1); set(1, 2, 0); set(1, 3, translation.getY());
	set(2, 0, 0); set(2, 1, 0); set(2, 2, 1); set(2, 3, 0);
	set(3, 0, 0); set(3, 1, 0); set(3, 2, 0); set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initTranslation(const Vector3f& translation) {
    // clang-format off
	set(0, 0, 1); set(0, 1, 0); set(0, 2, 0); set(0, 3, translation.getX());
	set(1, 0, 0); set(1, 1, 1); set(1, 2, 0); set(1, 3, translation.getY());
	set(2, 0, 0); set(2, 1, 0); set(2, 2, 1); set(2, 3, translation.getZ());
	set(3, 0, 0); set(3, 1, 0); set(3, 2, 0); set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initRotation(float angle, bool x, bool y, bool z) {
    float c = cosf(utils_maths::toRadians(angle));
    float s = sinf(utils_maths::toRadians(angle));

    // clang-format off
	if (x) {
		set(0, 0, 1); set(0, 1, 0); set(0, 2, 0);  set(0, 3, 0);
		set(1, 0, 0); set(1, 1, c); set(1, 2, -s); set(1, 3, 0);
		set(2, 0, 0); set(2, 1, s); set(2, 2, c);  set(2, 3, 0);
		set(3, 0, 0); set(3, 1, 0); set(3, 2, 0);  set(3, 3, 1);
	} else if (y) {
		set(0, 0, c);  set(0, 1, 0); set(0, 2, s); set(0, 3, 0);
		set(1, 0, 0);  set(1, 1, 1); set(1, 2, 0); set(1, 3, 0);
		set(2, 0, -s); set(2, 1, 0); set(2, 2, c); set(2, 3, 0);
		set(3, 0, 0);  set(3, 1, 0); set(3, 2, 0); set(3, 3, 1);
	} else if (z) {
		set(0, 0, c); set(0, 1, -s); set(0, 2, 0); set(0, 3, 0);
		set(1, 0, s); set(1, 1, c);  set(1, 2, 0); set(1, 3, 0);
		set(2, 0, 0); set(2, 1, 0);  set(2, 2, 1); set(2, 3, 0);
		set(3, 0, 0); set(3, 1, 0);  set(3, 2, 0); set(3, 3, 1);
	}
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initScale(const Vector2f& scale) {
    // clang-format off
	set(0, 0, scale.getX()); set(0, 1, 0);            set(0, 2, 0); set(0, 3, 0);
	set(1, 0, 0);            set(1, 1, scale.getY()); set(1, 2, 0); set(1, 3, 0);
	set(2, 0, 0);            set(2, 1, 0);            set(2, 2, 1); set(2, 3, 0);
	set(3, 0, 0);            set(3, 1, 0);            set(3, 2, 0); set(3, 3, 1);
    // clang-format on

    return *this;
}

const Matrix4f& Matrix4f::initScale(const Vector3f& scale) {
    // clang-format off
	set(0, 0, scale.getX()); set(0, 1, 0);            set(0, 2, 0);            set(0, 3, 0);
	set(1, 0, 0);            set(1, 1, scale.getY()); set(1, 2, 0);            set(1, 3, 0);
	set(2, 0, 0);            set(2, 1, 0);            set(2, 2, scale.getZ()); set(2, 3, 0);
	set(3, 0, 0);            set(3, 1, 0);            set(3, 2, 0);            set(3, 3, 1);
    // clang-format on

    return *this;
}