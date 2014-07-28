Vector4()
#if !ENABLE_SSE2
: X(0.0f), Y(0.0f), Z(0.0f), W(1.0f)
#endif
{
#if ENABLE_SSE2
	xmm = _mm_set_ss(1.0f);
#endif
}

Vector4(const Vector4& other)
#if ENABLE_SSE2
: xmm(other.xmm)
#else
: X(other.X), Y(other.Y), Z(other.Z), W(other.W)
#endif
{}

Vector4& operator = (const Vector4& other)
{
#if ENABLE_SSE2
	xmm = other.xmm;
#else
	X = other.X;
	Y = other.Y;
	Z = other.Z;
	W = other.W;
#endif
	return *this;
}

float& operator [] (const int index)
{
#if ENABLE_SSE2
	return *((float*)&xmm + index);
#else
	assert(index >= 0);
	assert(index <= 3);

#endif
	return *(&X + index);
}

float* begin()
{
#if ENABLE_SSE2
	return (float*)&xmm;
#else
	return &X;
#endif
}

float* end()
{
	return &W + 1;
}

inline Vector4& operator += (const Vector4& other)
{
#if ENABLE_SSE2
	xmm = _mm_add_ps(xmm, other.xmm);
#else
	X += other.X;
	Y += other.Y;
	Z += other.Z;
	W += other.W;
#endif
	return *this;
}

inline Vector4 operator + (const Vector4& other) const
{
	return Vector4(*this) += other;
}

inline Vector4& operator -= (const Vector4& other)
{
#if ENABLE_SSE2
	xmm = _mm_sub_ps(xmm, other.xmm);
#else
	X -= other.X;
	Y -= other.Y;
	Z -= other.Z;
	W -= other.W;
#endif
	return *this;
}

inline Vector4 operator - (const Vector4& other) const
{
	return Vector4(*this) -= other;
}

inline Vector4& operator *= (float scalar)
{
#if ENABLE_SSE2
	__m128 scalarVec = _mm_load1_ps(&scalar);
	xmm = _mm_mul_ps(xmm, scalarVec);
#else
	X *= scalar;
	Y *= scalar;
	Z *= scalar;
	W *= scalar;
#endif
	return *this;
}

inline Vector4 operator * (float scalar) const
{
	return Vector4(*this) *= scalar;
}

inline Vector4& operator /= (float scalar)
{
	float reciprocal = 1.0f / scalar;
	return *this *= reciprocal;
}

inline Vector4 operator / (float scalar) const
{
	return Vector4(*this) /= scalar;
}

inline float dot3(const Vector4& other) const
{
#if ENABLE_SSE2
	__m128 resultVec = _mm_dp_ps(xmm, other.xmm, 0x7f);
	float result;
	_mm_store_ss(&result, resultVec);
	return result;
#else
	return X * other.X + Y * other.Y + Z * other.Z;
#endif
}

inline float length3Squared() const
{
	return dot3(*this);
}

inline float length3() const
{
	return sqrt(length3Squared());
}

inline float dot4(const Vector4& other) const
{
#if ENABLE_SSE2
	__m128 resultVec = _mm_dp_ps(xmm, other.xmm, 0xff);
	float result;
	_mm_store_ss(&result, resultVec);
	return result;
#else
	return dot3(other) + W * other.W;
#endif
}

inline float length4Squared() const
{
	return dot4(*this);
}

inline float length4() const
{
	return sqrt(length4Squared());
}

inline Vector4& crossEquals(const Vector4& other)
{
	X = Y * other.Z - Z * other.Y;
	Y = Z * other.X - X * other.Z;
	Z = X * other.Y - Y * other.X;
	return *this;
}

inline Vector4 cross(const Vector4& other) const
{
	return Vector4(*this).crossEquals(other);
}
explicit Vector4(const float x, const float y, const float z, const float w = 1.0f)
: X(x), Y(y), Z(z), W(w)
{}

explicit Vector4(const float value)
: X(value), Y(value), Z(value), W(value)
{}