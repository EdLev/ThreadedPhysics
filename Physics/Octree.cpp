#include "Octree.hpp"

#include <algorithm>
#include <iterator>

using namespace Core;
namespace Physics
{
	OctreeNode::OctreeNode(const BoundingBox& bounds) :
		Bounds(bounds)
	{
		std::fill(std::begin(Children), std::end(Children), nullptr);

		bLeaf = false;
	}

	Octree::Octree(const BoundingBox& bounds) :
		Root(std::make_unique<OctreeNode>(bounds))
	{
	}

	void Octree::Rebuild(const simd_vector<PhysicsObject>& Objects)
	{
		//build array of pointers (safe for now since the manager's array is preemptively sized)
		std::vector<const PhysicsObject*> objectPointers;
		std::transform(Objects.begin(), Objects.end(), objectPointers.begin(), [](auto& object) {return &object; });

		BuildSubtree(Root, objectPointers);
	}

	void Octree::BuildSubtree(std::unique_ptr<OctreeNode>& Node, const std::vector<const PhysicsObject*>& Objects)
	{
		using namespace std;

		if (Objects.size() == 1)
		{
			Node->Objects.push_back(Objects.front());
			Node->bLeaf = true;
		}
		else
		{
			Vector4 center = Node->Bounds.Min + Node->Bounds.Max / 2.0f;
			for (int childIndex = 0; childIndex < 8; ++childIndex)
			{
				Vector4 min;
				Vector4 max;

				if ((childIndex & 1) != 0)
				{
					min.X = Node->Bounds.Min.X;
					max.X = center.X;
				}
				else
				{
					min.X = center.X;
					max.X = Node->Bounds.Max.X;
				}

				if ((childIndex & 2) != 0)
				{
					min.Y = Node->Bounds.Min.Y;
					max.Y = center.Y;
				}
				else
				{
					min.Y = center.Y;
					max.Y = Node->Bounds.Max.Y;
				}

				if ((childIndex & 4) != 0)
				{
					min.Z = Node->Bounds.Min.Z;
					max.Z = center.Z;
				}
				else
				{
					min.Z = center.Z;
					max.Z = Node->Bounds.Max.Z;
				}
				

				Node->Children[childIndex] = std::make_unique<OctreeNode>(BoundingBox(min, max));
			}

			std::vector<const PhysicsObject*> childObjects[8];

			size_t objectIndex = 0;
			for(const PhysicsObject* object : Objects)
			{
				//make an index out of a bitmask consisting of the comparison on each axis of position vs center
				int childIndex = 0;
				childIndex |= (object->Position.X < center.X);
				childIndex |= (object->Position.Y < center.Y) << 1;
				childIndex |= (object->Position.Z < center.Z) << 2;
				childObjects[childIndex].push_back(Objects[objectIndex]);

				for (int cardinal = 0; cardinal < 8; ++cardinal)
				{
					const float Radius = object->CollisionRadius;
					Vector4 testPosition(
						(cardinal & 1) ? object->Position.X - Radius : object->Position.X + Radius,
						(cardinal & 2) ? object->Position.Y - Radius : object->Position.Y + Radius,
						(cardinal & 4) ? object->Position.Z - Radius : object->Position.Z + Radius
					);

					int testIndex = 0;
					testIndex |= (testPosition.X < Node->Center.X);
					testIndex |= (testPosition.Y < Node->Center.Y) << 1;
					testIndex |= (testPosition.Z < Node->Center.Z) << 2;

					if (testIndex != childIndex)
					{
						childObjects[testIndex].push_back(Objects[testIndex]);
					}
				}
				
				++objectIndex;
			};

			int childIndex = 0;
			for_each(Node->Children, Node->Children + 8, [&](auto& Child)
			{
				BuildSubtree(Child, childObjects[childIndex]);
				++childIndex;
			});
		}
	}

	void GetNodeColliders(std::unique_ptr<OctreeNode>& Node, Vector4& Position, float Radius, std::vector<const PhysicsObject*>& OutObjects)
	{
		if (Node == nullptr)
			return;

		if (Node->bLeaf)
		{
			OutObjects.insert(OutObjects.end(), Node->Objects.begin(), Node->Objects.end());
		}
		else
		{
			//make an index out of a bitmask consisting of the comparison on each axis of position vs center
			int childIndex = 0;
			childIndex |= (Position.X > Node->Center.X);
			childIndex |= (Position.Y > Node->Center.Y) << 1;
			childIndex |= (Position.Z > Node->Center.Z) << 2;
			
			GetNodeColliders(Node->Children[childIndex], Position, Radius, OutObjects);

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
					GetNodeColliders(Node->Children[testIndex], Position, Radius, OutObjects);
				}
			}
		}
	}

	void Octree::GetPotentialColliders(Vector4& Position, float Radius, std::vector<const PhysicsObject*>& OutObjects)
	{
		GetNodeColliders(Root, Position, Radius, OutObjects);
	}

}