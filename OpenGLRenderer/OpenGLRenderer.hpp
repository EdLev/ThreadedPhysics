#pragma once

#include <GL/glew.h>
#include <GL/wglew.h>

namespace Rendering
{
	//eventually inherit from a parent Renderer class to support other APIs
	class OpenGLRenderer
	{
	public:
		OpenGLRenderer(HWND windowHandle);
		~OpenGLRenderer();

		void ResizeViewport(int width, int height);

		void Render();

	private:
		bool CreateContext();
		void PrepareScene();

		HWND Window;
		HDC DeviceContext;
		HGLRC RenderContext;

		int WindowWidth;
		int WindowHeight;
	};
}