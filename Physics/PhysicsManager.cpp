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
		CurrentPairsBuffer(&CollisionPairs),
		CollisionDetectionJob(NumThreads, &StateFrontBuffer, &CurrentPairsBuffer, this),
		CollisionResolutionJob(NumThreads, &CurrentPairsBuffer, &StateBackBuffer, this),
		ApplyVelocitiesJob(NumThreads, &StateFrontBuffer, &StateBackBuffer, this),
		CollisionOctree(BoundingBox(Vector4(-1000000, -1000000, -1000000), Vector4(1000000, 1000000, 1000000)))
	{}

	PhysicsManager::~PhysicsManager()
	{}

	bool PhysicsManager::RunFrame(float deltaTime)
	{
		CurrentDeltaTime = deltaTime;

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
		PhysicsObject newObject{ position, velocity, Core::Vector4(0.0f, 0.1f, 0.2f, 1.0f), radius };
		StateFrontBuffer->push_back(newObject);
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

	void PhysicsManager::CopyCurrentPhysicsObjects(simd_vector<PhysicsObject>& outputBuffer)
	{
		CurrentBufferMutex.lock();
		outputBuffer = *StateFrontBuffer;
		CurrentBufferMutex.unlock();
	}
}