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
		: CurrentStateBuffer( &PhysicsStateBuffers[0] ),
		CurrentStateBufferIndex( 0 ),
		CurrentCollisionPairIndex( 0 ),
		NumWorkerThreads( NumThreads ),
		bResolveCollisions( false ),
		bFinishedResolvingCollisions( false ),
		bApplyAccelerations( false ),
		bFinishedApplyingAccelerations( false ),
		bApplyVelocities( false ),
		bFinishedApplyingVelocities( false ),
		bShutdown( false ),
		CurrentObjectBuffer( &CollisionObjects ),
		CurrentPairsBuffer( &CollisionPairs ),
		CollisionDetectionJob( 4, &CurrentObjectBuffer, &CurrentPairsBuffer, &DetectCollisionsWorkerFunction, this ),
		NumFinishedThreads( 0 )
	{
		//create detection job for each object to collide against all other objects (for now)
		for (int threadIndex = 0; threadIndex < NumWorkerThreads; ++threadIndex)
		{
			WorkerThreads.push_back(std::thread(&PhysicsManager::ResolveCollisionsWorkerFunction, this));
			WorkerThreads.push_back(std::thread(&PhysicsManager::ApplyVelocitiesWorkerFunction, this));
		}

		for (auto& workerThread : WorkerThreads)
		{
			//detach threads to allow them to work in the background (and not have to create them each frame)
			workerThread.detach();
		}
	}

	PhysicsManager::~PhysicsManager()
	{
		bShutdown = true;

		while (NumFinishedThreads < WorkerThreads.size())
		{
			std::this_thread::yield();
		}
	}

	bool PhysicsManager::PhysicsFrame(float deltaTime)
	{
		CurrentdeltaTime = deltaTime;

		//copy current state to back buffer
		PhysicsStateBuffers[!CurrentStateBufferIndex] = *CurrentStateBuffer;

		bool result = DetectCollisions();
		ResolveCollisions();
		ApplyAccelerationsAndImpulses();
		ApplyVelocities();

		//lock in case someone else is trying to copy out the current state right now
		CurrentBufferMutex.lock();
		CurrentStateBufferIndex = !CurrentStateBufferIndex;
		CurrentStateBuffer = &PhysicsStateBuffers[CurrentStateBufferIndex];
		CurrentBufferMutex.unlock();

		return result;
	}

	void PhysicsManager::AddCollisionObject(Core::Vector4& position, Core::Vector4& velocity, float radius)
	{
		CollisionObjects.reserve(100);
		PhysicsState newState = { position, velocity };
		CurrentStateBuffer->push_back(newState);
		//CollisionObject newObject = ;
		CollisionObjects.push_back({ CurrentStateBuffer->size() - 1, Core::Vector4(1.0f, 0.0f, 0.0f, 1.0f), radius });
	}

	std::mutex PairsMutex;

	void DetectCollisionsWorkerFunction( decltype(PhysicsManager::CollisionObjects)** CollisionObjects, decltype(PhysicsManager::CollisionPairs)** CollisionPairs, size_t CollisionObjectIndex, PhysicsManager* Manager )
	{
		CollisionObject& first = (**CollisionObjects)[CollisionObjectIndex];

		for (auto& second : **CollisionObjects)
		{
			//don't check it against itself
			if (first.PhysicsStateIndex == second.PhysicsStateIndex)
			{
				continue;
			}

			auto& currentPhysicsBuffer = *(Manager->CurrentStateBuffer);

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
		CurrentCollisionPairIndex = 0;

		bFinishedResolvingCollisions = false;
		bResolveCollisions = true;
		while (!bFinishedResolvingCollisions)
		{
			std::this_thread::yield();
		}
		bResolveCollisions = false;
	}

	std::default_random_engine engine;
	std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);

	void PhysicsManager::ResolveCollisionsWorkerFunction()
	{
		while (!bShutdown)
		{
			if (bResolveCollisions)
			{
				if (CollisionPairs.empty())
				{
					bFinishedResolvingCollisions = true;
					std::this_thread::yield();
					continue;
				}

				//OutputDebugString("R");

				const auto& currentPhysicsBuffer = *CurrentStateBuffer;
				auto& backBuffer = PhysicsStateBuffers[!CurrentStateBufferIndex];

				unsigned int currentPairIndex;
				while ((currentPairIndex = CurrentCollisionPairIndex++) < CollisionPairs.size())
				{
					auto& collisionPair = CollisionPairs[currentPairIndex];

					//from second to first
					Vector4 collisionNormal = (currentPhysicsBuffer[collisionPair.first->PhysicsStateIndex].Position - currentPhysicsBuffer[collisionPair.second->PhysicsStateIndex].Position).getNormalized3();

					//only do anything if they're approaching each other (avoid oscillation between interpenetrating spheres)
					if (currentPhysicsBuffer[collisionPair.first->PhysicsStateIndex].Velocity.dot3(collisionNormal) - currentPhysicsBuffer[collisionPair.second->PhysicsStateIndex].Velocity.dot3(collisionNormal) < 0.0f)
					{
						float a1 = currentPhysicsBuffer[collisionPair.first->PhysicsStateIndex].Velocity.dot3(collisionNormal);
						float a2 = currentPhysicsBuffer[collisionPair.second->PhysicsStateIndex].Velocity.dot3(collisionNormal);

						float p = (2.0f * (a1 - a2)) / 2.0f /*m1 + m2, assume 1.0 mass for now*/;

						backBuffer[collisionPair.first->PhysicsStateIndex].Velocity = currentPhysicsBuffer[collisionPair.first->PhysicsStateIndex].Velocity - collisionNormal * p;
						backBuffer[collisionPair.second->PhysicsStateIndex].Velocity = currentPhysicsBuffer[collisionPair.second->PhysicsStateIndex].Velocity + collisionNormal * p;

						Vector4 color(colorDist(engine), colorDist(engine), colorDist(engine), 1.0f);
						collisionPair.first->Color = color;
						collisionPair.second->Color = color;
					}

					if (currentPairIndex == CollisionPairs.size() - 1)
					{
						bFinishedResolvingCollisions = true;
					}
				}

				//does this risk setting the flag early?
				bFinishedResolvingCollisions = true;

				//yield if we're out of the loop, to avoid spending time aggressively checking array bounds if we've finished
				std::this_thread::yield();
			}
			else
			{
				//yield to other physics routines
				std::this_thread::yield();
			}
		}

		++NumFinishedThreads;
	}

	void PhysicsManager::ApplyAccelerationsAndImpulses()
	{}

	void PhysicsManager::ApplyVelocities()
	{
		CurrentPhysicsStateIndex = 0;

		bFinishedApplyingVelocities = false;
		bApplyVelocities = true;

		while (!bFinishedApplyingVelocities)
		{
			std::this_thread::yield();
		}

		bApplyVelocities = false;
	}

	void PhysicsManager::ApplyVelocitiesWorkerFunction()
	{
		while (!bShutdown)
		{
			if (bApplyVelocities)
			{
				unsigned int currentIndex;
				while ((currentIndex = CurrentPhysicsStateIndex++) < CurrentStateBuffer->size())
				{
					auto& backBuffer = PhysicsStateBuffers[!CurrentStateBufferIndex];

					//Forward Euler for now
					//BackBufferMutex.lock();
					backBuffer[currentIndex].Position = (*CurrentStateBuffer)[currentIndex].Position + (*CurrentStateBuffer)[currentIndex].Velocity * CurrentdeltaTime;
					//BackBufferMutex.unlock();
				}

				//does this risk setting the flag early?
				bFinishedApplyingVelocities = true;

				//yield if we're out of the loop, to avoid spending time aggressively checking array bounds if we've finished
				std::this_thread::yield();
			}
			else
			{
				//yield to other physics routines
				std::this_thread::yield();
			}
		}

		++NumFinishedThreads;
	}

	void PhysicsManager::CopyCurrentPhysicsState(simd_vector<PhysicsState>& outputBuffer)
	{
		CurrentBufferMutex.lock();
		outputBuffer = *CurrentStateBuffer;
		CurrentBufferMutex.unlock();
	}

	void PhysicsManager::CopyCollisionObjects(simd_vector<CollisionObject>& outputBuffer)
	{
		outputBuffer = CollisionObjects;
	}


}