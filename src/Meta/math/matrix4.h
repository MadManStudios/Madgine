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

#include "matrix3.h"
#include "vector4.h"

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
namespace Math {
    /** A 3x3 matrix which can represent rotations around axes.
    @note
        <b>All the code is adapted from the Wild Magic 0.2 Matrix
        library (http://www.geometrictools.com/).</b>
    @par
        The coordinate system is assumed to be <b>right-handed</b>.
*/
    struct META_EXPORT Matrix4 {
        /** Default constructor.
            @note
                It does <b>NOT</b> initialize the matrix for efficiency.
        */
        inline Matrix4() { }
        inline constexpr explicit Matrix4(const Matrix3 &in)
            : m { { in[0][0], in[1][0], in[2][0], 0.0f },
                { in[0][1], in[1][1], in[2][1], 0.0f },
                { in[0][2], in[1][2], in[2][2], 0.0f },
                { 0.0f, 0.0f, 0.0f, 1.0f } }
        {
        }
        inline constexpr Matrix4(const float arr[4][4])
            : m { { arr[0][0], arr[1][0], arr[2][0], arr[3][0] },
                { arr[0][1], arr[1][1], arr[2][1], arr[3][1] },
                { arr[0][2], arr[1][2], arr[2][2], arr[3][2] },
                { arr[0][3], arr[1][3], arr[2][3], arr[3][3] } }
        {
        }
        inline constexpr Matrix4(const Matrix4 &in)
            : m { { in[0][0], in[1][0], in[2][0], in[3][0] },
                { in[0][1], in[1][1], in[2][1], in[3][1] },
                { in[0][2], in[1][2], in[2][2], in[3][2] },
                { in[0][3], in[1][3], in[2][3], in[3][3] } }
        {
        }
        constexpr Matrix4(float fEntry00, float fEntry01, float fEntry02, float fEntry03,
            float fEntry10, float fEntry11, float fEntry12, float fEntry13,
            float fEntry20, float fEntry21, float fEntry22, float fEntry23,
            float fEntry30, float fEntry31, float fEntry32, float fEntry33)
            : m { { fEntry00, fEntry10, fEntry20, fEntry30 },
                { fEntry01, fEntry11, fEntry21, fEntry31 },
                { fEntry02, fEntry12, fEntry22, fEntry32 },
                { fEntry03, fEntry13, fEntry23, fEntry33 } }
        {
        }

        /** Exchange the contents of this matrix with another.
         */
        inline void swap(Matrix4 &other)
        {
            std::swap(m[0][0], other.m[0][0]);
            std::swap(m[0][1], other.m[0][1]);
            std::swap(m[0][2], other.m[0][2]);
            std::swap(m[0][3], other.m[0][3]);
            std::swap(m[1][0], other.m[1][0]);
            std::swap(m[1][1], other.m[1][1]);
            std::swap(m[1][2], other.m[1][2]);
            std::swap(m[1][3], other.m[1][3]);
            std::swap(m[2][0], other.m[2][0]);
            std::swap(m[2][1], other.m[2][1]);
            std::swap(m[2][2], other.m[2][2]);
            std::swap(m[2][3], other.m[2][3]);
            std::swap(m[3][0], other.m[3][0]);
            std::swap(m[3][1], other.m[3][1]);
            std::swap(m[3][2], other.m[3][2]);
            std::swap(m[3][3], other.m[3][3]);
        }

        /// Member access, allows use of construct mat[r][c]
        inline constexpr const Vector4 operator[](size_t iRow) const { return { m[0][iRow], m[1][iRow], m[2][iRow], m[3][iRow] }; }

        struct AccessHelper {
            constexpr float &operator[](size_t iCol) { return m[iCol][row]; }
            size_t row;
            float (&m)[4][4];
        };
        inline constexpr AccessHelper operator[](size_t iRow) { return { iRow, m }; }

        inline float *data()
        {
            return &m[0][0];
        }

        /*inline operator Real* ()
        {
            return (Real*)m[0];
        }*/
        Vector4 GetColumn(size_t iCol) const;
        void SetColumn(size_t iCol, const Vector4 &vec);
        void FromAxes(const Vector4 &xAxis, const Vector4 &yAxis,
            const Vector4 &zAxis, const Vector4 &wAxis);

        /// Assignment and comparison
        inline Matrix4 &operator=(const Matrix4 &rkMatrix)
        {
            memcpy(m, rkMatrix.m, 16 * sizeof(float));
            return *this;
        }

        bool equalsWithEpsilon(const Matrix4 &rkMatrix, float epsilon = floatZeroThreshold) const;

        /** Tests 2 matrices for equality.
         */
        bool operator==(const Matrix4 &rkMatrix) const;

        /** Tests 2 matrices for inequality.
         */
        inline bool operator!=(const Matrix4 &rkMatrix) const
        {
            return !operator==(rkMatrix);
        }

        // arithmetic operations
        /** Matrix addition.
         */
        Matrix4 operator+(const Matrix4 &rkMatrix) const;

        /** Matrix subtraction.
         */
        Matrix4 operator-(const Matrix4 &rkMatrix) const;

        /** Matrix concatenation using '*'.
         */
        Matrix4 operator*(const Matrix4 &rkMatrix) const;
        Matrix4 operator-() const;

        /// Matrix * vector [3x3 * 3x1 = 3x1]
        Vector4 operator*(const Vector4 &rkVector) const;

        /// Vector * matrix [1x3 * 3x3 = 1x3]
        META_EXPORT friend Vector4 operator*(const Vector4 &rkVector,
            const Matrix4 &rkMatrix);

        /// Matrix * scalar
        Matrix4 operator*(float fScalar) const;

        /// Scalar * matrix
        META_EXPORT friend Matrix4 operator*(float fScalar,
            const Matrix4 &rkMatrix);

        // utilities
        Matrix4 Transpose() const;
        void Inverse(Matrix4 &rkInverse) const;
        Matrix4 Inverse() const;
        /*		float Determinant () const;

                /// Singular value decomposition
                void SingularValueDecomposition (Matrix4& rkL, Vector4& rkS,
                    Matrix4& rkR) const;
                void SingularValueComposition (const Matrix4& rkL,
                    const Vector4& rkS, const Matrix4& rkR);

                /// Gram-Schmidt orthonormalization (applied to columns of rotation
           matrix) void Orthonormalize ();

                /// Orthogonal Q, diagonal D, upper triangular U stored as
           (u01,u02,u12) void QDUDecomposition (Matrix4& rkQ, Vector4& rkD, Vector4&
           rkU) const;

                        float SpectralNorm () const;
                        */
        /*       /// Note: Matrix must be orthonormal
               void ToAngleAxis (Vector4& rkAxis, Radian& rfAngle) const;
               inline void ToAngleAxis (Vector4& rkAxis, Degree& rfAngle) const {
                   Radian r;
                   ToAngleAxis ( rkAxis, r );
                   rfAngle = r;
               }
               void FromAngleAxis (const Vector4& rkAxis, const Radian& fRadians);*/

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
               void FromEulerAnglesXYZ (const Radian& fYAngle, const Radian&
           fPAngle, const Radian& fRAngle); void FromEulerAnglesXZY (const Radian&
           fYAngle, const Radian& fPAngle, const Radian& fRAngle); void
           FromEulerAnglesYXZ (const Radian& fYAngle, const Radian& fPAngle, const
           Radian& fRAngle); void FromEulerAnglesYZX (const Radian& fYAngle, const
           Radian& fPAngle, const Radian& fRAngle); void FromEulerAnglesZXY (const
           Radian& fYAngle, const Radian& fPAngle, const Radian& fRAngle); void
           FromEulerAnglesZYX (const Radian& fYAngle, const Radian& fPAngle, const
           Radian& fRAngle);*/
        /// Eigensolver, matrix must be symmetric
        /*       void EigenSolveSymmetric (float afEigenvalue[3],
                   Vector4 akEigenvector[3]) const;

               static void TensorProduct (const Vector4& rkU, const Vector4& rkV,
                   Matrix4& rkProduct);
                               */
        /** Determines if this matrix involves a scaling. */
        /*inline bool hasScale() const
        {
            // check magnitude of column vectors (==local axes)
                        float t = m[0][0] * m[0][0] + m[1][0] * m[1][0] + m[2][0] *
        m[2][0]; if (!Math::RealEqual(t, 1.0, (float)1e-04)) return true; t =
        m[0][1] * m[0][1] + m[1][1] * m[1][1] + m[2][1] * m[2][1]; if
        (!Math::RealEqual(t, 1.0, (float)1e-04)) return true; t = m[0][2] * m[0][2]
        + m[1][2] * m[1][2] + m[2][2] * m[2][2]; if (!Math::RealEqual(t, 1.0,
        (float)1e-04)) return true;

            return false;
        }*/

        Matrix3 ToMat3() const
        {
            return Matrix3(m[0][0], m[1][0], m[2][0], m[0][1], m[1][1], m[2][1],
                m[0][2], m[1][2], m[2][2]);
        }

        /** Function for writing to a stream.
         */
        inline META_EXPORT friend std::ostream &
        operator<<(std::ostream &o, const Matrix4 &mat)
        {
            o << "Matrix4(" << mat[0][0] << ", " << mat[1][0] << ", " << mat[2][0]
              << ", " << mat[3][0] << ", " << mat[0][1] << ", " << mat[1][1] << ", "
              << mat[2][1] << ", " << mat[3][1] << ", " << mat[0][2] << ", "
              << mat[1][2] << ", " << mat[2][2] << ", " << mat[3][2] << ", "
              << mat[0][3] << ", " << mat[1][3] << ", " << mat[2][3] << ", "
              << mat[3][3] << ")";
            return o;
        }

        inline META_EXPORT friend std::istream &operator>>(std::istream &in, Matrix4 &mat)
        {
            char c;
            in >> c;
            if (c != '[') {
                in.setstate(std::ios_base::failbit);
                return in;
            }
            for (int x = 0; x < 4; ++x) {
                for (int y = 0; y < 4; ++y) {
                    in >> mat[x][y];
                    in >> c;
                    if (x != 3 || y != 3) {
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

        // static const float EPSILON;
        static constexpr float ZERO[4][4] { { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
        static constexpr float IDENTITY[4][4] { { 1, 0, 0, 0 }, { 0, 1, 0, 0 }, { 0, 0, 1, 0 }, { 0, 0, 0, 1 } };

    protected:
        // support for eigensolver
        /* void Tridiagonal (float afDiag[3], float afSubDiag[3]);
         bool QLAlgorithm (float afDiag[3], float afSubDiag[3]);

         // support for singular value decomposition
         static const float msSvdEpsilon;
         static const unsigned int msSvdMaxIterations;
         static void Bidiagonalize (Matrix4& kA, Matrix4& kL,
             Matrix4& kR);
         static void GolubKahanStep (Matrix4& kA, Matrix4& kL,
             Matrix4& kR);

         // support for spectral norm
         static float MaxCubicRoot (float afCoeff[3]);
                 */
        float m[4][4];
    };

} // namespace Engine
}