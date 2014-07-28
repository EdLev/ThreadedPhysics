#include "PhysicsManager.hpp"

#include <iostream>
#include <sstream>
#include <algorithm>

namespace Physics
{
	PhysicsManager::PhysicsManager(int NumThreads)
		: CurrentStateBuffer(&PhysicsStateBuffers[0]),
		CurrentStateBufferIndex( 0 ),
		CurrentObjectIndex( 0 ),
		NumWorkerThreads(NumThreads),
		bDetectCollisions( false ),
		bFinishedDetectingCollisions( false ),
		bResolveCollisions( false ),
		bFinishedResolvingCollisions( false ),
		bApplyAccelerations( false ),
		bFinishedApplyingAccelerations( false ),
		bApplyVelocities( false ),
		bFinishedApplyingVelocities( false ),
		bShutdown( false )
	{
		//create detection job for each object to collide against all other objects (for now)
		for (int threadIndex = 0; threadIndex < NumWorkerThreads; ++threadIndex)
		{
			WorkerThreads.push_back(std::thread(&PhysicsManager::DetectCollisionsWorkerFunction, this));
		}

		for (auto& workerThread : WorkerThreads)
		{
			//detach threads to allow them to work in the background (and not have to create them each frame)
			workerThread.detach();
		}
	}

	bool PhysicsManager::PhysicsFrame(float DeltaTime)
	{
		CurrentDeltaTime = DeltaTime;

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
		PhysicsState newState = { position, velocity };
		CurrentStateBuffer->push_back(newState);
		CollisionObject newObject = { CurrentStateBuffer->size() - 1, radius };
		CollisionObjects.push_back(newObject);
	}

	void PhysicsManager::DetectCollisionsWorkerFunction()
	{
		while (!bShutdown)
		{
			if (bDetectCollisions)
			{
				unsigned int currentObjectIndex;
				while ((currentObjectIndex = CurrentObjectIndex++) < CollisionObjects.size())
				{
					//std::stringstream ss;
					//ss << currentObjectIndex << std::endl;
					//std::cout << ss.str();

					const CollisionObject& first = CollisionObjects[currentObjectIndex];

					for (const auto& second : CollisionObjects)
					{
						//don't check it against itself
						if (first.PhysicsStateIndex == second.PhysicsStateIndex)
						{
							continue;
						}

						auto& currentPhysicsBuffer = *CurrentStateBuffer;

						const float distanceSquared = (currentPhysicsBuffer[first.PhysicsStateIndex].Position - currentPhysicsBuffer[second.PhysicsStateIndex].Position).length3Squared();
						const float totalRadiusSquared = (first.Radius + second.Radius) * (first.Radius + second.Radius);
						if (distanceSquared < totalRadiusSquared)
						{
							PairsMutex.lock();

							CollisionPairs.push_back(std::make_pair(first, second));

							PairsMutex.unlock();
						}
					}

					if (currentObjectIndex == CollisionObjects.size() - 1)
					{
						bFinishedDetectingCollisions = true;
					}
				}

				//yield if we're out of the loop, to avoid spending time aggressively checking array bounds if we've finished
				std::this_thread::yield();
			}
			else
			{
				//yield to other physics routines
				std::this_thread::yield();
			}
		}
	}

	bool PhysicsManager::DetectCollisions()
	{
		CollisionPairs.clear();
		CurrentObjectIndex = 0;

		bFinishedDetectingCollisions = false;
		bDetectCollisions = true; //allow threads to do work
		while (!bFinishedDetectingCollisions)
		{
			//wait until threads are done
			std::this_thread::yield();
		}
		bDetectCollisions = false;
		return !CollisionPairs.empty();
	}

	void PhysicsManager::ResolveCollisions()
	{
	}

	void PhysicsManager::ApplyAccelerationsAndImpulses()
	{}

	void PhysicsManager::ApplyVelocities()
	{
		simd_vector< PhysicsState >& backBuffer = PhysicsStateBuffers[!CurrentStateBufferIndex];

		std::vector< std::thread > workerThreads;

		CurrentObjectIndex = 0;

		for (int threadIndex = 0; threadIndex < NumWorkerThreads; ++threadIndex)
		{
			//don't really need a separate worker function for this
			workerThreads.push_back(std::thread([this, &backBuffer]()
			{
				int currentIndex;
				while (currentIndex = CurrentObjectIndex++ < CurrentStateBuffer->size())
				{
					//Forward Euler for now
					BackBufferMutex.lock();
					backBuffer[currentIndex].Position = (*CurrentStateBuffer)[currentIndex].Position + (*CurrentStateBuffer)[currentIndex].Velocity * CurrentDeltaTime;
					BackBufferMutex.unlock();
				}
			}));
		}

		for (auto& workerThread : workerThreads)
		{
			workerThread.join();
		}

	}

	void PhysicsManager::CopyCurrentPhysicsState(simd_vector<PhysicsState>& OutputBuffer)
	{
		CurrentBufferMutex.lock();
		OutputBuffer = *CurrentStateBuffer;
		CurrentBufferMutex.unlock();
	}

	PhysicsManager::~PhysicsManager()
	{
		bShutdown = true;
	}
}