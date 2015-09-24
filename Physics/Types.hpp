#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

#include "../Core/AlignedAllocator.hpp"
#include "../Core/Vector4.hpp"

template<typename value_type> using simd_vector = std::vector<value_type, aligned_allocator<value_type, 16>>;

namespace Physics
{
	struct PhysicsObject
	{
		Core::Vector4 Position;
		Core::Vector4 Velocity;
		Core::Vector4 Color;
		//just spheres for now, subclass for other shapes later (CollidesWith(other) method)
		float CollisionRadius;
	};
}

#endif