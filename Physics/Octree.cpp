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

	void Octree::Rebuild(simd_vector<PhysicsObject>& Objects)
	{
		//build array of pointers (safe for now since the manager's array is preemptively sized)
		std::vector<PhysicsObject*> objectPointers;
		objectPointers.reserve(Objects.size());
		std::transform(Objects.begin(), Objects.end(), std::back_inserter(objectPointers), [](auto& object) {return &object; });

		BoundingBox newBounds;
		for (auto& object : Objects)
		{
			newBounds.Min.X = std::min(newBounds.Min.X, object.Position.X - object.CollisionRadius);
			newBounds.Min.Y = std::min(newBounds.Min.Y, object.Position.Y - object.CollisionRadius);
			newBounds.Min.Z = std::min(newBounds.Min.Z, object.Position.Z - object.CollisionRadius);

			newBounds.Max.X = std::max(newBounds.Max.X, object.Position.X + object.CollisionRadius);
			newBounds.Max.Y = std::max(newBounds.Max.Y, object.Position.Y + object.CollisionRadius);
			newBounds.Max.Z = std::max(newBounds.Max.Z, object.Position.Z + object.CollisionRadius);
		}

		Root->Bounds = newBounds;
		BuildSubtree(Root, objectPointers);
	}

	void Octree::BuildSubtree(std::unique_ptr<OctreeNode>& Node, const std::vector<PhysicsObject*>& Objects)
	{
		using namespace std;

		if (Objects.size() <= 1 || (Node->Bounds.Max - Node->Bounds.Min).length3Squared() < 1.0f)
		{
			if (!Objects.empty())
			{
				Node->Objects.push_back(Objects.front());
			}
				
			Node->bLeaf = true;
		}
		else
		{
			Vector4 center = (Node->Bounds.Min + Node->Bounds.Max) / 2.0f;
			Node->Center = center;

			std::vector<PhysicsObject*> childObjects[8];

			for(PhysicsObject* object : Objects)
			{
				//make an index out of a bitmask consisting of the comparison on each axis of position vs center
				int childIndex = 0;
				childIndex |= (object->Position.X < center.X);
				childIndex |= (object->Position.Y < center.Y) << 1;
				childIndex |= (object->Position.Z < center.Z) << 2;
				childObjects[childIndex].push_back(object);

				//for (int cardinal = 0; cardinal < 8; ++cardinal)
				//{
				//	const float Radius = object->CollisionRadius;
				//	Vector4 testPosition(
				//		(cardinal & 1) ? object->Position.X - Radius : object->Position.X + Radius,
				//		(cardinal & 2) ? object->Position.Y - Radius : object->Position.Y + Radius,
				//		(cardinal & 4) ? object->Position.Z - Radius : object->Position.Z + Radius
				//	);

				//	int testIndex = 0;
				//	testIndex |= (testPosition.X < center.X);
				//	testIndex |= (testPosition.Y < center.Y) << 1;
				//	testIndex |= (testPosition.Z < center.Z) << 2;

				//	if (testIndex != childIndex)
				//	{
				//		childObjects[testIndex].push_back(object);
				//	}
				//}
			}

			int childIndex = 0;
			for(auto& Child : Node->Children)
			{
				if (!childObjects[childIndex].empty())
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

					Child = std::make_unique<OctreeNode>(BoundingBox(min, max));
					Child->Parent = Node.get();
					BuildSubtree(Child, childObjects[childIndex]);
				}
				++childIndex;
			}
		}
	}

	void GetNodeColliders(std::unique_ptr<OctreeNode>& Node, Vector4& Position, float Radius, std::vector<PhysicsObject*>& OutObjects)
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
			childIndex |= (Position.X < Node->Center.X);
			childIndex |= (Position.Y < Node->Center.Y) << 1;
			childIndex |= (Position.Z < Node->Center.Z) << 2;
			
			GetNodeColliders(Node->Children[childIndex], Position, Radius, OutObjects);

			for (int cardinal = 0; cardinal < 8; ++cardinal)
			{
				Vector4 testPosition(
					(cardinal & 1) ? Position.X - Radius : Position.X + Radius,
					(cardinal & 2) ? Position.Y - Radius : Position.Y + Radius,
					(cardinal & 4) ? Position.Z - Radius : Position.Z + Radius
					);

				int testIndex = 0;
				testIndex |= (testPosition.X < Node->Center.X);
				testIndex |= (testPosition.Y < Node->Center.Y) << 1;
				testIndex |= (testPosition.Z < Node->Center.Z) << 2;

				if (testIndex != childIndex)
				{
					GetNodeColliders(Node->Children[testIndex], Position, Radius, OutObjects);
				}
			}
		}
	}

	void Octree::GetPotentialColliders(Vector4& Position, float Radius, std::vector<PhysicsObject*>& OutObjects)
	{
		GetNodeColliders(Root, Position, Radius, OutObjects);
	}

}