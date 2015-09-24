#include "BoundingBox.hpp"

Core::BoundingBox::BoundingBox()
{
}

Core::BoundingBox::BoundingBox(const Vector4& min, const Vector4& max) :
	Min(min),
	Max(max)
{}

bool Core::BoundingBox::Contains(const Vector4& point) const
{
	return(	point.X >= Min.X && point.X < Max.X &&
			point.Y >= Min.Y && point.Y < Max.Y &&
			point.Z >= Min.Z && point.Z < Max.Z);
}

bool Core::BoundingBox::Contains(const Vector4& center, float radius) const
{
	return(	center.X - radius >= Min.X && center.X + radius < Max.X &&
			center.Y - radius >= Min.Y && center.Y + radius < Max.Y &&
			center.Z - radius >= Min.Z && center.Z + radius < Max.Z);
}

bool Core::BoundingBox::Contains(const BoundingBox & other) const
{
	return(	other.Min.X >= Min.X && other.Max.X < Max.X &&
			other.Min.Y >= Min.Y && other.Max.Y < Max.Y &&
			other.Min.Z >= Min.Z && other.Max.Z < Max.Z);
}
