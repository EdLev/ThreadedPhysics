#ifndef TYPES_HPP
#define TYPES_HPP

#include <vector>

#include "../Core/AlignedAllocator.hpp"
#include "../Core/Vector4.hpp"

template<typename value_type> using simd_vector = std::vector<value_type, aligned_allocator<value_type, 16>>;

namespace Physics
{
	struct PhysicsState
	{
		Core::Vector4 Position;
		Core::Vector4 Velocity;
	};

	struct CollisionObject
	{
		//index into the current state buffer
		size_t PhysicsStateIndex;
		Core::Vector4 Color;
		//just spheres for now, subclass for other shapes later (CollidesWith(other) method)
		float Radius;
	};
}

#endif