#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#include "../Core/Vector4.hpp"
#include "../Core/AlignedAllocator.hpp"

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

		bool PhysicsFrame( float DeltaTime );

		void AddCollisionObject( Core::Vector4& position, Core::Vector4& velocity, float radius );

		void CopyCurrentPhysicsState(simd_vector<PhysicsState>& OutputBuffer);

	private:

		bool DetectCollisions();
		void DetectCollisionsWorkerFunction();
		void ResolveCollisions();
		void ApplyAccelerationsAndImpulses();
		void ApplyVelocities();

		void SwapPhysicsStateBuffers();
		void FinishFrame();

		//set at the beginning of the frame
		float CurrentDeltaTime;

		std::vector<CollisionObject> CollisionObjects;

		//front and back buffers for threading
		simd_vector<PhysicsState> PhysicsStateBuffers[2];
		simd_vector<PhysicsState>* CurrentStateBuffer;
		int CurrentStateBufferIndex;

		//lock when swapping buffers and when copying out the current state
		std::mutex CurrentBufferMutex;
		//lock when writing to back buffer
		std::mutex BackBufferMutex;

		std::vector<std::pair<CollisionObject&, CollisionObject&>> CollisionPairs;
		std::mutex PairsMutex;

		int NumWorkerThreads;
		//used by threads to determine which object to fetch
		std::atomic<unsigned int> CurrentObjectIndex;

		//flags to enable worker threads and have them report their status
		bool bDetectCollisions;
		bool bFinishedDetectingCollisions;
		bool bResolveCollisions;
		bool bFinishedResolvingCollisions;
		bool bApplyAccelerations;
		bool bFinishedApplyingAccelerations;
		bool bApplyVelocities;
		bool bFinishedApplyingVelocities;

		//signal threads to return
		bool bShutdown;

		std::vector<std::thread> WorkerThreads;
	};
}