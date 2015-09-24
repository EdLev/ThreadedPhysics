#include "PhysicsManager.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace Physics
{
	using namespace Core;

	PhysicsManager::PhysicsManager(int NumThreads)
		: StateFrontBuffer(&PhysicsStateBuffers[0]),
		StateBackBuffer(&PhysicsStateBuffers[1]),
		StateFrontBufferIndex(0),
		CurrentObjectBuffer(&CollisionObjects),
		CurrentPairsBuffer(&CollisionPairs),
		CollisionDetectionJob(NumThreads, &CurrentObjectBuffer, &CurrentPairsBuffer, this),
		CollisionResolutionJob(NumThreads, &CurrentPairsBuffer, &StateFrontBuffer, this),
		ApplyVelocitiesJob(NumThreads, &StateFrontBuffer, &StateBackBuffer, this),
		CollisionOctree(500)
	{}

	PhysicsManager::~PhysicsManager()
	{}

	bool PhysicsManager::RunFrame(float deltaTime)
	{
		CurrentdeltaTime = deltaTime;

		//copy current state to back buffer
		PhysicsStateBuffers[!StateFrontBufferIndex] = *StateFrontBuffer;

		//CollisionOctree.Rebuild(*CurrentObjectBuffer, *StateFrontBuffer);

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

	void PhysicsManager::ApplyAccelerationsAndImpulses()
	{}

	void PhysicsManager::ApplyVelocities()
	{
		ApplyVelocitiesJob.Work();
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