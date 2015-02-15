#pragma once

#include <mutex>
#include <random>

#include "Types.hpp"

namespace Physics
{
	struct DetectCollisionsWorkerFunction
	{
		std::mutex PairsMutex;
		void operator () (simd_vector<CollisionObject>** CollisionObjects, std::vector<std::pair<CollisionObject*, CollisionObject*>>** CollisionPairs, size_t CollisionObjectIndex, class PhysicsManager* Manager);
	};

	struct ResolveCollisionsWorkerFunction
	{
		std::default_random_engine engine;
		std::uniform_real_distribution<float> colorDist;

		ResolveCollisionsWorkerFunction()
			: colorDist(0.0f, 1.0f)
		{}

		void operator () (std::vector<std::pair<CollisionObject*, CollisionObject*>>** CollisionPairs, simd_vector<PhysicsState>** PhysicsStateBuffer, size_t PairIndex, PhysicsManager* Manager);
	};

	struct ApplyVelocitiesWorkerFunction
	{
		void operator () (simd_vector<PhysicsState>** FrontBuffer, simd_vector<PhysicsState>** BackBuffer, size_t StateIndex, PhysicsManager* Manager);
	};
}