#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "../Core/Matrix4.hpp"
#include "../Core/AlignedAllocator.hpp"
#include "../Core/BackgroundJob.hpp"

namespace Physics
{
	struct PhysicsState
	{
		Core::Vector4 Position;
		Core::Vector4 Velocity;
	};

	struct CollisionObject
	{
		//index into the current state buffer
		size_t PhysicsStateIndex;
		Core::Vector4 Color;
		//just spheres for now, subclass for other shapes later (CollidesWith(other) method)
		float Radius;
	};

	class PhysicsManager
	{
	public:

		PhysicsManager( int NumThreads = 1 );
		PhysicsManager(PhysicsManager& other) = delete;
		PhysicsManager(PhysicsManager&& other) = delete;
		~PhysicsManager();

		bool PhysicsFrame( float deltaTime );

		void AddCollisionObject( Core::Vector4& position, Core::Vector4& velocity, float radius );

		void CopyCurrentPhysicsState(simd_vector<PhysicsState>& outputBuffer);
		void CopyCollisionObjects(simd_vector<CollisionObject>& outputBuffer);

	private:

		bool DetectCollisions();
		//void DetectCollisionsWorkerFunction();

		void ResolveCollisions();
		void ResolveCollisionsWorkerFunction();

		void ApplyAccelerationsAndImpulses();

		void ApplyVelocities();
		void ApplyVelocitiesWorkerFunction();

		void SwapPhysicsStateBuffers();
		void FinishFrame();

		//set at the beginning of the frame
		float CurrentdeltaTime;

		simd_vector<CollisionObject> CollisionObjects;
		decltype(CollisionObjects)* CurrentObjectBuffer;

		//front and back buffers for threading
		simd_vector<PhysicsState> PhysicsStateBuffers[2];
		simd_vector<PhysicsState>* CurrentStateBuffer;
		int CurrentStateBufferIndex;

		//lock when swapping buffers and when copying out the current state
		std::mutex CurrentBufferMutex;
		//lock when writing to back buffer
		std::mutex BackBufferMutex;

		std::vector<std::pair<CollisionObject*, CollisionObject*>> CollisionPairs;
		decltype(CollisionPairs)* CurrentPairsBuffer;
		std::mutex PairsMutex;

		int NumWorkerThreads;
		std::vector<std::thread> WorkerThreads;

		//signal threads to return
		bool bShutdown;

		//used by threads to determine which object to fetch
		std::atomic<unsigned int> CurrentCollisionPairIndex;
		std::atomic<unsigned int> CurrentPhysicsStateIndex;

		//flags to enable worker threads and have them report their status
		bool bResolveCollisions;
		bool bFinishedResolvingCollisions;
		bool bApplyAccelerations;
		bool bFinishedApplyingAccelerations;
		bool bApplyVelocities;
		bool bFinishedApplyingVelocities;

		std::atomic<unsigned int> NumFinishedThreads;

		friend void DetectCollisionsWorkerFunction(decltype(CollisionObjects)**, decltype(CollisionPairs)**, size_t, PhysicsManager*);

		BackgroundJob<decltype(CollisionObjects), decltype(CollisionPairs), decltype(DetectCollisionsWorkerFunction), PhysicsManager> CollisionDetectionJob;
	};
}