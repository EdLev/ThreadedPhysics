#pragma once

#include <vector>
#include <memory>

#include "Types.hpp"
#include "../Core/BoundingBox.hpp"

namespace Physics
{
	struct OctreeNode
	{
		OctreeNode(const Core::BoundingBox& bounds);

		//TODO make this better
		//aligned new and delete for make_unique
		void* operator new(size_t i)
		{
			return _mm_malloc(i,16);
		}

			void operator delete(void* p)
		{
			_mm_free(p);
		}

		OctreeNode* Parent;
		std::unique_ptr<OctreeNode> Children[8];
		Core::Vector4 Center;
		bool bLeaf;
		std::vector<const PhysicsObject*> Objects;
		Core::BoundingBox Bounds;
	};

	class Octree
	{
	public:
		Octree(const Core::BoundingBox& bounds);

		void Rebuild(const simd_vector<PhysicsObject>& Objects);	
		void AddObject(const PhysicsObject* const NewObject);

		void GetPotentialColliders(Core::Vector4& Position, float Radius, std::vector<const PhysicsObject*>& OutObjects);

	private:

		void BuildSubtree(std::unique_ptr<OctreeNode>& Node, const std::vector<const PhysicsObject*>& Objects);

		std::unique_ptr<OctreeNode> Root;
	};
}