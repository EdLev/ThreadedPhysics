#include <thread>

#include "TaskFunctions.hpp"
#include "PhysicsManager.hpp"

namespace Physics
{
	void DetectCollisionsWorkerFunction::operator() (simd_vector<CollisionObject>** CollisionObjects, std::vector<std::pair<CollisionObject*, CollisionObject*>>** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager)
	{
		//OutputDebugString("D\n");
		CollisionObject& first = (**CollisionObjects)[CollisionObjectIndex];

		for (auto& second : **CollisionObjects)
		{
			//don't check it against itself
			if (first.PhysicsStateIndex == second.PhysicsStateIndex)
			{
				continue;
			}

			auto& currentPhysicsBuffer = *(Manager->StateFrontBuffer);

			const float distanceSquared = (currentPhysicsBuffer[first.PhysicsStateIndex].Position - currentPhysicsBuffer[second.PhysicsStateIndex].Position).length3Squared();
			const float totalRadiusSquared = (first.Radius + second.Radius) * (first.Radius + second.Radius);
			if (distanceSquared < totalRadiusSquared)
			{
				PairsMutex.lock();
				(**CollisionPairs).push_back(std::make_pair(&first, &second));
				PairsMutex.unlock();
			}
		}
	}

	void ResolveCollisionsWorkerFunction::operator() (decltype(PhysicsManager::CollisionPairs)** CollisionPairs, decltype(PhysicsManager::StateFrontBuffer)* PhysicsStateBuffer, size_t PairIndex, PhysicsManager* Manager)
	{
		using namespace Core;
		auto& backBuffer = Manager->PhysicsStateBuffers[!Manager->StateFrontBufferIndex];
		auto& collisionPair = (**CollisionPairs)[PairIndex];

		const auto& firstState = (**PhysicsStateBuffer)[collisionPair.first->PhysicsStateIndex];
		const auto& secondState = (**PhysicsStateBuffer)[collisionPair.second->PhysicsStateIndex];

		//from second to first
		Vector4 collisionNormal = (firstState.Position - secondState.Position).getNormalized3();

		//only do anything if they're approaching each other (avoid oscillation between interpenetrating spheres)
		if (firstState.Velocity.dot3(collisionNormal) - secondState.Velocity.dot3(collisionNormal) < 0.0f)
		{
			float a1 = firstState.Velocity.dot3(collisionNormal);
			float a2 = secondState.Velocity.dot3(collisionNormal);

			float p = (2.0f * (a1 - a2)) / 2.0f /*m1 + m2, assume 1.0 mass for now*/;

			backBuffer[collisionPair.first->PhysicsStateIndex].Velocity = firstState.Velocity - collisionNormal * p;
			backBuffer[collisionPair.second->PhysicsStateIndex].Velocity = secondState.Velocity + collisionNormal * p;

			Vector4 color(colorDist(engine), colorDist(engine), colorDist(engine), 1.0f);
			collisionPair.first->Color = color;
			collisionPair.second->Color = color;
		}
	}

	void ApplyVelocitiesWorkerFunction::operator () (simd_vector<PhysicsState>** FrontBuffer, simd_vector<PhysicsState>** BackBuffer, size_t StateIndex, PhysicsManager* Manager)
	{
		//Forward Euler for now
		//Don't need to lock - 2 threads with this function will never try to write to the same position in the array
		(**BackBuffer)[StateIndex].Position = (**FrontBuffer)[StateIndex].Position + (**FrontBuffer)[StateIndex].Velocity * Manager->CurrentdeltaTime;
	}
}