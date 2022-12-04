#pragma once

#include <initializer_list>

#include "Quaternion.h"
#include "Vector.h"

/*****************************************************************************
 * Generalised version of a Matrix
 *****************************************************************************/

/* T - type of value, N - dimension of matrix (assumes N x N) */
template <typename T, unsigned int N>
class Matrix {
protected:
    /* Always initialise member variables */
    T values[N][N] = {};

public:
    /* Constructor */
    Matrix() {}

    /* Constructor taking an initial value (specefied in column major order) */
    Matrix(const std::initializer_list<std::initializer_list<T>>& values) {
        // Ensure correct size
        if (values.size() != N)
            throw std::invalid_argument("Incorrect number of elements");
        // Go through columns and copy
        unsigned int colIdx = 0;
        for (auto& col : values)
            std::copy(col.begin(), col.end(), this->values[colIdx++]);
    }

    /* Operations for obtaining/assigning values */
    inline T& operator[](unsigned int col) { return this->values[col]; }
    inline const T& operator[](unsigned int col) const { return this->values[col]; }

    /* Get and set functions allow row major access */
    inline void set(unsigned int row, unsigned int col, const T& value) { values[col][row] = value; }
    inline const T get(unsigned int row, unsigned int col) const { return values[col][row]; }

    /* Adds another matrix to this one and returns the result */
    inline Matrix<T, N> operator+(const Matrix<T, N>& other) const {
        Matrix<T, N> result;
        for (unsigned int col = 0; col < N; ++col)
            for (unsigned int row = 0; row < N; ++row)
                result.set(row, col, get(row, col) + other.get(row, col));
        return result;
    }

    /* Subtracts a matrix from this one and returns the result */
    inline Matrix<T, N> operator-(const Matrix<T, N>& other) const {
        Matrix<T, N> result;
        for (unsigned int col = 0; col < N; ++col)
            for (unsigned int row = 0; row < N; ++row)
                result.set(row, col, get(row, col) - other.get(row, col));
        return result;
    }

    /* Returns the result of element-wise multiplication by a scalar  */
    inline Matrix<T, N> operator*(const T& value) const {
        Matrix<T, N> result;
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                result.set(row, col, get(row, col) * value);
        }
        return result;
    }

    /* Returns the result of element-wise division by a scalar  */
    inline Matrix<T, N> operator/(const T& value) const {
        Matrix<T, N> result;
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                result.set(row, col, get(row, col) / value);
        }
        return result;
    }

    /* Adds another matrix to this one and returns the result */
    inline Matrix<T, N>& operator+=(const Matrix<T, N>& other) {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                set(row, col, get(row, col) + other.get(row, col));
        }
        return *this;
    }

    /* Subtracts a matrix from this one and returns the result */
    inline Matrix<T, N>& operator-=(const Matrix<T, N>& other) {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                set(row, col, get(row, col) - other.get(row, col));
        }
        return *this;
    }

    /* Element-wise multiplication by a scalar  */
    inline Matrix<T, N>& operator*=(const T& value) {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                set(row, col, get(row, col) * value);
        }
        return *this;
    }

    /* Element-wise division by a scalar  */
    inline Matrix<T, N>& operator/=(const T& value) {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                set(row, col, get(row, col) / value);
        }
        return *this;
    }

    /* Returns the result of multiplying this matrix by a vector */
    inline Vector<T, N> operator*(const Vector<T, N>& other) const {
        Vector<T, N> result;
        for (unsigned int row = 0; row < N; ++row) {
            for (unsigned int col = 0; col < N; ++col)
                result[row] += other[col] * get(row, col);
        }
        return result;
    }

    /* Returns the result of multiplying this matrix by another */
    inline Matrix<T, N> operator*(const Matrix<T, N>& other) const {
        Matrix<T, N> result;
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row) {
                for (unsigned int i = 0; i < N; ++i)
                    result.set(row, col, result.get(row, col) + (get(row, i) * other.get(i, col)));
            }
        }
        return result;
    }

    /* Multiplies this matrix by another */
    inline Matrix<T, N>& operator*=(const Matrix<T, N>& other) {
        Matrix<T, N> result = (*this) * other;

        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                set(row, col, result.get(row, col));
        }
        return *this;
    }

    /* Compares this vector to another one and returns whether they are equal
       or not */
    inline bool operator==(const Matrix<T, N>& other) const {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row) {
                if (other.get(row, col) != get(row, col))
                    return false;
            }
        }
        return true;
    }

    /* Returns the transpose of this matrix */
    inline Matrix<T, N> transpose() const {
        Matrix<T, N> result;
        // Flip rows and columns
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row)
                result.set(row, col, get(col, row));
        }
        return result;
    }

    /* Makes this matrix the identity */
    inline void setIdentity() {
        for (unsigned int col = 0; col < N; ++col) {
            for (unsigned int row = 0; row < N; ++row) {
                if (col == row)
                    set(row, col, 1);
                else
                    set(row, col, 0);
            }
        }
    }

    /* Intialises the identity and returns itself */
    inline Matrix<T, N>& initIdentity() {
        setIdentity();
        return *this;
    }

    /* Converts this matrix to a string format (row major order) */
    std::string toString() const {
        std::string result = "";
        for (unsigned int row = 0; row < N; ++row) {
            for (unsigned int col = 0; col < N; ++col) {
                result += utils_string::str(this->values[col][row]);
                if (col != N - 1)
                    result += ",";
            }
            if (row != N - 1)
                result += "\n";
        }
        return result;
    }

    /* Returns the number of elements in this matrix */
    inline int getNumElements() { return N * N; }

    /* Returns the size of this matrix in bytes */
    inline int getSize() { return N * N * sizeof(T); }
};

/*****************************************************************************
 * Slightly more specific versions of Matrice
 *****************************************************************************/

/* 2x2 matrix */
template <typename T>
class Matrix2 : public Matrix<T, 2> {
public:
    /* Various constructors */
    Matrix2() {}
    Matrix2(const std::initializer_list<std::initializer_list<T>>& values) : Matrix<T, 2>(values) {}
    Matrix2(const Matrix<T, 2>& base) : Matrix<T, 2>(base) {}
};

/* 3x3 matrix */
template <typename T>
class Matrix3 : public Matrix<T, 3> {
public:
    /* Various constructors */
    Matrix3() {}
    Matrix3(const std::initializer_list<std::initializer_list<T>>& values) : Matrix<T, 3>(values) {}
    Matrix3(const Matrix<T, 3>& base) : Matrix<T, 3>(base) {}
};

/* 4x4 matrix */
template <typename T>
class Matrix4 : public Matrix<T, 4> {
public:
    /* Various constructors */
    Matrix4() {}
    Matrix4(const std::initializer_list<std::initializer_list<T>>& values) : Matrix<T, 4>(values) {}
    Matrix4(const Matrix<T, 4>& base) : Matrix<T, 4>(base) {}

    /* Converts to a 3x3 matrix (Ignores extra values) */
    inline Matrix3<T> to3x3() {
        Matrix3<T> matrix;

        // clang-format off
		matrix.set(0, 0, this->get(0, 0)); matrix.set(0, 1, this->get(0, 1)); matrix.set(0, 2, this->get(0, 2));
		matrix.set(1, 0, this->get(1, 0)); matrix.set(1, 1, this->get(1, 1)); matrix.set(1, 2, this->get(1, 2));
		matrix.set(2, 0, this->get(2, 0)); matrix.set(2, 1, this->get(2, 1)); matrix.set(2, 2, this->get(2, 2));
        // clang-format on

        return matrix;
    }
};

/*****************************************************************************
 * Specific versions of Matrix
 *****************************************************************************/

using Matrix2i = Matrix2<int>;
using Matrix2f = Matrix2<float>;
using Matrix2d = Matrix2<double>;

using Matrix3i = Matrix3<int>;
class Matrix3f : public Matrix3<float> {
public:
    /* Various constructors */
    Matrix3f() {}
    Matrix3f(const std::initializer_list<std::initializer_list<float>>& values) : Matrix3<float>(values) {}

    Matrix3f(const Matrix3<float>& base) : Matrix3<float>(base) {}
    Matrix3f(const Matrix<float, 3>& base) : Matrix3<float>(base) {}

    /* Inverts this matrix - if singular returned matrix will be a matrix of zeros */
    Matrix3f inverse() const;
};
using Matrix3d = Matrix3<double>;

using Matrix4i = Matrix4<int>;

class Matrix4f : public Matrix4<float> {
public:
    /* Various constructors */
    Matrix4f() {}
    Matrix4f(const std::initializer_list<std::initializer_list<float>>& values) : Matrix4<float>(values) {}

    Matrix4f(const Matrix4<float>& base) : Matrix4<float>(base) {}
    Matrix4f(const Matrix<float, 4>& base) : Matrix4<float>(base) {}

    /* Initialises this matrix from 3 axis vectors */
    const Matrix4f& initFromVectors(const Vector3f& forward, const Vector3f& up, const Vector3f& right);

    /* Initialises an orthographic view matrix */
    const Matrix4f& initOrthographic(float left, float right, float bottom, float top, float zNear, float zFar);

    /* Initialises an perspective view matrix */
    const Matrix4f& initPerspective(float fovY, float aspect, float zNear, float zFar);

    /* Initialises this matrix looking at a particular point */
    const Matrix4f& initLookAt(const Vector3f& eye, const Vector3f& centre, const Vector3f& up);

    /* Initialises this matrix as a translation matrix */
    const Matrix4f& initTranslation(const Vector2f& translation);
    const Matrix4f& initTranslation(const Vector3f& translation);

    /* Initialises this matrix as a rotation matrix along a particular axis */
    const Matrix4f& initRotation(float angle, bool x, bool y, bool z);

    /* Initialises this matrix as a scale matrix */
    const Matrix4f& initScale(const Vector2f& scale);
    const Matrix4f& initScale(const Vector3f& scale);

    /* Translates this matrix */
    inline void translate(const Vector2f& translation) { (*this) *= Matrix4f().initTranslation(translation); }
    inline void translate(const Vector3f& translation) { (*this) *= Matrix4f().initTranslation(translation); }

    /* Rotates this matrix along a given axis */
    inline void rotate(float angle, bool x, bool y, bool z) { (*this) *= Matrix4f().initRotation(angle, x, y, z); }

    /* Rotates this matrix along the z axes by a the given amount */
    inline void rotate(float angle) {
        rotate(angle, 0, 0, 1);
    }

    /* Rotates this matrix along the x, y and z axes by the given amounts */
    inline void rotate(const Vector3f& angles) {
        rotate(angles.getX(), 1, 0, 0);
        rotate(angles.getY(), 0, 1, 0);
        rotate(angles.getZ(), 0, 0, 1);
    }

    /* Rotates this matrix using a quaternion */
    inline void rotate(const Quaternion& quaternion) { (*this) *= quaternion.toMatrix(); }

    /* Scales this matrix */
    inline void scale(const Vector2f& scale) { (*this) *= Matrix4f().initScale(scale); }
    inline void scale(const Vector3f& scale) { (*this) *= Matrix4f().initScale(scale); }

    /* Transforms this matrix given a translation, rotation and scale */
    inline void transform(const Vector2f translation, float rotation, const Vector2f& scale) {
        translate(translation);
        rotate(rotation);
        this->scale(scale);
    }

    inline void transform(const Vector3f translation, const Vector3f& eulerAngles, const Vector2f& scale) {
        translate(translation);
        rotate(eulerAngles);
        this->scale(scale);
    }
};

using Matrix4d = Matrix4<double>;