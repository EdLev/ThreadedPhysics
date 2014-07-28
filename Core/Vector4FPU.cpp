//Vector4 implementation using FPU math as a fallback, if no vectorized version was available for the platform.
#include "Vector4.hpp"

#ifdef VECTORIZATION_NONE

#include <cmath>

namespace Core
{
	Vector4::Vector4(const float x, const float y, const float z, const float w /* = 1.0f */)
		: X(x), Y(y), Z(z), W(w)
	{}

	Vector4::Vector4(const float value)
		: X(value), Y(value), Z(value), W(value)
	{}

	Vector4::Vector4()
		: X(0.0f), Y(0.0f), Z(0.0f), W(1.0f)
	{}

	Vector4::Vector4(const Vector4& other)
		: X(other.X), Y(other.Y), Z(other.Z), W(other.W)
	{}

	Vector4& Vector4::operator = (const Vector4& other)
	{
		X = other.X;
		Y = other.Y;
		Z = other.Z;
		W = other.W;

		return *this;
	}

	float& Vector4::operator [] (const int index)
	{
		assert(index >= 0);
		assert(index <= 3);

		return *(&X + index);
	}

	float* Vector4::begin()
	{
		return &X;
	}

	float* Vector4::end()
	{
		return &W + 1;
	}

	Vector4& Vector4::operator += (const Vector4& other)
	{
		X += other.X;
		Y += other.Y;
		Z += other.Z;
		W += other.W;

		return *this;
	}

	Vector4 Vector4::operator + (const Vector4& other) const
	{
		return Vector4(*this) += other;
	}

	Vector4& Vector4::operator -= (const Vector4& other)
	{
		X -= other.X;
		Y -= other.Y;
		Z -= other.Z;
		W -= other.W;

		return *this;
	}

	Vector4 Vector4::operator - (const Vector4& other) const
	{
		return Vector4(*this) -= other;
	}

	Vector4& Vector4::operator *= (float scalar)
	{
		X *= scalar;
		Y *= scalar;
		Z *= scalar;
		W *= scalar;

		return *this;
	}

	Vector4 Vector4::operator * (float scalar) const
	{
		return Vector4(*this) *= scalar;
	}

	Vector4& Vector4::operator /= (float scalar)
	{
		float reciprocal = 1.0f / scalar;
		return *this *= reciprocal;
	}

	Vector4 Vector4::operator / (float scalar) const
	{
		return Vector4(*this) /= scalar;
	}

	float Vector4::dot3(const Vector4& other) const
	{
		return X * other.X + Y * other.Y + Z * other.Z;
	}

	float Vector4::length3Squared() const
	{
		return dot3(*this);
	}

	float Vector4::length3() const
	{
		return sqrt(length3Squared());
	}

	float Vector4::dot4(const Vector4& other) const
	{
		return dot3(other) + W * other.W;
	}

	float Vector4::length4Squared() const
	{
		return dot4(*this);
	}

	float Vector4::length4() const
	{
		return sqrt(length4Squared());
	}

	Vector4& Vector4::crossEquals(const Vector4& other)
	{
		X = Y * other.Z - Z * other.Y;
		Y = Z * other.X - X * other.Z;
		Z = X * other.Y - Y * other.X;
		return *this;
	}

	Vector4 Vector4::cross(const Vector4& other) const
	{
		return Vector4(*this).crossEquals(other);
	}
}

#endif