#pragma once

#include "Vector4.hpp"
//Basd on Vector4, so most vectorization is done there

#include <array>
#include <algorithm>

namespace Core
{
	//row-based matrix
	class Matrix4
	{
	public:
		Matrix4()
		{
			//default initialize to identity
			Rows[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			Rows[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			Rows[2] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			Rows[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		Matrix4(const Matrix4& other)
			: Rows(other.Rows)
		{}

		Matrix4(std::array< Vector4, 4 > rows)
		{
			Rows = rows;
		}

		Matrix4(const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3)
		{
			Rows[0] = row0;
			Rows[1] = row1;
			Rows[2] = row2;
			Rows[3] = row3;
		}

		Matrix4(const Vector4& translation)
		{
			Rows[0] = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
			Rows[1] = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
			Rows[2] = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
			Rows[3] = translation;
		}

		Matrix4(const Vector4& axis, const float angle)
		{
			float cosine = cos(angle * 0.0174532925f);
			float sine = sin(angle * 0.0174532925f);
			float oneMinusCos = 1 - cosine;

			Rows[0] = Vector4(axis.X * axis.X * oneMinusCos + cosine, axis.X * axis.Y * oneMinusCos - axis.Z * sine, axis.X * axis.Z * oneMinusCos + axis.Y * sine , 0.0f);
			Rows[1] = Vector4(axis.Y * axis.X * oneMinusCos + axis.Z * sine , axis.Y * axis.Y * oneMinusCos + cosine, axis.Y * axis.Z * oneMinusCos - axis.X * sine, 0.0f);
			Rows[2] = Vector4(axis.Z * axis.X * oneMinusCos - axis.Y * sine, axis.Z * axis.Y * oneMinusCos + axis.X * sine, axis.Z * axis.Z * oneMinusCos + cosine, 0.0f);
			Rows[3] = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		}

		static void OpenGlFrustum(float l, float r, float b, float t, float n, float f, Matrix4 &mat) 
		{ 
			mat[0][0] = 2 * n / (r - l); 
			mat[0][1] = 0; 
			mat[0][2] = 0; 
			mat[0][3] = 0; 

			mat[1][0] = 0; 
			mat[1][1] = 2 * n / (t - b); 
			mat[1][2] = 0; 
			mat[1][3] = 0; 

			mat[2][0] = (r + l) / (r - l); 
			mat[2][1] = (t + b) / (t - b); 
			mat[2][2] = -(f + n) / (f - n); 
			mat[2][3] = -2 * f * n / (f - n);

			mat[3][0] = 0; 
			mat[3][1] = 0; 
			mat[3][2] = -1;
			mat[3][3] = 0; 
		}

		static void OpenGlPerspective(float angle, float imageAspectRatio, float n, float f, Matrix4 &mat) 
		{ 
			float scale = tan(0.0174532925f * (angle * 0.5f)) * n;
			float r = imageAspectRatio * scale, l = -r; 
			float t = scale, b = -t; 
			OpenGlFrustum(l, r, b, t, n, f, mat); 
		}

		static void setorthographicmat(float l, float r, float t, float b, float n, float f, Matrix4 &mat) 
		{ 
			mat[0][0] = 2 / (r - l); 
			mat[0][1] = 0; 
			mat[0][2] = 0; 
			mat[0][3] = 0; 
			
			mat[1][0] = 0; 
			mat[1][1] = 2 / (t - b); 
			mat[1][2] = 0; 
			mat[1][3] = 0; 
			
			mat[2][0] = 0; 
			mat[2][1] = 0; 
			mat[2][2] = -1 / (f - n); 
			mat[2][3] = 0; 
			
			mat[3][0] = -(r + l) / (r - l); 
			mat[3][1] = -(t + b) / (t - b); 
			mat[3][2] = -n / (f - n); 
			mat[3][3] = 1; 
		}

		static Matrix4 PerspectiveProjectionMatrix(const float fov, const float aspectRatio, const float nearDistance, const float farDistance)
		{
			Matrix4 result;
			OpenGlPerspective(fov, aspectRatio, nearDistance, farDistance, result);
			return result;
		}

		static Matrix4 OrthographicProjectionMatrix(const float left, const float right, const float top, const float bottom, const float nearDistance, const float farDistance)
		{
			Matrix4 result;
			setorthographicmat(left, right, top, bottom, nearDistance, farDistance, result);
			return result;
		}

		Matrix4& operator = (const Matrix4& other)
		{
			Rows = other.Rows;
			return *this;
		}

		Vector4& operator [] (const int index)
		{
			return Rows[index];
		}

		Vector4* begin()
		{
			return &Rows[0];
		}

		const Vector4* begin() const
		{
			return &Rows[0];
		}

		Vector4* end()
		{
			return &Rows[3] + 1;
		}

		Vector4 column(const int index) const
		{
			assert(index >= 0);
			assert(index <= 3);

			return Vector4(Rows[0][index], Rows[1][index], Rows[2][index], Rows[3][index]);
		}

		Matrix4& operator += (const Matrix4& other)
		{
			std::transform(Rows.begin(), Rows.end(), other.Rows.begin(), Rows.begin(), 
				[](const Vector4& v1, const Vector4& v2)->Vector4{ return v1 + v2; });
			return *this;
		}

		Matrix4 operator + (const Matrix4& other) const
		{
			return Matrix4(*this) += other;
		}

		Matrix4& operator -= (const Matrix4& other)
		{
			std::transform(Rows.begin(), Rows.end(), other.Rows.begin(), Rows.begin(), 
				[](const Vector4& v1, const Vector4& v2)->Vector4{ return v1 - v2; });
			return *this;
		}

		Matrix4 operator - (const Matrix4& other) const
		{
			return Matrix4(*this) -= other;
		}

		Matrix4& operator *= (const float scalar)
		{
			std::transform(Rows.begin(), Rows.end(), Rows.begin(),
				[ &scalar ](const Vector4& v)->Vector4{ return v * scalar; });

			return *this;
		}

		Matrix4 operator * (const float scalar) const
		{
			return Matrix4(*this) *= scalar;
		}

		Matrix4& operator /= (const float scalar)
		{
			float reciprocal = 1.0f / scalar;
			std::transform(Rows.begin(), Rows.end(), Rows.begin(),
				[ reciprocal ](const Vector4& v)->Vector4{ return v * reciprocal; });

			return *this;
		}

		Matrix4 operator / (const float scalar) const
		{
			return Matrix4(*this) /= scalar;
		}

		//in this case, the non-assignment operator requires a copy to iterate, so the assignment version will be based on it
		Matrix4 operator * (const Matrix4& other) const
		{
			Matrix4 result(*this);
#ifdef VECTORIZATION_SSE
			std::array< Vector4, 4 > columns(other.Rows);
			_MM_TRANSPOSE4_PS(columns[0].xmm, columns[1].xmm, columns[2].xmm, columns[3].xmm);
#else
			const std::array< Vector4, 4 > columns = { other.column(0), other.column(1), other.column(2), other.column(3) };
#endif
			for(int rowIndex = 0; rowIndex < 4; ++rowIndex)
			{
				for(int columnIndex = 0; columnIndex < 4; ++columnIndex)
				{
					result.Rows[rowIndex][columnIndex] = Rows[rowIndex].dot4(columns[columnIndex]);
				}
			}
			return result;
		}

		Matrix4& operator *= (const Matrix4& other)
		{
			*this = *this * other;
			return *this;
		}

		Vector4 operator * (const Vector4& vector)
		{
			Vector4 result;
			result.X = Rows[0].dot4(vector);
			result.Y = Rows[1].dot4(vector);
			result.Z = Rows[2].dot4(vector);
			result.W = Rows[3].dot4(vector);

			return result;
		}

		Matrix4& transpose()
		{
#ifdef VECTORIZATION_SSE
			_MM_TRANSPOSE4_PS(Rows[0].xmm, Rows[1].xmm, Rows[2].xmm, Rows[3].xmm);
#else
			std::array< Vector4, 4 > columns = { column(0), column(1), column(2), column(3) };
			Rows = columns;
#endif
			return *this;
		}

		Matrix4 getTranspose() const
		{
			return Matrix4(*this).transpose();
		}

		//determinant of 2x2 matrix
		float det2(const float f00, const float f01, const float f10, const float f11)
		{
			return f00 * f11 - f01 * f10;
		}

		Matrix4& invert()
		{
#ifdef VECTORIZATION_SSE
			//From the Intel paper at http://download.intel.com/design/PentiumIII/sml/24504301.pdf

			float* src = begin()->begin();
			__m128 minor0, minor1, minor2, minor3;
			__m128 row0, row1, row2, row3;
			__m128 det, tmp1;

			tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src + 4));
			row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src + 8	)), (__m64*)(src + 12));
			row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
			row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
			tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src + 2)), (__m64*)(src + 6));
			row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src + 10)), (__m64*)(src + 14));
			row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
			row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
		// -----------------------------------------------
			tmp1 = _mm_mul_ps(row2, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_mul_ps(row1, tmp1);
			minor1 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
			minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
			minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
		// -----------------------------------------------
			tmp1 = _mm_mul_ps(row1, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
			minor3 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
			minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
		// -----------------------------------------------
			tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			row2 = _mm_shuffle_ps(row2, row2, 0x4E);
			minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
			minor2 = _mm_mul_ps(row0, tmp1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
			minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
		// -----------------------------------------------
			tmp1 = _mm_mul_ps(row0, row1);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
		//	-----------------------------------------------
			tmp1 = _mm_mul_ps(row0, row3);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
			minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
			minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
		//	-----------------------------------------------
			tmp1 = _mm_mul_ps(row0, row2);
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
			minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
			minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
			tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
			minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
			minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
		//	-----------------------------------------------
			det = _mm_mul_ps(row0, minor0);
			det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
			det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
			tmp1 = _mm_rcp_ss(det);
			det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
			det = _mm_shuffle_ps(det, det, 0x00);
			minor0 = _mm_mul_ps(det, minor0);
			_mm_storel_pi((__m64*)(src), minor0);
			_mm_storeh_pi((__m64*)(src + 2), minor0);
			minor1 = _mm_mul_ps(det, minor1);
			_mm_storel_pi((__m64*)(src + 4), minor1);
			_mm_storeh_pi((__m64*)(src + 6), minor1);
			minor2 = _mm_mul_ps(det, minor2);
			_mm_storel_pi((__m64*)(src + 8), minor2);
			_mm_storeh_pi((__m64*)(src + 10), minor2);
			minor3 = _mm_mul_ps(det, minor3);
			_mm_storel_pi((__m64*)(src + 12), minor3);
			_mm_storeh_pi((__m64*)(src + 14), minor3);
#else
			Matrix4 transposed = getTranspose();

			//Laplace expansion from http://www.geometrictools.com/Documentation/LaplaceExpansionTheorem.pdf

			//common 2x2 determinants used both in inverse and adjugate
			float s0 = det2((*this)[0][0], (*this)[0][1], (*this)[1][0], (*this)[1][1]);
			float s1 = det2((*this)[0][0], (*this)[0][2], (*this)[1][0], (*this)[1][2]);
			float s2 = det2((*this)[0][0], (*this)[0][3], (*this)[1][0], (*this)[1][3]);
			float s3 = det2((*this)[0][1], (*this)[0][2], (*this)[1][1], (*this)[1][2]);
			float s4 = det2((*this)[0][1], (*this)[0][3], (*this)[1][1], (*this)[1][3]);
			float s5 = det2((*this)[0][2], (*this)[0][3], (*this)[1][2], (*this)[1][3]);

			float c0 = det2((*this)[2][2], (*this)[2][3], (*this)[3][2], (*this)[3][3]);
			float c1 = det2((*this)[2][1], (*this)[2][3], (*this)[3][1], (*this)[3][3]);
			float c2 = det2((*this)[2][1], (*this)[2][2], (*this)[3][1], (*this)[3][2]);
			float c3 = det2((*this)[2][0], (*this)[2][3], (*this)[3][0], (*this)[3][3]);
			float c4 = det2((*this)[2][0], (*this)[2][3], (*this)[3][0], (*this)[3][2]);
			float c5 = det2((*this)[2][0], (*this)[2][1], (*this)[3][0], (*this)[3][1]);

			float determinant = s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0;
			float invDet = 1.0f / determinant;

			Matrix4 adjugate(	Core::Vector4(	 (*this)[1][1] * c5 - (*this)[1][2] * c4 + (*this)[1][3] * c3, 
												-(*this)[0][1] * c5 + (*this)[0][2] * c4 - (*this)[0][3] * c3, 
												 (*this)[3][1] * s5 - (*this)[3][2] * s4 + (*this)[3][3] * s3,
												-(*this)[2][1] * s5 + (*this)[2][2] * s4 - (*this)[2][3] * s3),

								Core::Vector4(	-(*this)[1][0] * c5 + (*this)[1][2] * c2 - (*this)[1][3] * c1,
												 (*this)[0][0] * c5 - (*this)[0][2] * c2 + (*this)[0][3] * c1, 
												-(*this)[3][0] * s5 + (*this)[3][2] * s2 - (*this)[3][3] * s1,
												 (*this)[2][0] * s5 - (*this)[2][2] * s2 + (*this)[2][3] * s1),

								Core::Vector4(	 (*this)[1][0] * c4 - (*this)[1][1] * c2 + (*this)[1][3] * c0,
												-(*this)[0][0] * c4 + (*this)[0][1] * c2 - (*this)[0][3] * c0, 
												 (*this)[3][0] * s4 - (*this)[3][1] * s2 + (*this)[3][3] * s0,
												-(*this)[2][0] * s4 + (*this)[2][1] * s2 - (*this)[2][3] * s0),

								Core::Vector4(	-(*this)[1][0] * c3 + (*this)[1][1] * c1 - (*this)[1][2] * c0,
												 (*this)[0][0] * c3 - (*this)[0][1] * c1 + (*this)[0][2] * c0,
												-(*this)[3][0] * s3 + (*this)[3][1] * s1 - (*this)[3][2] * s0,
												 (*this)[2][0] * s3 - (*this)[2][1] * s1 + (*this)[2][2] * s0));

			*this = adjugate * invDet;
#endif
			return *this;
		}

		float getDeterminant() const
		{
			assert(0);
		}

	private:

		std::array< Vector4, 4 > Rows;
	};
}