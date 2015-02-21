#pragma once
#include "Types.hpp"

namespace Physics
{
	struct OctreeNode
	{
		simd_vector<CollisionObject> ObjectCopies;
		simd_vector<PhysicsState> StateCopies; //for memory contiguity

		OctreeNode* Children[8];
		Core::Vector4 Center;
		bool bLeaf;

		OctreeNode();
	};

	class Octree
	{
	public:
		Octree(int MaxObjectsPerNode = 10);

		void Rebuild(const simd_vector<CollisionObject>& Objects, const simd_vector<PhysicsState>& States);	
		void AddObject(const CollisionObject* const NewObject, const PhysicsState& NewState);

		void GetPotentialColliders(Core::Vector4& Position, float Radius, simd_vector<CollisionObject*>& OutObjects, simd_vector<PhysicsState*>& OutStates);

	private:

		void BuildNode(OctreeNode* Node, const simd_vector<CollisionObject>& Objects, const simd_vector<PhysicsState>& States);

		OctreeNode Root;
		size_t MaxObjectsPerNode;
		simd_vector<OctreeNode> ChildrenBuffer;
	};
}