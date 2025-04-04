/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#pragma once

#include "vector3.h"

// NB All code adapted from Wild Magic 0.2 Matrix math (free source code)
// http://www.geometrictools.com/

// NOTE.  The (x,y,z) coordinate system is assumed to be right-handed.
// Coordinate axis rotation matrices are of the form
//   RX =    1       0       0
//           0     cos(t) -sin(t)
//           0     sin(t)  cos(t)
// where t > 0 indicates a counterclockwise rotation in the yz-plane
//   RY =  cos(t)    0     sin(t)
//           0       1       0
//        -sin(t)    0     cos(t)
// where t > 0 indicates a counterclockwise rotation in the zx-plane
//   RZ =  cos(t) -sin(t)    0
//         sin(t)  cos(t)    0
//           0       0       1
// where t > 0 indicates a counterclockwise rotation in the xy-plane.

namespace Engine {
/** A 3x3 matrix which can represent rotations around axes.
        @note
            <b>All the code is adapted from the Wild Magic 0.2 Matrix
            library (http://www.geometrictools.com/).</b>
        @par
            The coordinate system is assumed to be <b>right-handed</b>.
    */
struct META_EXPORT Matrix3 {
    /** Default constructor.
            @note
                It does <b>NOT</b> initialize the matrix for efficiency.
        */
    inline Matrix3() { }
    inline constexpr explicit Matrix3(const float arr[3][3])
        : m00 { arr[0][0] }
        , m01 { arr[1][0] }
        , m02 { arr[2][0] }
        , m10 { arr[0][1] }
        , m11 { arr[1][1] }
        , m12 { arr[2][1] }
        , m20 { arr[0][2] }
        , m21 { arr[1][2] }
        , m22 { arr[2][2] }
    {
    }
    inline constexpr Matrix3(const Matrix3 &rkMatrix) = default;

    constexpr Matrix3(float fEntry00, float fEntry01, float fEntry02,
        float fEntry10, float fEntry11, float fEntry12,
        float fEntry20, float fEntry21, float fEntry22)
        : m00 { fEntry00 }
        , m01 { fEntry10 }
        , m02 { fEntry20 }
        , m10 { fEntry01 }
        , m11 { fEntry11 }
        , m12 { fEntry21 }
        , m20 { fEntry02 }
        , m21 { fEntry12 }
        , m22 { fEntry22 }
    {
    }

    /** Exchange the contents of this matrix with another. 
        */
    inline void swap(Matrix3 &other)
    {
        std::swap(m00, other.m00);
        std::swap(m01, other.m01);
        std::swap(m02, other.m02);
        std::swap(m10, other.m10);
        std::swap(m11, other.m11);
        std::swap(m12, other.m12);
        std::swap(m20, other.m20);
        std::swap(m21, other.m21);
        std::swap(m22, other.m22);
    }

    struct AccessHelper {
        constexpr float &operator[](size_t iCol) { return m[3*iCol+row]; }
        size_t row;
        float *m;
    };

    struct const_AccessHelper {
        constexpr const float &operator[](size_t iCol) { return m[3*iCol+row]; }
        size_t row;
        const float *m;
    };

    inline constexpr AccessHelper operator[](size_t iRow)
    {
        return { iRow, &m00 };
    }

    inline constexpr const_AccessHelper operator[](size_t iRow) const
    {
        return { iRow, &m00 };
    }

    inline float *data()
    {
        return &m00;
    }

    /*inline operator Real* ()
        {
            return (Real*)m[0];
        }*/
    Vector3 GetColumn(size_t iCol) const;
    void SetColumn(size_t iCol, const Vector3 &vec);
    void FromAxes(const Vector3 &xAxis, const Vector3 &yAxis, const Vector3 &zAxis);

    /// Assignment and comparison
    inline Matrix3 &operator=(const Matrix3 &rkMatrix) = default;

    /** Tests 2 matrices for equality.
         */
    bool operator==(const Matrix3 &rkMatrix) const;

    /** Tests 2 matrices for inequality.
         */
    inline bool operator!=(const Matrix3 &rkMatrix) const
    {
        return !operator==(rkMatrix);
    }

    bool equalsWithEpsilon(const Matrix3 &rkMatrix, float epsilon = floatZeroThreshold);

    // arithmetic operations
    /** Matrix addition.
         */
    constexpr Matrix3 &operator+=(const Matrix3 &rkMatrix)
    {
        m00 += rkMatrix.m00;
        m01 += rkMatrix.m01;
        m02 += rkMatrix.m02;
        m10 += rkMatrix.m10;
        m11 += rkMatrix.m11;
        m12 += rkMatrix.m12;
        m20 += rkMatrix.m20;
        m21 += rkMatrix.m21;
        m22 += rkMatrix.m22;
        return *this;
    }
    constexpr Matrix3 operator+(const Matrix3 &rkMatrix) const
    {
        Matrix3 kSum = *this;
        kSum += rkMatrix;
        return kSum;
    }

    /** Matrix subtraction.
         */
    Matrix3 &operator-=(const Matrix3 &rkMatrix);
    Matrix3 operator-(const Matrix3 &rkMatrix) const;

    /** Matrix concatenation using '*'.
         */
    Matrix3 operator*(const Matrix3 &rkMatrix) const;
    Matrix3 operator-() const;

    /// Matrix * vector [3x3 * 3x1 = 3x1]
    Vector3 operator*(const Vector3 &rkVector) const;

    /// Vector * matrix [1x3 * 3x3 = 1x3]
    META_EXPORT friend Vector3 operator*(const Vector3 &rkVector,
        const Matrix3 &rkMatrix);

    /// Matrix * scalar
    Matrix3 operator*(float fScalar) const;

    /// Scalar * matrix
    META_EXPORT friend Matrix3 operator*(float fScalar, const Matrix3 &rkMatrix);

    // utilities
    Matrix3 Transpose() const;
    bool Inverse(Matrix3 &rkInverse, float fTolerance = 1e-06) const;
    Matrix3 Inverse(float fTolerance = 1e-06) const;
    float Determinant() const;

    /// Singular value decomposition
    /*    void SingularValueDecomposition(Matrix3 &rkL, Vector3 &rkS,
        Matrix3 &rkR) const;
    void SingularValueComposition(const Matrix3 &rkL,
        const Vector3 &rkS, const Matrix3 &rkR);

    /// Gram-Schmidt orthonormalization (applied to columns of rotation matrix)
    void Orthonormalize();

    /// Orthogonal Q, diagonal D, upper triangular U stored as (u01,u02,u12)
    void QDUDecomposition(Matrix3 &rkQ, Vector3 &rkD,
        Vector3 &rkU) const;

    float SpectralNorm() const;*/

    /*       /// Note: Matrix must be orthonormal
        void ToAngleAxis (Vector3& rkAxis, Radian& rfAngle) const;
        inline void ToAngleAxis (Vector3& rkAxis, Degree& rfAngle) const {
            Radian r;
            ToAngleAxis ( rkAxis, r );
            rfAngle = r;
        }
        void FromAngleAxis (const Vector3& rkAxis, const Radian& fRadians);
		*/
    /** The matrix must be orthonormal.  The decomposition is yaw*pitch*roll
            where yaw is rotation about the Up vector, pitch is rotation about the
            Right axis, and roll is rotation about the Direction axis. */
    /*       bool ToEulerAnglesXYZ (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        bool ToEulerAnglesXZY (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        bool ToEulerAnglesYXZ (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        bool ToEulerAnglesYZX (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        bool ToEulerAnglesZXY (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        bool ToEulerAnglesZYX (Radian& rfYAngle, Radian& rfPAngle,
            Radian& rfRAngle) const;
        void FromEulerAnglesXYZ (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesXZY (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesYXZ (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesYZX (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesZXY (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);
        void FromEulerAnglesZYX (const Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle);*/
    /// Eigensolver, matrix must be symmetric
    void EigenSolveSymmetric(float afEigenvalue[3],
        Vector3 akEigenvector[3]) const;

    static void TensorProduct(const Vector3 &rkU, const Vector3 &rkV,
        Matrix3 &rkProduct);

    /** Determines if this matrix involves a scaling. */
    inline bool hasScale() const
    {
        // check magnitude of column vectors (==local axes)
        float t = m00 * m00 + m01 * m01 + m02 * m02;
        if (!isZero(t - 1.0f))
            return true;
        t = m10 * m10 + m11 * m11 + m12 * m12;
        if (!isZero(t - 1.0f))
            return true;
        t = m20 * m20 + m21 * m21 + m22 * m22;
        if (!isZero(t - 1.0f))
            return true;

        return false;
    }

    /** Function for writing to a stream.
        */
    inline META_EXPORT friend std::ostream &operator<<(std::ostream &out, const Matrix3 &mat)
    {
        return out << "[" << mat[0][0] << ", " << mat[0][1] << ", " << mat[0][2] << ", "
                   << mat[1][0] << ", " << mat[1][1] << ", " << mat[1][2] << ", "
                   << mat[2][0] << ", " << mat[2][1] << ", " << mat[2][2] << "]";
    }

    inline META_EXPORT friend std::istream &operator>>(std::istream &in, Matrix3 &mat)
    {
        char c;
        in >> c;
        if (c != '[') {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        for (int x = 0; x < 3; ++x) {
            for (int y = 0; y < 3; ++y) {
                in >> mat[x][y];
                in >> c;
                if (x != 2 || y != 2) {
                    if (c != ',') {
                        in.setstate(std::ios_base::failbit);
                        return in;
                    }
                } else {
                    if (c != ']') {
                        in.setstate(std::ios_base::failbit);
                        return in;
                    }
                }
            }
        }
        return in;
    }

    static const Matrix3 ZERO;
    static const Matrix3 IDENTITY;

    // support for eigensolver
    /*    void Tridiagonal(float afDiag[3], float afSubDiag[3]);
    bool QLAlgorithm(float afDiag[3], float afSubDiag[3]);

    // support for singular value decomposition
    static const float msSvdEpsilon;
    static const unsigned int msSvdMaxIterations;
    static void Bidiagonalize(Matrix3 &kA, Matrix3 &kL,
        Matrix3 &kR);
    static void GolubKahanStep(Matrix3 &kA, Matrix3 &kL,
        Matrix3 &kR);

    // support for spectral norm
    static float MaxCubicRoot(float afCoeff[3]);*/

    float m00, m01, m02, m10, m11, m12, m20, m21, m22;

    // for faster access
    friend struct Matrix4;
};


constexpr Matrix3 lerp(const Matrix3 &m1, const Matrix3 &m2, float ratio)
{
    return (1.0f - ratio) * m1 + ratio * m2;
}


}
