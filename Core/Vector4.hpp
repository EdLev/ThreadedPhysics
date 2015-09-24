#pragma once

#include "Assert.hpp"
#include <intrin.h>

#if (_WIN32 || _WIN64 || _LINUX)
#define VECTORIZATION_SSE
#else
#define VECTORIZATION_NONE
#endif

namespace Core
{
	//SIMD vector class (for platforms that support it)
	//Includes dot, length and cross methods for 3D vector functionality.
	class Vector4
	{
	public:

		explicit Vector4(const float x, const float y, const float z, const float w = 0.0f);
		explicit Vector4(const float value);
		Vector4();
		Vector4(const Vector4& other);

		Vector4& operator = (const Vector4& other);

		const float& operator [] (const int index) const;
		float& operator [] (const int index);
		float* begin();
		const float* begin() const;
		float* end();

		Vector4& operator += (const Vector4& other);
		Vector4 operator + (const Vector4& other) const;
		Vector4& operator -= (const Vector4& other);
		Vector4 operator - (const Vector4& other) const;

		Vector4& operator *= (float scalar);
		Vector4 operator * (float scalar) const;
		Vector4& operator /= (float scalar);
		Vector4 operator / (float scalar) const;

		float dot3(const Vector4& other) const;
		float length3Squared() const;
		float length3() const;
		Vector4& normalize3();
		Vector4 getNormalized3() const;
		float dot4(const Vector4& other) const;
		float length4Squared() const;
		float length4() const;
		Vector4& normalize4();
		Vector4 getNormalized4() const;

		Vector4& crossEquals(const Vector4& other);
		Vector4 cross(const Vector4& other) const;

#pragma warning(push)
#pragma warning(disable : 4201)
		union
		{
			struct
			{
				float X, Y, Z, W;
			};
			//16-byte aligned members provide alignment to the struct
#ifdef VECTORIZATION_SSE
			__m128 xmm;
#endif
		};
#pragma warning(pop)
	};
}