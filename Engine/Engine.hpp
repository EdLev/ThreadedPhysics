#pragma once

#include <random>

#include "../Physics/PhysicsManager.hpp"
#include "../OpenGLRenderer/OpenGLRenderer.hpp"
#include "../OpenGLRenderer/SpriteTechnique.hpp"
#include "../PlatformManager/PlatformManager.hpp"

namespace Engine
{
	class Engine
	{
	public:
		Engine(std::shared_ptr<Rendering::OpenGLRenderer> Renderer);

		int MainLoop();

	private:

		void Simulate(float deltaTime);
		void Render();

		std::shared_ptr<Rendering::OpenGLRenderer> Renderer;

		SpriteTechnique SpriteTechnique;
		Physics::PhysicsManager PhysicsManager;
		PlatformManager PlatformManager;

		std::default_random_engine RandomEngine;
		std::uniform_real_distribution<float> PositionDist;
		std::uniform_real_distribution<float> VelocityDist;
		const int NumSpheres;
	};
}
