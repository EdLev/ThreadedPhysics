#pragma once

#include <algorithm>

#include "Vector4.hpp"

namespace Core
{
	class BoundingBox
	{
	public:
		BoundingBox(const Vector4& min, const Vector4& max);
		bool Contains(const Vector4& point) const;
		bool Contains(const Vector4& center, float radius) const;
		bool Contains(const BoundingBox& other) const;

		Vector4 Min;
		Vector4 Max;
	};
}