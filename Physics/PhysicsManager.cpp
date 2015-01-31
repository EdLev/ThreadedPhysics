#include "PhysicsManager.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>

#include <Windows.h>

namespace Physics
{
	using namespace Core;

	PhysicsManager::PhysicsManager( int NumThreads )
		: StateFrontBuffer(&PhysicsStateBuffers[0]),
		StateBackBuffer(&PhysicsStateBuffers[1]),
		StateFrontBufferIndex( 0 ),
		CurrentObjectBuffer( &CollisionObjects ),
		CurrentPairsBuffer( &CollisionPairs ),
		CollisionDetectionJob( NumThreads, &CurrentObjectBuffer, &CurrentPairsBuffer, &DetectCollisionsWorkerFunction, this ),
		CollisionResolutionJob( NumThreads, &CurrentPairsBuffer, &StateFrontBuffer, &ResolveCollisionsWorkerFunction, this ),
		ApplyVelocitiesJob( NumThreads, &StateFrontBuffer, &StateBackBuffer, &ApplyVelocitiesWorkerFunction, this )
	{}

	PhysicsManager::~PhysicsManager()
	{}

	bool PhysicsManager::RunFrame(float deltaTime)
	{
		CurrentdeltaTime = deltaTime;

		//copy current state to back buffer
		PhysicsStateBuffers[!StateFrontBufferIndex] = *StateFrontBuffer;

		bool result = DetectCollisions();
		ResolveCollisions();
		ApplyAccelerationsAndImpulses();
		ApplyVelocities();

		//lock in case someone else is trying to copy out the current state right now
		CurrentBufferMutex.lock();
		StateFrontBufferIndex = !StateFrontBufferIndex;
		StateFrontBuffer = &PhysicsStateBuffers[StateFrontBufferIndex];
		StateBackBuffer = &PhysicsStateBuffers[!StateFrontBufferIndex];
		CurrentBufferMutex.unlock();

		return result;
	}

	void PhysicsManager::AddCollisionObject(Core::Vector4& position, Core::Vector4& velocity, float radius)
	{
		CollisionObjects.reserve(100);
		PhysicsState newState = { position, velocity };
		StateFrontBuffer->push_back(newState);
		//CollisionObject newObject = ;
		CollisionObjects.push_back({ StateFrontBuffer->size() - 1, Core::Vector4(1.0f, 0.0f, 0.0f, 1.0f), radius });
	}

	std::mutex PairsMutex;

	void DetectCollisionsWorkerFunction( decltype(PhysicsManager::CollisionObjects)** CollisionObjects, decltype(PhysicsManager::CollisionPairs)** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager )
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

	bool PhysicsManager::DetectCollisions()
	{
		CollisionPairs.clear();
		CollisionDetectionJob.Work();
		return true;
	}

	void PhysicsManager::ResolveCollisions()
	{
		CollisionResolutionJob.Work();
	}

	std::default_random_engine engine;
	std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

	void ResolveCollisionsWorkerFunction(decltype(PhysicsManager::CollisionPairs)** CollisionPairs, decltype(PhysicsManager::StateFrontBuffer)* PhysicsStateBuffer, size_t PairIndex, PhysicsManager* Manager)
	{
		//OutputDebugString("R\n");

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

	void PhysicsManager::ApplyAccelerationsAndImpulses()
	{}

	void PhysicsManager::ApplyVelocities()
	{
		ApplyVelocitiesJob.Work();
	}

	void ApplyVelocitiesWorkerFunction(decltype(PhysicsManager::StateFrontBuffer)* StateFrontBuffer, decltype(PhysicsManager::StateFrontBuffer)* BackBuffer, size_t StateIndex, PhysicsManager* Manager)
	{
		//Forward Euler for now
		//Don't need to lock - 2 threads with this function will never try to write to the same position in the array
		(**BackBuffer)[StateIndex].Position = (**StateFrontBuffer)[StateIndex].Position + (**StateFrontBuffer)[StateIndex].Velocity * Manager->CurrentdeltaTime;
	}

	void PhysicsManager::CopyCurrentPhysicsState(simd_vector<PhysicsState>& outputBuffer)
	{
		CurrentBufferMutex.lock();
		outputBuffer = *StateFrontBuffer;
		CurrentBufferMutex.unlock();
	}

	void PhysicsManager::CopyCollisionObjects(simd_vector<CollisionObject>& outputBuffer)
	{
		outputBuffer = CollisionObjects;
	}


}