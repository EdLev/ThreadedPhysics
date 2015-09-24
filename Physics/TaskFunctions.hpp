#pragma once

#include <mutex>
#include <random>

#include "Types.hpp"

namespace Physics
{
	class PhysicsManager;

	typedef std::pair<PhysicsObject*, PhysicsObject*> CollisionPair;

	struct DetectCollisionsWorkerFunction
	{
		std::mutex PairsMutex;
		void operator () (simd_vector<PhysicsObject>** CollisionObjects, std::vector<CollisionPair>** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager);
	};

	struct ResolveCollisionsWorkerFunction
	{
		std::default_random_engine RandomEngine;
		std::uniform_real_distribution<float> colorDist;

		ResolveCollisionsWorkerFunction()
			: colorDist(0.0f, 1.0f)
		{}

		void operator () (std::vector<CollisionPair>** CollisionPairs, simd_vector<PhysicsObject>** FrontBuffer, size_t PairIndex, PhysicsManager* Manager);
	};

	struct ApplyVelocitiesWorkerFunction
	{
		void operator () (simd_vector<PhysicsObject>** FrontBuffer, simd_vector<PhysicsObject>** BackBuffer, size_t StateIndex, PhysicsManager* Manager);
	};
}