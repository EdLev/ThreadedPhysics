#include <thread>

#include "TaskFunctions.hpp"
#include "PhysicsManager.hpp"

namespace Physics
{
	void DetectCollisionsWorkerFunction::operator() (simd_vector<PhysicsObject>** CollisionObjects, std::vector<CollisionPair>** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager)
	{
		PhysicsObject& first = (**CollisionObjects)[CollisionObjectIndex];

		int stateIndex = 0;
		
		std::vector<PhysicsObject*> potentialColliders;
		Manager->CollisionOctree.GetPotentialColliders(first.Position, first.CollisionRadius, potentialColliders);
		for (auto second : potentialColliders)
		{
			//don't check it against itself
			if (&first == second)
			{
				continue;
			}
			
			const float distanceSquared = (first.Position - second->Position).length3Squared();
			const float totalRadiusSquared = (first.CollisionRadius + second->CollisionRadius) * (first.CollisionRadius + second->CollisionRadius);
			if (distanceSquared < totalRadiusSquared)
			{
				PairsMutex.lock();
				(**CollisionPairs).push_back(std::make_pair(&first, second));
				PairsMutex.unlock();
			}
			++stateIndex;
		}
	}

	void ResolveCollisionsWorkerFunction::operator() (decltype(PhysicsManager::CollisionPairs)** CollisionPairs, decltype(PhysicsManager::StateFrontBuffer)* BackBuffer, size_t PairIndex, PhysicsManager* Manager)
	{
		using namespace Core;
		auto& backBuffer = **BackBuffer;
		auto& collisionPair = (**CollisionPairs)[PairIndex];

		auto firstObject = collisionPair.first;
		auto secondObject = collisionPair.second;

		//from second to first
		Vector4 collisionNormal = (firstObject->Position - secondObject->Position).getNormalized3();

		//only do anything if they're approaching each other (avoid oscillation between interpenetrating spheres)
		if (firstObject->Velocity.dot3(collisionNormal) - secondObject->Velocity.dot3(collisionNormal) < 0.0f)
		{
			float a1 = firstObject->Velocity.dot3(collisionNormal);
			float a2 = secondObject->Velocity.dot3(collisionNormal);

			float p = (2.0f * (a1 - a2)) / 2.0f /*m1 + m2, assume 1.0 mass for now*/;

			backBuffer[firstObject->Index].Velocity = firstObject->Velocity - collisionNormal * p;
			backBuffer[secondObject->Index].Velocity = secondObject->Velocity + collisionNormal * p;

			Vector4 color(colorDist(RandomEngine), colorDist(RandomEngine), colorDist(RandomEngine), 1.0f);
			backBuffer[firstObject->Index].Color = color;
			backBuffer[secondObject->Index].Color = color;
		}
	}

	void ApplyVelocitiesWorkerFunction::operator () (simd_vector<PhysicsObject>** FrontBuffer, simd_vector<PhysicsObject>** BackBuffer, size_t StateIndex, PhysicsManager* Manager)
	{
		//Forward Euler for now
		//Don't need to lock - 2 threads with this function will never try to write to the same position in the array
		Core::Vector4 position((**FrontBuffer)[StateIndex].Position);
		(**BackBuffer)[StateIndex].Position = (**FrontBuffer)[StateIndex].Position + (**FrontBuffer)[StateIndex].Velocity * Manager->CurrentDeltaTime;
		//TODO this is a hack for testing
		(**BackBuffer)[StateIndex].Velocity -= position.getNormalized3() * 0.01f;
	}
}