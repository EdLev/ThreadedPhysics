#pragma once

#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <random>

#include "../Core/Matrix4.hpp"
#include "../Core/AlignedAllocator.hpp"
#include "../Core/Task.hpp"

#include "Types.hpp"
#include "TaskFunctions.hpp"
#include "Octree.hpp"

namespace Physics
{
	class PhysicsManager
	{
	public:

		PhysicsManager(int NumThreads = 1);
		PhysicsManager(PhysicsManager& other) = delete;
		PhysicsManager(PhysicsManager&& other) = delete;
		~PhysicsManager();

		bool RunFrame(float deltaTime);

		void AddCollisionObject(Core::Vector4& position, Core::Vector4& velocity, float radius);

		void CopyCurrentPhysicsState(simd_vector<PhysicsState>& outputBuffer);
		void CopyCollisionObjects(simd_vector<CollisionObject>& outputBuffer);

	private:

		bool DetectCollisions();
		void ResolveCollisions();
		void ApplyAccelerationsAndImpulses();

		void ApplyVelocities();

		void SwapPhysicsStateBuffers();
		void FinishFrame();

		//set at the beginning of the frame
		float CurrentdeltaTime;

		simd_vector<CollisionObject> CollisionObjects;
		decltype(CollisionObjects)* CurrentObjectBuffer;

		//front and back buffers for threading
		simd_vector<PhysicsState> PhysicsStateBuffers[2];
		simd_vector<PhysicsState>* StateFrontBuffer;
		simd_vector<PhysicsState>* StateBackBuffer;
		int StateFrontBufferIndex;

		//lock when swapping buffers and when copying out the current state
		std::mutex CurrentBufferMutex;
		//lock when writing to back buffer
		std::mutex BackBufferMutex;

		std::vector<std::pair<CollisionObject*, CollisionObject*>> CollisionPairs;
		decltype(CollisionPairs)* CurrentPairsBuffer;

		friend struct DetectCollisionsWorkerFunction;
		friend struct ResolveCollisionsWorkerFunction;
		friend struct ApplyVelocitiesWorkerFunction;

		Task<decltype(CollisionObjects), decltype(CollisionPairs), DetectCollisionsWorkerFunction, PhysicsManager> CollisionDetectionJob;
		Task<decltype(CollisionPairs), simd_vector<PhysicsState>, ResolveCollisionsWorkerFunction, PhysicsManager> CollisionResolutionJob;
		Task<simd_vector<PhysicsState>, simd_vector<PhysicsState>, ApplyVelocitiesWorkerFunction, PhysicsManager> ApplyVelocitiesJob;

		Octree CollisionOctree;
	};
}