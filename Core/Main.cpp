#include <iostream>
#include <random>
#include <chrono>

#include "../Physics/PhysicsManager.hpp"

using namespace Physics;


int main()
{
	//ugly test code
	using namespace Physics;

	PhysicsManager manager( 4 );

	std::default_random_engine engine;
	//using constant seed to make sure results are equivalent with SSE and FPU math
	//engine.seed( std::chrono::high_resolution_clock::now().time_since_epoch().count() );
	std::uniform_real<float> positionDistribution(-10000, 10000);
	std::uniform_real<float> velocityDistribution(-10, 10);

	int numObjects = 5000;

	for( int i = 0; i < numObjects; ++i )
	{
		Core::Vector4 position(positionDistribution(engine), positionDistribution(engine), positionDistribution(engine));
		Core::Vector4 velocity(velocityDistribution(engine), velocityDistribution(engine), velocityDistribution(engine));

		manager.AddCollisionObject(position, velocity, 100);
	}

	std::chrono::high_resolution_clock::time_point lastTime;
	std::chrono::duration<float> interval;
	int frameCounter = 0;

	//infinite loop, need input handling later - just kill the process
	for (;;)
	{
		interval = std::chrono::high_resolution_clock::now() - lastTime;

		manager.PhysicsFrame(0.1f);

		++frameCounter;

		if (interval.count() >= 1.0f)
		{
			std::cout << frameCounter << " fps" << std::endl;
			frameCounter = 0;
			lastTime = std::chrono::high_resolution_clock::now();
		}
	}

	system("pause");
}