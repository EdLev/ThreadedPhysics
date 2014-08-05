//Vector4 implementation using SSE intrinsics.
#include "Vector4.hpp"

#ifdef VECTORIZATION_SSE

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
	{
		xmm = _mm_set_ss(1.0f);
	}

	Vector4::Vector4(const Vector4& other)
		: xmm(other.xmm)
	{}

	Vector4& Vector4::operator = (const Vector4& other)
	{
		xmm = other.xmm;

		return *this;
	}

	float& Vector4::operator [] (const int index)
	{
		assert(index >= 0);
		assert(index <= 3);

		return *((float*)&xmm + index);
	}

	const float& Vector4::operator [] (const int index) const
	{
		assert(index >= 0);
		assert(index <= 3);

		return *((float*)&xmm + index);
	}

	float* Vector4::begin()
	{
		return (float*)&xmm;
	}

	const float* Vector4::begin() const
	{
		return (const float*)&xmm;
	}

	float* Vector4::end()
	{
		return &W + 1;
	}

	Vector4& Vector4::operator += (const Vector4& other)
	{
		xmm = _mm_add_ps(xmm, other.xmm);

		return *this;
	}

	Vector4 Vector4::operator + (const Vector4& other) const
	{
		return Vector4(*this) += other;
	}

	Vector4& Vector4::operator -= (const Vector4& other)
	{
		xmm = _mm_sub_ps(xmm, other.xmm);

		return *this;
	}

	Vector4 Vector4::operator - (const Vector4& other) const
	{
		return Vector4(*this) -= other;
	}

	Vector4& Vector4::operator *= (float scalar)
	{
		__m128 scalarVec = _mm_load1_ps(&scalar);
		xmm = _mm_mul_ps(xmm, scalarVec);
	
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
		__m128 resultVec = _mm_dp_ps(xmm, other.xmm, 0x7f);
		float result;
		_mm_store_ss(&result, resultVec);

		return result;
	}

	float Vector4::length3Squared() const
	{
		return dot3(*this);
	}

	float Vector4::length3() const
	{
		return sqrt(length3Squared());
	}

	Vector4& Vector4::normalize3()
	{
		return (*this /= length3());
	}

	Vector4 Vector4::getNormalized3() const
	{
		Vector4 result(*this);
		result.normalize3();
		return result;
	}

	float Vector4::dot4(const Vector4& other) const
	{
		__m128 resultVec = _mm_dp_ps(xmm, other.xmm, 0xff);
		float result;
		_mm_store_ss(&result, resultVec);
		return result;
	}

	float Vector4::length4Squared() const
	{
		return dot4(*this);
	}

	float Vector4::length4() const
	{
		return sqrt(length4Squared());
	}

	Vector4& Vector4::normalize4()
	{
		return (*this /= length4());
	}

	Vector4 Vector4::getNormalized4() const
	{
		Vector4 result(*this);
		result.normalize4();
		return result;
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