#include "Octree.hpp"

#include <algorithm>
#include <iterator>

using namespace Core;
namespace Physics
{
	OctreeNode::OctreeNode()
	{
		for (int childIndex = 0; childIndex < 8; ++childIndex)
		{
			Children[childIndex] = nullptr;
		}

		bLeaf = false;
	}

	Octree::Octree(int MaxObjectsPerNode)
	{
		this->MaxObjectsPerNode = MaxObjectsPerNode;
		ChildrenBuffer.reserve(10000);
	}

	void Octree::Rebuild(const simd_vector<CollisionObject>& Objects, const simd_vector<PhysicsState>& States)
	{
		BuildNode(&Root, Objects, States);
	}

	void Octree::BuildNode(OctreeNode* Node, const simd_vector<CollisionObject>& Objects, const simd_vector<PhysicsState>& States)
	{
		using namespace std;

		if (Objects.size() < MaxObjectsPerNode)
		{
			Node->ObjectCopies = Objects;
			Node->StateCopies = States;
			Node->bLeaf = true;
		}
		else
		{
			for (int childIndex = 0; childIndex < 8; ++childIndex)
			{
				ChildrenBuffer.emplace_back();
			};

			//separate loop due to emplace_back reallocating the vector
			for (int childIndex = 0; childIndex < 8; ++childIndex)
			{
				Node->Children[childIndex] = &ChildrenBuffer.back() - 7 + childIndex;
			}

			Vector4 minPosition;
			Vector4 maxPosition;
			for_each(States.begin(), States.end(), [&](const PhysicsState& State)
			{
				minPosition.X = min(State.Position.X, minPosition.X);
				minPosition.Y = min(State.Position.Y, minPosition.Y);
				minPosition.Z = min(State.Position.Z, minPosition.Z);

				maxPosition.X = max(State.Position.X, maxPosition.X);
				maxPosition.Y = max(State.Position.Y, maxPosition.Y);
				maxPosition.Z = max(State.Position.Z, maxPosition.Z);
			});

			Node->Center = minPosition + (maxPosition - minPosition) / 2.0f;
			simd_vector<PhysicsState> childStates[8];
			simd_vector<CollisionObject> childObjects[8];

			for (int i = 0; i < 8; ++i)
			{
				childStates[i].reserve(100);
				childObjects[i].reserve(100);
			}

			size_t objectIndex = 0;
			for_each(States.begin(), States.end(), [&](const PhysicsState& State)
			{
				//make an index out of a bitmask consisting of the comparison on each axis of position vs center
				int childIndex = 0;
				childIndex |= (State.Position.X > Node->Center.X);
				childIndex |= (State.Position.Y > Node->Center.Y) << 1;
				childIndex |= (State.Position.Z > Node->Center.Z) << 2;
				childStates[childIndex].push_back(State);
				childObjects[childIndex].push_back(Objects[objectIndex]);

				for (int cardinal = 0; cardinal < 8; ++cardinal)
				{
					const float Radius = Objects[objectIndex].Radius;
					Vector4 testPosition(
						(cardinal & 1) ? State.Position.X + Radius : State.Position.X - Radius,
						(cardinal & 2) ? State.Position.Y + Radius : State.Position.Y - Radius,
						(cardinal & 4) ? State.Position.Z + Radius : State.Position.Z - Radius
					);

					int testIndex = 0;
					testIndex |= (testPosition.X > Node->Center.X);
					testIndex |= (testPosition.Y > Node->Center.Y) << 1;
					testIndex |= (testPosition.Z > Node->Center.Z) << 2;

					if (testIndex != childIndex)
					{
						childStates[testIndex].push_back(State);
						childObjects[testIndex].push_back(Objects[testIndex]);
					}
				}
				
				++objectIndex;
			});

			int childIndex = 0;
			for_each(Node->Children, Node->Children + 8, [&](OctreeNode* Child)
			{
				BuildNode(Child, childObjects[childIndex], childStates[childIndex]);
				++childIndex;
			});
		}
	}

	void GetNodeColliders(OctreeNode* Node, Vector4& Position, float Radius, simd_vector<CollisionObject*>& OutObjects, simd_vector<PhysicsState*>& OutStates)
	{
		if (Node == nullptr)
			return;

		if (Node->bLeaf)
		{
			std::transform(Node->ObjectCopies.begin(), Node->ObjectCopies.end(), std::back_inserter(OutObjects), [](CollisionObject& object)->CollisionObject*
			{
				return &object;
			});

			std::transform(Node->StateCopies.begin(), Node->StateCopies.end(), std::back_inserter(OutStates), [](PhysicsState& state)->PhysicsState*
			{
				return &state;
			});
		}
		else
		{
			//make an index out of a bitmask consisting of the comparison on each axis of position vs center
			int childIndex = 0;
			childIndex |= (Position.X > Node->Center.X);
			childIndex |= (Position.Y > Node->Center.Y) << 1;
			childIndex |= (Position.Z > Node->Center.Z) << 2;
			
			GetNodeColliders(Node->Children[childIndex], Position, Radius, OutObjects, OutStates);

			for (int cardinal = 0; cardinal < 8; ++cardinal)
			{
				Vector4 testPosition(
					(cardinal & 1) ? Position.X + Radius : Position.X - Radius,
					(cardinal & 2) ? Position.Y + Radius : Position.Y - Radius,
					(cardinal & 4) ? Position.Z + Radius : Position.Z - Radius
					);

				int testIndex = 0;
				testIndex |= (testPosition.X > Node->Center.X);
				testIndex |= (testPosition.Y > Node->Center.Y) << 1;
				testIndex |= (testPosition.Z > Node->Center.Z) << 2;

				if (testIndex != childIndex)
				{
					GetNodeColliders(Node->Children[testIndex], Position, Radius, OutObjects, OutStates);
				}
			}
		}
	}

	void Octree::GetPotentialColliders(Vector4& Position, float Radius, simd_vector<CollisionObject*>& OutObjects, simd_vector<PhysicsState*>& OutStates)
	{
		GetNodeColliders(&Root, Position, Radius, OutObjects, OutStates);
	}

}