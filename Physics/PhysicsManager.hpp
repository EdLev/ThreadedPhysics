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

		PhysicsManager(int NumThreads = 1, size_t NumObjects = 5000);
		PhysicsManager(PhysicsManager& other) = delete;
		PhysicsManager(PhysicsManager&& other) = delete;
		~PhysicsManager();

		bool RunFrame(float deltaTime);

		void AddCollisionObject(const Core::Vector4& position, const Core::Vector4& velocity, float radius);

		void CopyCurrentPhysicsObjects(simd_vector<PhysicsObject>& outputBuffer);

	private:

		bool DetectCollisions();
		void ResolveCollisions();
		void ApplyAccelerationsAndImpulses();

		void ApplyVelocities();

		void SwapPhysicsStateBuffers();
		void FinishFrame();

		//set at the beginning of the frame
		float CurrentDeltaTime;

		//front and back buffers for threading
		simd_vector<PhysicsObject> PhysicsStateBuffers[2];
		simd_vector<PhysicsObject>* StateFrontBuffer;
		simd_vector<PhysicsObject>* StateBackBuffer;
		int StateFrontBufferIndex;

		//lock when swapping buffers and when copying out the current state
		std::mutex CurrentBufferMutex;
		//lock when writing to back buffer
		std::mutex BackBufferMutex;

		std::vector<CollisionPair> CollisionPairs;
		decltype(CollisionPairs)* CurrentPairsBuffer;

		friend struct DetectCollisionsWorkerFunction;
		friend struct ResolveCollisionsWorkerFunction;
		friend struct ApplyVelocitiesWorkerFunction;

		Task<simd_vector<PhysicsObject>, std::vector<CollisionPair>, DetectCollisionsWorkerFunction, PhysicsManager> CollisionDetectionJob;
		Task<std::vector<CollisionPair>, simd_vector<PhysicsObject>, ResolveCollisionsWorkerFunction, PhysicsManager> CollisionResolutionJob;
		Task<simd_vector<PhysicsObject>, simd_vector<PhysicsObject>, ApplyVelocitiesWorkerFunction, PhysicsManager> ApplyVelocitiesJob;

		Octree CollisionOctree;
	};
}