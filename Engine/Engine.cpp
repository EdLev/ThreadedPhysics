#include "Engine.hpp"

#include <GL/glew.h>
#include <GL/wglew.h>

#include "../OpenGLRenderer/OpenGLRenderer.hpp"

using namespace std;
using namespace Core;
using namespace Physics;

namespace Engine
{
	Engine::Engine(std::shared_ptr<Rendering::OpenGLRenderer> InRenderer) :
		PhysicsManager(16),
		Renderer(InRenderer),
		RandomEngine((unsigned int)chrono::high_resolution_clock::now().time_since_epoch().count()),
		PositionDist(-500.0f, 500.0f),
		VelocityDist(-10.0f, 10.0f),
		NumSpheres(5000)
	{
		for (int i = 0; i < NumSpheres; ++i)
		{
			PhysicsManager.AddCollisionObject(Vector4(PositionDist(RandomEngine), PositionDist(RandomEngine), PositionDist(RandomEngine)),
				Vector4(VelocityDist(RandomEngine), VelocityDist(RandomEngine), VelocityDist(RandomEngine)),
				5.0f);
		}
	}

	int Engine::MainLoop()
	{

		chrono::time_point<chrono::high_resolution_clock> lastTime = chrono::high_resolution_clock::now();
		chrono::duration<float> second(0);
		unsigned int frameCounter = 0;

		bool bRunning = true;
		while (bRunning)
		{
			if (PlatformManager.PumpMessage() == Quit)
			{
				bRunning = false;
			}
			else
			{
				chrono::duration<float> frame(0);

				while (frame.count() < 1.0f / 60.0f)
				{
					chrono::duration<float> interval(chrono::high_resolution_clock::now() - lastTime);
					lastTime = chrono::high_resolution_clock::now();
					Simulate(interval.count());
					frame += interval;
					second += interval;
					++frameCounter;
				}

				frame = frame.zero();

				Render();

				char str[16];

				if (second.count() >= 1.0f)
				{
					sprintf_s(str, "%d\n", frameCounter);
					OutputDebugString(str);
					frameCounter = 0;
					second = second.zero();
				}

			}
		}
		return 0;
	}

	void Engine::Simulate(float deltaTime)
	{
		PhysicsManager.RunFrame(deltaTime);
	}

	void Engine::Render()
	{
		simd_vector<PhysicsObject> physicsObjects;
		simd_vector<SphereSprite> sprites;

		PhysicsManager.CopyCurrentPhysicsObjects(physicsObjects);
		sprites.reserve(NumSpheres);

		transform(physicsObjects.begin(), physicsObjects.end(), back_inserter(sprites),
			[](const PhysicsObject& object)	{
			return SphereSprite{ object.Position, object.Color, object.CollisionRadius };
		});

		Vector4 cameraPosition(0, 0, -500);

		Matrix4 viewProjection = Matrix4(cameraPosition) * Matrix4::PerspectiveProjectionMatrix(2.5f, Renderer->AspectRatio, 10, 10000);

		sort(sprites.begin(), sprites.end(), [&](const SphereSprite& first, const SphereSprite& second)
		{
			return (viewProjection * first.Position).Z < (viewProjection * second.Position).Z;
		});

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		//sprites
		unsigned int spriteVBO;
		glGenBuffers(1, &spriteVBO);
		glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);

		SpriteTechnique.Enable();
		SpriteTechnique.SetViewProjection(viewProjection);
		//spriteTechnique.SetViewProjection(Matrix4::OrthographicProjectionMatrix(-500, 500, 500, -500, -500, 500));
		SpriteTechnique.SetCameraPosition(cameraPosition);

		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), (void*)offsetof(SphereSprite, Position));
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), (void*)offsetof(SphereSprite, Color));
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), (void*)offsetof(SphereSprite, Radius));

		int glError = glGetError();

		glBufferData(GL_ARRAY_BUFFER, sprites.size() * sizeof(SphereSprite), &sprites.front(), GL_DYNAMIC_DRAW);

		glDrawArrays(GL_POINTS, 0, sprites.size());
		glDeleteBuffers(1, &spriteVBO);

		Renderer->Render();

	}
}