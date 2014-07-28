#include "Vector4.hpp"

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
			Rows[0] = Vector4( 1.0f, 0.0f, 0.0f, 0.0f );
			Rows[1] = Vector4( 0.0f, 1.0f, 0.0f, 0.0f );
			Rows[2] = Vector4( 0.0f, 0.0f, 1.0f, 0.0f );
			Rows[3] = Vector4( 0.0f, 0.0f, 0.0f, 1.0f );
		}

		Matrix4( const Matrix4& other )
			: Rows( other.Rows )
		{}

		Matrix4( std::array< Vector4, 4 > rows )
		{
			Rows = rows;
		}

		Matrix4( const Vector4& row0, const Vector4& row1, const Vector4& row2, const Vector4& row3 )
		{
			Rows[0] = row0;
			Rows[1] = row1;
			Rows[2] = row2;
			Rows[3] = row3;
		}

		Matrix4& operator = ( const Matrix4& other )
		{
			Rows = other.Rows;
			return *this;
		}

		Vector4& operator [] ( const int index )
		{
			return Rows[index];
		}

		Vector4* begin()
		{
			return &Rows[0];
		}

		Vector4* end()
		{
			return &Rows[3] + 1;
		}

		Vector4 column( const int index )
		{
			assert( index >= 0 );
			assert( index <= 3 );

			return Vector4( Rows[0][index], Rows[1][index], Rows[2][index], Rows[3][index] );
		}

		Matrix4& operator += ( const Matrix4& other )
		{
			std::transform( Rows.begin(), Rows.end(), other.Rows.begin(), Rows.begin(), 
				[]( const Vector4& v1, const Vector4& v2 )->Vector4{ return v1 + v2; } );
			return *this;
		}

		Matrix4 operator + ( const Matrix4& other ) const
		{
			return Matrix4(*this) += other;
		}

		Matrix4& operator -= ( const Matrix4& other )
		{
			std::transform( Rows.begin(), Rows.end(), other.Rows.begin(), Rows.begin(), 
				[]( const Vector4& v1, const Vector4& v2 )->Vector4{ return v1 - v2; } );
			return *this;
		}

		Matrix4 operator - ( const Matrix4& other ) const
		{
			return Matrix4(*this) -= other;
		}

		Matrix4& operator *= ( const float scalar )
		{
			std::transform( Rows.begin(), Rows.end(), Rows.begin(),
				[ &scalar ]( const Vector4& v )->Vector4{ return v * scalar; } );

			return *this;
		}

		Matrix4 operator * ( const float scalar ) const
		{
			return Matrix4( *this ) *= scalar;
		}

		Matrix4& operator /= ( const float scalar )
		{
			float reciprocal = 1.0f / scalar;
			std::transform( Rows.begin(), Rows.end(), Rows.begin(),
				[ reciprocal ]( const Vector4& v )->Vector4{ return v * reciprocal; } );

			return *this;
		}

		Matrix4 operator / ( const float scalar ) const
		{
			return Matrix4( *this ) /= scalar;
		}

		//in this case, the non-assignment operator requires a copy to iterate, so the assignment version will be based on it
		Matrix4 operator * ( const Matrix4& other )
		{
			Matrix4 result( *this );
#ifdef VECTORIZATION_SSE
			std::array< Vector4, 4 > columns( other.Rows );
			_MM_TRANSPOSE4_PS( columns[0].xmm, columns[1].xmm, columns[2].xmm, columns[3].xmm );
#else
			std::array< Vector4, 4 > columns = { other.column(0), other.column(1), other.column(2), other.column(3) };
#endif
			for( int rowIndex = 0; rowIndex < 4; ++rowIndex )
			{
				for( int columnIndex = 0; columnIndex < 4; ++columnIndex )
				{
					result.Rows[rowIndex][columnIndex] = Rows[rowIndex].dot4( columns[columnIndex] );
				}
			}
			return result;
		}

		Matrix4& operator *= ( const Matrix4& other )
		{
			*this = *this * other;
			return *this;
		}

		Matrix4& transpose()
		{
#ifdef VECTORIZATION_SSE
			_MM_TRANSPOSE4_PS( Rows[0].xmm, Rows[1].xmm, Rows[2].xmm, Rows[3].xmm );
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

		Matrix4& invert()
		{

		}

		float getDeterminant() const
		{
			
		}

	private:

		std::array< Vector4, 4 > Rows;
	};
}