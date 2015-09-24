#include <thread>

#include "TaskFunctions.hpp"
#include "PhysicsManager.hpp"

namespace Physics
{
	void DetectCollisionsWorkerFunction::operator() (simd_vector<PhysicsObject>** CollisionObjects, std::vector<std::pair<size_t, size_t>>** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager)
	{
		PhysicsObject& first = (**CollisionObjects)[CollisionObjectIndex];

		auto& currentPhysicsBuffer = *(Manager->StateFrontBuffer);

		int stateIndex = 0;
		for (auto& second : **CollisionObjects)
		{
			//don't check it against itself
			if (&first == &second)
			{
				continue;
			}
			
			const float distanceSquared = (first.Position - second.Position).length3Squared();
			const float totalRadiusSquared = (first.CollisionRadius + second.CollisionRadius) * (first.CollisionRadius + second.CollisionRadius);
			if (distanceSquared < totalRadiusSquared)
			{
				PairsMutex.lock();
				(**CollisionPairs).push_back(std::make_pair(CollisionObjectIndex, stateIndex));
				PairsMutex.unlock();
			}
			++stateIndex;
		}
	}

	void ResolveCollisionsWorkerFunction::operator() (decltype(PhysicsManager::CollisionPairs)** CollisionPairs, decltype(PhysicsManager::StateFrontBuffer)* BackBuffer, size_t PairIndex, PhysicsManager* Manager)
	{
		using namespace Core;
		auto& backBuffer = **BackBuffer;
		auto& frontBuffer = *Manager->StateFrontBuffer;
		auto& collisionPair = (**CollisionPairs)[PairIndex];

		auto& firstObject = frontBuffer[collisionPair.first];
		auto& secondObject = frontBuffer[collisionPair.second];

		//from second to first
		Vector4 collisionNormal = (firstObject.Position - secondObject.Position).getNormalized3();

		//only do anything if they're approaching each other (avoid oscillation between interpenetrating spheres)
		if (firstObject.Velocity.dot3(collisionNormal) - secondObject.Velocity.dot3(collisionNormal) < 0.0f)
		{
			float a1 = firstObject.Velocity.dot3(collisionNormal);
			float a2 = secondObject.Velocity.dot3(collisionNormal);

			float p = (2.0f * (a1 - a2)) / 2.0f /*m1 + m2, assume 1.0 mass for now*/;

			backBuffer[collisionPair.first].Velocity = firstObject.Velocity - collisionNormal * p;
			backBuffer[collisionPair.second].Velocity = secondObject.Velocity + collisionNormal * p;

			Vector4 color(colorDist(RandomEngine), colorDist(RandomEngine), colorDist(RandomEngine), 1.0f);
			backBuffer[collisionPair.first].Color = color;
			backBuffer[collisionPair.second].Color = color;
		}
	}

	void ApplyVelocitiesWorkerFunction::operator () (simd_vector<PhysicsObject>** FrontBuffer, simd_vector<PhysicsObject>** BackBuffer, size_t StateIndex, PhysicsManager* Manager)
	{
		//Forward Euler for now
		//Don't need to lock - 2 threads with this function will never try to write to the same position in the array
		(**BackBuffer)[StateIndex].Position = (**FrontBuffer)[StateIndex].Position + (**FrontBuffer)[StateIndex].Velocity * Manager->CurrentDeltaTime;
	}
}