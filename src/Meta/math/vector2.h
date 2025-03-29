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

#include "vector2i.h"
#include "common.h"

namespace Engine {
/** Standard 2-dimensional vector.
	@remarks
	A direction in 2D space represented as distances along the 2
	orthogonal axes (x, y). Note that positions, directions and
	scaling factors can be represented by a vector, depending on how
	you interpret the values.
	*/
struct META_EXPORT Vector2 {
    float x, y;

public:
    /** Default constructor.
		@note
		It does <b>NOT</b> initialize the vector for efficiency.
		*/
    Vector2()
    {
    }

    constexpr Vector2(const float fX, const float fY)
        : x(fX)
        , y(fY)
    {
    }

    explicit Vector2(const float scaler)
        : x(scaler)
        , y(scaler)
    {
    }

    Vector2(const float afCoordinate[2])
        : x(afCoordinate[0])
        , y(afCoordinate[1])
    {
    }

    explicit Vector2(const Vector2i &v)
        : x(static_cast<float>(v.x))
        , y(static_cast<float>(v.y))
    {
    }

    explicit Vector2(const int afCoordinate[2])
    {
        x = static_cast<float>(afCoordinate[0]);
        y = static_cast<float>(afCoordinate[1]);
    }

    explicit Vector2(float *const r)
        : x(r[0])
        , y(r[1])
    {
    }

    /** Exchange the contents of this vector with another.
		*/
    void swap(Vector2 &other) noexcept
    {
        std::swap(x, other.x);
        std::swap(y, other.y);
    }

    float operator[](const size_t i) const
    {
        assert(i < 2);

        return *(&x + i);
    }

    float &operator[](const size_t i)
    {
        assert(i < 2);

        return *(&x + i);
    }

    /// Pointer accessor for direct copying
    float *ptr()
    {
        return &x;
    }

    /// Pointer accessor for direct copying
    const float *ptr() const
    {
        return &x;
    }

    /** Assigns the value of the other vector.
		@param
		rkVector The other vector
		*/
    constexpr Vector2 &operator=(const Vector2 &rkVector)
    {
        x = rkVector.x;
        y = rkVector.y;

        return *this;
    }

    Vector2 &operator=(const float fScalar)
    {
        x = fScalar;
        y = fScalar;

        return *this;
    }

    bool operator==(const Vector2 &rkVector) const
    {
        return x == rkVector.x && y == rkVector.y;
    }

    bool operator!=(const Vector2 &rkVector) const
    {
        return x != rkVector.x || y != rkVector.y;
    }

    // arithmetic operations
    constexpr Vector2 operator+(const Vector2 &rkVector) const
    {
        return Vector2(
            x + rkVector.x,
            y + rkVector.y);
    }

    Vector2 operator-(const Vector2 &rkVector) const
    {
        return Vector2(
            x - rkVector.x,
            y - rkVector.y);
    }

    Vector2 operator-(const Vector2i &other) const
    {
        return {
            x - other.x, y - other.y
        };
    }

    constexpr Vector2 operator*(const float fScalar) const
    {
        return Vector2(
            x * fScalar,
            y * fScalar);
    }

    constexpr Vector2 operator*(const Vector2 &rhs) const
    {
        return Vector2(
            x * rhs.x,
            y * rhs.y);
    }

    Vector2 operator/(const float fScalar) const
    {
        assert(fScalar != 0.0);

        float fInv = 1.0f / fScalar;

        return Vector2(
            x * fInv,
            y * fInv);
    }

    Vector2 operator/(const Vector2 &rhs) const
    {
        return Vector2(
            x / rhs.x,
            y / rhs.y);
    }

    const Vector2 &operator+() const
    {
        return *this;
    }

    Vector2 operator-() const
    {
        return Vector2(-x, -y);
    }

    // overloaded operators to help Vector2
    friend Vector2 operator*(const float fScalar, const Vector2 &rkVector)
    {
        return Vector2(
            fScalar * rkVector.x,
            fScalar * rkVector.y);
    }

    friend Vector2 operator/(const float fScalar, const Vector2 &rkVector)
    {
        return Vector2(
            fScalar / rkVector.x,
            fScalar / rkVector.y);
    }

    friend constexpr Vector2 operator+(const Vector2 &lhs, const float rhs)
    {
        return Vector2(
            lhs.x + rhs,
            lhs.y + rhs);
    }

    friend constexpr Vector2 operator+(const float lhs, const Vector2 &rhs)
    {
        return Vector2(
            lhs + rhs.x,
            lhs + rhs.y);
    }

    friend Vector2 operator-(const Vector2 &lhs, const float rhs)
    {
        return Vector2(
            lhs.x - rhs,
            lhs.y - rhs);
    }

    friend Vector2 operator-(const float lhs, const Vector2 &rhs)
    {
        return Vector2(
            lhs - rhs.x,
            lhs - rhs.y);
    }

    // arithmetic updates
    Vector2 &operator+=(const Vector2 &rkVector)
    {
        x += rkVector.x;
        y += rkVector.y;

        return *this;
    }

    Vector2 &operator+=(const float fScaler)
    {
        x += fScaler;
        y += fScaler;

        return *this;
    }

    Vector2 &operator-=(const Vector2 &rkVector)
    {
        x -= rkVector.x;
        y -= rkVector.y;

        return *this;
    }

    Vector2 &operator-=(const float fScaler)
    {
        x -= fScaler;
        y -= fScaler;

        return *this;
    }

    Vector2 &operator*=(const float fScalar)
    {
        x *= fScalar;
        y *= fScalar;

        return *this;
    }

    Vector2 &operator*=(const Vector2 &rkVector)
    {
        x *= rkVector.x;
        y *= rkVector.y;

        return *this;
    }

    Vector2 &operator/=(const float fScalar)
    {
        assert(fScalar != 0.0);

        float fInv = 1.0f / fScalar;

        x *= fInv;
        y *= fInv;

        return *this;
    }

    Vector2 &operator/=(const Vector2 &rkVector)
    {
        x /= rkVector.x;
        y /= rkVector.y;

        return *this;
    }

    /// @copydoc Vector3::length
    float length() const
    {
        return sqrtf(x * x + y * y);
    }

    /// @copydoc Vector3::squaredLength
    float squaredLength() const
    {
        return x * x + y * y;
    }

    /// @copydoc Vector3::distance
    float distance(const Vector2 &rhs) const
    {
        return (*this - rhs).length();
    }

    /// @copydoc Vector3::squaredDistance
    float squaredDistance(const Vector2 &rhs) const
    {
        return (*this - rhs).squaredLength();
    }

    /// @copydoc Vector3::dotProduct
    float dotProduct(const Vector2 &vec) const
    {
        return x * vec.x + y * vec.y;
    }

    /// @copydoc Vector3::normalise
    constexpr float normalize()
    {
        float fLength = sqrtf(x * x + y * y);

        // Will also work for zero-sized vectors, but will change nothing
        // We're not using epsilons because we don't need to.
        // Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
        if (fLength > float(0.0f)) {
            float fInvLength = 1.0f / fLength;
            x *= fInvLength;
            y *= fInvLength;
        }

        return fLength;
    }

    /** Returns a vector at a point half way between this and the passed
		in vector.
		*/
    Vector2 midPoint(const Vector2 &vec) const
    {
        return Vector2(
            (x + vec.x) * 0.5f,
            (y + vec.y) * 0.5f);
    }

    /** Returns true if the vector's scalar components are all greater
		that the ones of the vector it is compared against.
		*/
    /* bool operator<(const Vector2 &rhs) const
    {
        if (x < rhs.x && y < rhs.y)
            return true;
        return false;
    }*/

    /** Returns true if the vector's scalar components are all smaller
		that the ones of the vector it is compared against.
		*/
    /* bool operator>(const Vector2 &rhs) const
    {
        if (x > rhs.x && y > rhs.y)
            return true;
        return false;
    }*/

    /** Sets this vector's components to the minimum of its own and the
		ones of the passed in vector.
		@remarks
		'Minimum' in this case means the combination of the lowest
		value of x, y and z from both vectors. Lowest is taken just
		numerically, not magnitude, so -1 < 0.
		*/
    void makeFloor(const Vector2 &cmp)
    {
        if (cmp.x < x)
            x = cmp.x;
        if (cmp.y < y)
            y = cmp.y;
    }

    /** Sets this vector's components to the maximum of its own and the
		ones of the passed in vector.
		@remarks
		'Maximum' in this case means the combination of the highest
		value of x, y and z from both vectors. Highest is taken just
		numerically, not magnitude, so 1 > -3.
		*/
    void makeCeil(const Vector2 &cmp)
    {
        if (cmp.x > x)
            x = cmp.x;
        if (cmp.y > y)
            y = cmp.y;
    }

    /** Generates a vector perpendicular to this vector (eg an 'up' vector).
		@remarks
		This method will return a vector which is perpendicular to this
		vector. There are an infinite number of possibilities but this
		method will guarantee to generate one of them. If you need more
		control you should use the Quaternion class.
		*/
    Vector2 perpendicular() const
    {
        return Vector2(-y, x);
    }

    /** Calculates the 2 dimensional cross-product of 2 vectors, which results
		in a single floating point value which is 2 times the area of the triangle.
		*/
    float crossProduct(const Vector2 &rkVector) const
    {
        return x * rkVector.y - y * rkVector.x;
    }

    /* Generates a new random vector which deviates from this vector by a
		given angle in a random direction.
		@remarks
		This method assumes that the random number generator has already
		been seeded appropriately.
		@param angle
		The angle at which to deviate in radians
		@return
		A random vector which deviates from this vector by angle. This
		vector will not be normalised, normalise it if you wish
		afterwards.
		*/
    /*inline Vector2 randomDeviant(Radian angle) const
		{
			angle *= Math::RangeRandom(-1, 1);
			float cosa = Math::Cos(angle);
			float sina = Math::Sin(angle);
			return Vector2(cosa * x - sina * y,
				sina * x + cosa * y);
		}*/

    /** Returns true if this vector is zero length. */
    bool isZeroLength() const
    {
        float sqlen = x * x + y * y;
        return sqlen < 1e-06 * 1e-06;
    }

    /** As normalise, except that this vector is unaffected and the
		normalised vector is returned as a copy. */
    Vector2 normalizedCopy() const
    {
        Vector2 ret = *this;
        ret.normalize();
        return ret;
    }

    /** Calculates a reflection vector to the plane with the given normal .
		@remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
		*/
    Vector2 reflect(const Vector2 &normal) const
    {
        return Vector2(*this - 2 * this->dotProduct(normal) * normal);
    }

    /// Check whether this vector contains valid values
    /*inline bool isNaN() const
		{
			return Math::isNaN(x) || Math::isNaN(y);
		}*/

    /*  Gets the angle between 2 vectors.
		@remarks
		Vectors do not have to be unit-length but must represent directions.
		*/
    /*inline Ogre::Radian angleBetween(const Ogre::Vector2& other) const
		{
			Ogre::float lenProduct = length() * other.length();
			// Divide by zero check
			if (lenProduct < 1e-6f)
				lenProduct = 1e-6f;

			Ogre::float f = dotProduct(other) / lenProduct;

			f = Ogre::Math::Clamp(f, (Ogre::float) - 1.0, (Ogre::float)1.0);
			return Ogre::Math::ACos(f);
		}*/

    /*  Gets the oriented angle between 2 vectors.
		@remarks
		Vectors do not have to be unit-length but must represent directions.
		The angle is comprised between 0 and 2 PI.
		*/
    /*inline Ogre::Radian angleTo(const Ogre::Vector2& other) const
		{
			Ogre::Radian angle = angleBetween(other);

			if (crossProduct(other)<0)
				angle = (Ogre::Radian)Ogre::Math::TWO_PI - angle;

			return angle;
		}*/

    // special points
    static const constexpr float ZERO[] { 0, 0 };
    static const constexpr float UNIT_X[] { 1, 0 };
    static const constexpr float UNIT_Y[] { 0, 1 };
    static const constexpr float NEGATIVE_UNIT_X[] { -1, 0 };
    static const constexpr float NEGATIVE_UNIT_Y[] { 0, -1 };
    static const constexpr float UNIT_SCALE[] { 1, 1 };

    /** Function for writing to a stream.
		*/
    friend std::ostream &operator<<(std::ostream &o, const Vector2 &v)
    {
        o << "[" << v.x << ", " << v.y << "]";
        return o;
    }

    friend std::istream &operator>>(std::istream &in, Vector2 &v)
    {
        char c;
        in >> c;
        if (c != '[') {
            in.setstate(std::ios_base::failbit);
            return in;
        }
        for (int i = 0; i < 2; ++i) {
            in >> v[i];
            in >> c;
            char expectedChar = (i == 1) ? ']' : ',';
            if (c != expectedChar) {
                in.setstate(std::ios_base::failbit);
                return in;
            }
        }
        return in;
    }

    constexpr Vector2i floor() const
    {
        return { static_cast<int>(x), static_cast<int>(y) };
    }
};

struct NormalizedVector2 : Vector2 {

    constexpr NormalizedVector2(float x, float y)
        : Vector2(x, y)
    {
        normalize();
    }

    constexpr NormalizedVector2(const Vector2 &v)
        : Vector2(v)
    {
        normalize();
    }

    constexpr NormalizedVector2 &operator=(const Vector2 &v)
    {
        Vector2::operator=(v);
        normalize();
        return *this;
    }
};

}
