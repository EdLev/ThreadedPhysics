#include "Octree.hpp"

#include <algorithm>
#include <iterator>

using namespace Core;
namespace Physics
{
	static const int MaxObjectsInLeaf = 100;

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

	Vector4 CardinalOffset(const Vector4& center, const float radius, const int cardinal)
	{
		return Vector4(
			(cardinal & 1) ? center.X - radius : center.X + radius,
			(cardinal & 2) ? center.Y - radius : center.Y + radius,
			(cardinal & 4) ? center.Z - radius : center.Z + radius
		);
	}

	//build the child box for the child octant
	BoundingBox BuildChildBounds(const BoundingBox& parentBounds, int childIndex)
	{
		Vector4 center = (parentBounds.Min + parentBounds.Max) / 2.0f;
		BoundingBox result;
		if ((childIndex & 1) != 0)
		{
			result.Min.X = parentBounds.Min.X;
			result.Max.X = center.X;
		}
		else
		{
			result.Min.X = center.X;
			result.Max.X = parentBounds.Max.X;
		}

		if ((childIndex & 2) != 0)
		{
			result.Min.Y = parentBounds.Min.Y;
			result.Max.Y = center.Y;
		}
		else
		{
			result.Min.Y = center.Y;
			result.Max.Y = parentBounds.Max.Y;
		}

		if ((childIndex & 4) != 0)
		{
			result.Min.Z = parentBounds.Min.Z;
			result.Max.Z = center.Z;
		}
		else
		{
			result.Min.Z = center.Z;
			result.Max.Z = parentBounds.Max.Z;
		}

		return result;
	}

	int SelectChild(const Vector4& parentCenter, const Vector4& position)
	{
		//make an index out of a bitmask consisting of the comparison on each axis of position vs center
		int childIndex = 0;
		childIndex |= (position.X < parentCenter.X);
		childIndex |= (position.Y < parentCenter.Y) << 1;
		childIndex |= (position.Z < parentCenter.Z) << 2;

		return childIndex;
	}

	void Octree::BuildSubtree(std::unique_ptr<OctreeNode>& Node, const std::vector<PhysicsObject*>& Objects)
	{
		using namespace std;

		if (Objects.size() <= MaxObjectsInLeaf || (Node->Bounds.Max - Node->Bounds.Min).length3Squared() < 1.0f)
		{
			if (!Objects.empty())
			{
				Node->Objects.insert(Node->Objects.end(), Objects.begin(), Objects.end());
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
				int childIndex = SelectChild(center, object->Position);
				childObjects[childIndex].push_back(object);

				bool alreadyAdded[8];
				std::fill(std::begin(alreadyAdded), std::end(alreadyAdded), false);

				//also add to any nodes overlapped by the radius
				for (int cardinal = 0; cardinal < 8; ++cardinal)
				{
					const float radius = object->CollisionRadius;
					Vector4 testPosition(CardinalOffset(object->Position, radius, cardinal));

					int testIndex = SelectChild(center, testPosition);

					if (testIndex != childIndex && !alreadyAdded[testIndex])
					{
						childObjects[testIndex].push_back(object);
						alreadyAdded[testIndex] = true;
					}
				}
			}

			int childIndex = 0;
			for(auto& Child : Node->Children)
			{
				if (!childObjects[childIndex].empty())
				{
					Child = std::make_unique<OctreeNode>(BuildChildBounds(Node->Bounds, childIndex));
					Child->Parent = Node.get();
					BuildSubtree(Child, childObjects[childIndex]);
				}
				++childIndex;
			}
		}
	}

	void GetNodeColliders(std::unique_ptr<OctreeNode>& node, Vector4& position, float radius, std::vector<PhysicsObject*>& OutObjects)
	{
		if (node == nullptr)
			return;

		if (node->bLeaf)
		{
			OutObjects.insert(OutObjects.end(), node->Objects.begin(), node->Objects.end());
		}
		else
		{
			int childIndex = SelectChild(node->Center, position);			
			GetNodeColliders(node->Children[childIndex], position, radius, OutObjects);

			//handle tests near node edges
			for (int cardinal = 0; cardinal < 8; ++cardinal)
			{
				Vector4 testPosition(CardinalOffset(position, radius, cardinal));

				int testIndex = SelectChild(node->Center, testPosition);

				if (testIndex != childIndex)
				{
					GetNodeColliders(node->Children[testIndex], position, radius, OutObjects);
				}
			}
		}
	}

	void Octree::GetPotentialColliders(Vector4& Position, float Radius, std::vector<PhysicsObject*>& OutObjects)
	{
		OutObjects.reserve(MaxObjectsInLeaf);
		GetNodeColliders(Root, Position, Radius, OutObjects);
	}

}