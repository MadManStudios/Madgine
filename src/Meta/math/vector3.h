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

#include "common.h"
#include "vector2.h"
#include "vector3i.h"

namespace Engine {
/** Standard 3-dimensional vector.
	@remarks
	A direction in 3D space represented as distances along the 3
	orthogonal axes (x, y, z). Note that positions, directions and
	scaling factors can be represented by a vector, depending on how
	you interpret the values.
	*/
struct META_EXPORT Vector3 {
    float x, y, z;

public:
    /** Default constructor.
		@note
		It does <b>NOT</b> initialize the vector for efficiency.
		*/
    Vector3() = default;

    constexpr Vector3(const Vector3 &) = default;

    constexpr Vector3(const float fX, const float fY, const float fZ)
        : x(fX)
        , y(fY)
        , z(fZ)
    {
    }

    constexpr Vector3(const float afCoordinate[3])
        : x(afCoordinate[0])
        , y(afCoordinate[1])
        , z(afCoordinate[2])
    {
    }

    explicit Vector3(const int afCoordinate[3])
    {
        x = static_cast<float>(afCoordinate[0]);
        y = static_cast<float>(afCoordinate[1]);
        z = static_cast<float>(afCoordinate[2]);
    }

    Vector3(const Vector3i &v)
    {
        x = static_cast<float>(v[0]);
        y = static_cast<float>(v[1]);
        z = static_cast<float>(v[2]);
    }

    explicit constexpr Vector3(float *const r)
        : x(r[0])
        , y(r[1])
        , z(r[2])
    {
    }

    explicit Vector3(const float scaler)
        : x(scaler)
        , y(scaler)
        , z(scaler)
    {
    }

    explicit Vector3(const Vector2 &xy, const float z)
        : x(xy.x)
        , y(xy.y)
        , z(z)
    {
    }

    /** Swizzle-like narrowing operations
		*/
    Vector2 xy() const
    {
        return { x, y };
    }

    Vector3 xyz() const
    {
        return { x, y, z };
    }

    /** Exchange the contents of this vector with another.
		*/
    void swap(Vector3 &other) noexcept
    {
        std::swap(x, other.x);
        std::swap(y, other.y);
        std::swap(z, other.z);
    }

    float operator[](const size_t i) const
    {
        assert(i < 3);

        return *(&x + i);
    }

    float &operator[](const size_t i)
    {
        assert(i < 3);

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
    constexpr Vector3 &operator=(const Vector3 &rkVector) = default;

    constexpr Vector3 &operator=(const float fScaler)
    {
        x = fScaler;
        y = fScaler;
        z = fScaler;

        return *this;
    }

    bool equalsWithEpsilon(const Vector3 &other, float epsilon = floatZeroThreshold)
    {
        return isEqual(x, other.x, epsilon) && isEqual(y, other.y, epsilon) && isEqual(z, other.z, epsilon);
    }

    bool operator==(const Vector3 &rkVector) const
    {
        return x == rkVector.x && y == rkVector.y && z == rkVector.z;
    }

    bool operator!=(const Vector3 &rkVector) const
    {
        return x != rkVector.x || y != rkVector.y || z != rkVector.z;
    }

    // arithmetic operations
    constexpr Vector3 operator+(const Vector3 &rkVector) const
    {
        return Vector3(
            x + rkVector.x,
            y + rkVector.y,
            z + rkVector.z);
    }

    Vector3 operator-(const Vector3 &rkVector) const
    {
        return Vector3(
            x - rkVector.x,
            y - rkVector.y,
            z - rkVector.z);
    }

    Vector3 operator*(const float fScalar) const
    {
        return Vector3(
            x * fScalar,
            y * fScalar,
            z * fScalar);
    }

    Vector3 operator*(const Vector3 &rhs) const
    {
        return Vector3(
            x * rhs.x,
            y * rhs.y,
            z * rhs.z);
    }

    Vector3 operator/(const float fScalar) const
    {
        assert(fScalar != 0.0);

        float fInv = 1.0f / fScalar;

        return Vector3(
            x * fInv,
            y * fInv,
            z * fInv);
    }

    Vector3 operator/(const Vector3 &rhs) const
    {
        return Vector3(
            x / rhs.x,
            y / rhs.y,
            z / rhs.z);
    }

    const Vector3 &operator+() const
    {
        return *this;
    }

    Vector3 operator-() const
    {
        return Vector3(-x, -y, -z);
    }

    // overloaded operators to help Vector3
    friend constexpr Vector3 operator*(const float fScalar, const Vector3 &rkVector)
    {
        return Vector3(
            fScalar * rkVector.x,
            fScalar * rkVector.y,
            fScalar * rkVector.z);
    }

    friend Vector3 operator/(const float fScalar, const Vector3 &rkVector)
    {
        return Vector3(
            fScalar / rkVector.x,
            fScalar / rkVector.y,
            fScalar / rkVector.z);
    }

    friend Vector3 operator+(const Vector3 &lhs, const float rhs)
    {
        return Vector3(
            lhs.x + rhs,
            lhs.y + rhs,
            lhs.z + rhs);
    }

    friend Vector3 operator+(const float lhs, const Vector3 &rhs)
    {
        return Vector3(
            lhs + rhs.x,
            lhs + rhs.y,
            lhs + rhs.z);
    }

    friend Vector3 operator-(const Vector3 &lhs, const float rhs)
    {
        return Vector3(
            lhs.x - rhs,
            lhs.y - rhs,
            lhs.z - rhs);
    }

    friend Vector3 operator-(const float lhs, const Vector3 &rhs)
    {
        return Vector3(
            lhs - rhs.x,
            lhs - rhs.y,
            lhs - rhs.z);
    }

    // arithmetic updates
    Vector3 &operator+=(const Vector3 &rkVector)
    {
        x += rkVector.x;
        y += rkVector.y;
        z += rkVector.z;

        return *this;
    }

    Vector3 &operator+=(const float fScalar)
    {
        x += fScalar;
        y += fScalar;
        z += fScalar;
        return *this;
    }

    Vector3 &operator-=(const Vector3 &rkVector)
    {
        x -= rkVector.x;
        y -= rkVector.y;
        z -= rkVector.z;

        return *this;
    }

    Vector3 &operator-=(const float fScalar)
    {
        x -= fScalar;
        y -= fScalar;
        z -= fScalar;
        return *this;
    }

    Vector3 &operator*=(const float fScalar)
    {
        x *= fScalar;
        y *= fScalar;
        z *= fScalar;
        return *this;
    }

    Vector3 &operator*=(const Vector3 &rkVector)
    {
        x *= rkVector.x;
        y *= rkVector.y;
        z *= rkVector.z;

        return *this;
    }

    Vector3 &operator/=(const float fScalar)
    {
        assert(fScalar != 0.0);

        float fInv = 1.0f / fScalar;

        x *= fInv;
        y *= fInv;
        z *= fInv;

        return *this;
    }

    Vector3 &operator/=(const Vector3 &rkVector)
    {
        x /= rkVector.x;
        y /= rkVector.y;
        z /= rkVector.z;

        return *this;
    }

    /** Returns the length (magnitude) of the vector.
		@warning
		This operation requires a square root and is expensive in
		terms of CPU operations. If you don't need to know the exact
		length (e.g. for just comparing lengths) use squaredLength()
		instead.
		*/
    constexpr float length() const
    {
        return sqrtf(x * x + y * y + z * z);
    }

    /** Returns the square of the length(magnitude) of the vector.
		@remarks
		This  method is for efficiency - calculating the actual
		length of a vector requires a square root, which is expensive
		in terms of the operations required. This method returns the
		square of the length of the vector, i.e. the same as the
		length but before the square root is taken. Use this if you
		want to find the longest / shortest vector without incurring
		the square root.
		*/
    float squaredLength() const
    {
        return x * x + y * y + z * z;
    }

    /** Returns the distance to another vector.
		@warning
		This operation requires a square root and is expensive in
		terms of CPU operations. If you don't need to know the exact
		distance (e.g. for just comparing distances) use squaredDistance()
		instead.
		*/
    float distance(const Vector3 &rhs) const
    {
        return (*this - rhs).length();
    }

    /** Returns the square of the distance to another vector.
		@remarks
		This method is for efficiency - calculating the actual
		distance to another vector requires a square root, which is
		expensive in terms of the operations required. This method
		returns the square of the distance to another vector, i.e.
		the same as the distance but before the square root is taken.
		Use this if you want to find the longest / shortest distance
		without incurring the square root.
		*/
    float squaredDistance(const Vector3 &rhs) const
    {
        return (*this - rhs).squaredLength();
    }

    /** Calculates the dot (scalar) product of this vector with another.
		@remarks
		The dot product can be used to calculate the angle between 2
		vectors. If both are unit vectors, the dot product is the
		cosine of the angle; otherwise the dot product must be
		divided by the product of the lengths of both vectors to get
		the cosine of the angle. This result can further be used to
		calculate the distance of a point from a plane.
		@param
		vec Vector with which to calculate the dot product (together
		with this one).
		@return
		A float representing the dot product value.
		*/
    float dotProduct(const Vector3 &vec) const
    {
        return x * vec.x + y * vec.y + z * vec.z;
    }

    /** Calculates the absolute dot (scalar) product of this vector with another.
		@remarks
		This function work similar dotProduct, except it use absolute value
		of each component of the vector to computing.
		@param
		vec Vector with which to calculate the absolute dot product (together
		with this one).
		@return
		A float representing the absolute dot product value.
		*/
    float absDotProduct(const Vector3 &vec) const
    {
        return fabs(x * vec.x) + fabs(y * vec.y) + fabs(z * vec.z);
    }

    /** Normalises the vector.
		@remarks
		This method normalises the vector such that it's
		length / magnitude is 1. The result is called a unit vector.
		@note
		This function will not crash for zero-sized vectors, but there
		will be no changes made to their components.
		@return The previous length of the vector.
		*/
    constexpr float normalize()
    {
        float fLength = sqrtf(x * x + y * y + z * z);

        // Will also work for zero-sized vectors, but will change nothing
        // We're not using epsilons because we don't need to.
        // Read http://www.ogre3d.org/forums/viewtopic.php?f=4&t=61259
        if (fLength > float(0.0f)) {
            float fInvLength = 1.0f / fLength;
            x *= fInvLength;
            y *= fInvLength;
            z *= fInvLength;
        }

        return fLength;
    }

    /** Calculates the cross-product of 2 vectors, i.e. the vector that
		lies perpendicular to them both.
		@remarks
		The cross-product is normally used to calculate the normal
		vector of a plane, by calculating the cross-product of 2
		non-equivalent vectors which lie on the plane (e.g. 2 edges
		of a triangle).
		@param rkVector
		Vector which, together with this one, will be used to
		calculate the cross-product.
		@return
		A vector which is the result of the cross-product. This
		vector will <b>NOT</b> be normalised, to maximise efficiency
		- call Vector3::normalise on the result if you wish this to
		be done. As for which side the resultant vector will be on, the
		returned vector will be on the side from which the arc from 'this'
		to rkVector is anticlockwise, e.g. UNIT_Y.crossProduct(UNIT_Z)
		= UNIT_X, whilst UNIT_Z.crossProduct(UNIT_Y) = -UNIT_X.
		This is because OGRE uses a right-handed coordinate system.
		@par
		For a clearer explanation, look a the left and the bottom edges
		of your monitor's screen. Assume that the first vector is the
		left edge and the second vector is the bottom edge, both of
		them starting from the lower-left corner of the screen. The
		resulting vector is going to be perpendicular to both of them
		and will go <i>inside</i> the screen, towards the cathode tube
		(assuming you're using a CRT monitor, of course).
		*/
    Vector3 crossProduct(const Vector3 &rkVector) const
    {
        return Vector3(
            y * rkVector.z - z * rkVector.y,
            z * rkVector.x - x * rkVector.z,
            x * rkVector.y - y * rkVector.x);
    }

    /** Returns a vector at a point half way between this and the passed
		in vector.
		*/
    Vector3 midPoint(const Vector3 &vec) const
    {
        return Vector3(
            (x + vec.x) * 0.5f,
            (y + vec.y) * 0.5f,
            (z + vec.z) * 0.5f);
    }

    /** Returns true if the vector's scalar components are all greater
		that the ones of the vector it is compared against.
		*/
    /* bool operator<(const Vector3 &rhs) const
    {
        if (x < rhs.x && y < rhs.y && z < rhs.z)
            return true;
        return false;
    }*/

    /** Returns true if the vector's scalar components are all smaller
		that the ones of the vector it is compared against.
		*/
    /* bool operator>(const Vector3 &rhs) const
    {
        if (x > rhs.x && y > rhs.y && z > rhs.z)
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
    void makeFloor(const Vector3 &cmp)
    {
        if (cmp.x < x)
            x = cmp.x;
        if (cmp.y < y)
            y = cmp.y;
        if (cmp.z < z)
            z = cmp.z;
    }

    /** Sets this vector's components to the maximum of its own and the
		ones of the passed in vector.
		@remarks
		'Maximum' in this case means the combination of the highest
		value of x, y and z from both vectors. Highest is taken just
		numerically, not magnitude, so 1 > -3.
		*/
    void makeCeil(const Vector3 &cmp)
    {
        if (cmp.x > x)
            x = cmp.x;
        if (cmp.y > y)
            y = cmp.y;
        if (cmp.z > z)
            z = cmp.z;
    }

    /** Generates a vector perpendicular to this vector (eg an 'up' vector).
		@remarks
		This method will return a vector which is perpendicular to this
		vector. There are an infinite number of possibilities but this
		method will guarantee to generate one of them. If you need more
		control you should use the Quaternion class.
		*/
    Vector3 perpendicular() const
    {
        // From Sam Hocevar's article "On picking an orthogonal
        // vector (and combing coconuts)"
        Vector3 perp = fabs(x) > fabs(z)
            ? Vector3(-y, x, 0.0)
            : Vector3(0.0, -z, y);
        return perp.normalizedCopy();
    }

    /* Generates a new random vector which deviates from this vector by a
		given angle in a random direction.
		@remarks
		This method assumes that the random number generator has already
		been seeded appropriately.
		@param
		angle The angle at which to deviate
		@param
		up Any vector perpendicular to this one (which could generated
		by cross-product of this vector and any other non-colinear
		vector). If you choose not to provide this the function will
		derive one on it's own, however if you provide one yourself the
		function will be faster (this allows you to reuse up vectors if
		you call this method more than once)
		@return
		A random vector which deviates from this vector by angle. This
		vector will not be normalised, normalise it if you wish
		afterwards.
		*/
    /*inline Vector3 randomDeviant(
			const Radian& angle,
			const Vector3& up = Vector3::ZERO) const
		{
			Vector3 newUp;

			if (up == Vector3::ZERO)
			{
				// Generate an up vector
				newUp = this->perpendicular();
			}
			else
			{
				newUp = up;
			}

			// Rotate up vector by random amount around this
			Quaternion q;
			q.FromAngleAxis(Radian(Math::UnitRandom() * Math::TWO_PI), *this);
			newUp = q * newUp;

			// Finally rotate this by given angle around randomised up
			q.FromAngleAxis(angle, newUp);
			return q * (*this);
		}*/

    /* Gets the angle between 2 vectors.
		@remarks
		Vectors do not have to be unit-length but must represent directions.
		*/
    /*inline Radian angleBetween(const Vector3& dest) const
		{
			float lenProduct = length() * dest.length();

			// Divide by zero check
			if (lenProduct < 1e-6f)
				lenProduct = 1e-6f;

			float f = dotProduct(dest) / lenProduct;

			f = Math::Clamp(f, (float)-1.0, (float)1.0);
			return Math::ACos(f);

		}*/
    /* Gets the shortest arc quaternion to rotate this vector to the destination
		vector.
		@remarks
		If you call this with a dest vector that is close to the inverse
		of this vector, we will rotate 180 degrees around the 'fallbackAxis'
		(if specified, or a generated axis if not) since in this case
		ANY axis of rotation is valid.
		*/
    /*Quaternion getRotationTo(const Vector3& dest,
			const Vector3& fallbackAxis = Vector3::ZERO) const
		{
			// From Sam Hocevar's article "Quaternion from two vectors:
			// the final version"
			float a = Math::Sqrt(this->squaredLength() * dest.squaredLength());
			float b = a + this->dotProduct(dest);
			Vector3 axis;

			if (b < (float)1e-06 * a)
			{
				b = (float)0.0;
				axis = fallbackAxis != Vector3::ZERO ? fallbackAxis
					: Math::Abs(x) > Math::Abs(z) ? Vector3(-y, x, (float)0.0)
					: Vector3((float)0.0, -z, y);
			}
			else
			{
				axis = this->crossProduct(dest);
			}

			Quaternion q(b, axis.x, axis.y, axis.z);
			q.normalise();
			return q;
		}*/

    /** Returns true if this vector is zero length. */
    bool isZeroLength() const
    {
        float sqlen = x * x + y * y + z * z;
        return sqlen < 1e-06 * 1e-06;
    }

    /** As normalise, except that this vector is unaffected and the
		normalised vector is returned as a copy. */
    Vector3 normalizedCopy() const
    {
        Vector3 ret = *this;
        ret.normalize();
        return ret;
    }

    /** Calculates a reflection vector to the plane with the given normal .
		@remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
		*/
    Vector3 reflect(const Vector3 &normal) const
    {
        return Vector3(*this - 2 * this->dotProduct(normal) * normal);
    }

    /* Returns whether this vector is within a positional tolerance
		of another vector.
		@param rhs The vector to compare with
		@param tolerance The amount that each element of the vector may vary by
		and still be considered equal
		*/
    /*inline bool positionEquals(const Vector3& rhs, float tolerance = 1e-03) const
		{
			return Math::floatEqual(x, rhs.x, tolerance) &&
				Math::floatEqual(y, rhs.y, tolerance) &&
				Math::floatEqual(z, rhs.z, tolerance);

		}*/

    /** Returns whether this vector is within a positional tolerance
		of another vector, also take scale of the vectors into account.
		@param rhs The vector to compare with
		@param tolerance The amount (related to the scale of vectors) that distance
		of the vector may vary by and still be considered close
		*/
    bool positionCloses(const Vector3 &rhs, float tolerance = 1e-03f) const
    {
        return squaredDistance(rhs) <= (squaredLength() + rhs.squaredLength()) * tolerance;
    }

    /// Extract the primary (dominant) axis from this direction vector
    Vector3 primaryAxis() const
    {
        float absx = fabs(x);
        float absy = fabs(y);
        float absz = fabs(z);
        if (absx > absy) {
            if (absx > absz)
                return x > 0 ? UNIT_X : NEGATIVE_UNIT_X;
            return z > 0 ? UNIT_Z : NEGATIVE_UNIT_Z;
        }
        // absx <= absy
        if (absy > absz)
            return y > 0 ? UNIT_Y : NEGATIVE_UNIT_Y;
        return z > 0 ? UNIT_Z : NEGATIVE_UNIT_Z;
    }

    constexpr Vector3i floor() const
    {
        return { static_cast<int>(x), static_cast<int>(y), static_cast<int>(z) };
    }

    // special points
    static const constexpr float ZERO[] { 0, 0, 0 };
    static const constexpr float UNIT_X[] { 1, 0, 0 };
    static const constexpr float UNIT_Y[] { 0, 1, 0 };
    static const constexpr float UNIT_Z[] { 0, 0, 1 };
    static const constexpr float NEGATIVE_UNIT_X[] { -1, 0, 0 };
    static const constexpr float NEGATIVE_UNIT_Y[] { 0, -1, 0 };
    static const constexpr float NEGATIVE_UNIT_Z[] { 0, 0, -1 };
    static const constexpr float UNIT_SCALE[] { 1, 1, 1 };

    /** Function for writing to a stream.
		*/
    friend std::ostream &operator<<(std::ostream &o, const Vector3 &v)
    {
        o << "[" << v.x << ", " << v.y << ", " << v.z << "]";
        return o;
    }

    friend std::istream &operator>>(std::istream &in, Vector3 &v)
    {
        char c;
        in >> c;
        if (c != '[')
            std::terminate();
        for (int i = 0; i < 3; ++i) {
            in >> v[i];
            in >> c;
            char expectedChar = (i == 2) ? ']' : ',';
            if (c != expectedChar)
                std::terminate();
        }
        return in;
    }
};

struct NormalizedVector3 : Vector3 {

    constexpr NormalizedVector3(float x, float y, float z)
        : Vector3(x, y, z)
    {
        normalize();
    }

    constexpr NormalizedVector3(const Vector3 &v)
        : Vector3(v)
    {
        normalize();
    }

    constexpr NormalizedVector3 &operator=(const Vector3 &v)
    {
        Vector3::operator=(v);
        normalize();
        return *this;
    }
};

constexpr Vector3 min(const Vector3 &v1, const Vector3 &v2)
{
    return {
        std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z)
    };
}

constexpr Vector3 max(const Vector3 &v1, const Vector3 &v2)
{
    return {
        std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)
    };
}

constexpr Vector3 lerp(const Vector3 &v1, const Vector3 &v2, float ratio)
{
    return (1.0f - ratio) * v1 + ratio * v2;
}

inline Vector3 slerp(const Vector3 &v1, const Vector3 &v2, float ratio)
{
    float theta = acosf(v1.normalizedCopy().dotProduct(v2.normalizedCopy()));
    float sinTheta = sinf(theta);
    float f1 = sinf((1.0f - ratio) * theta) / sinTheta;
    float f2 = sinf(ratio * theta) / sinTheta;
    return f1 * v1 + f2 * v2;
}

}

REGISTER_TYPE(Engine::Vector3)