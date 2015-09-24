#include "OpenGLRenderer.hpp"

namespace Rendering
{
	OpenGLRenderer::OpenGLRenderer(HWND windowHandle)
	{
		Window = windowHandle;
		CreateContext();
		PrepareScene();
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
		wglMakeCurrent(DeviceContext, 0);
		wglDeleteContext(RenderContext);
		ReleaseDC(Window, DeviceContext);
	}

	bool OpenGLRenderer::CreateContext()
	{
		DeviceContext = GetDC(Window);

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR)); 
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 32;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat = ChoosePixelFormat(DeviceContext, &pfd);
		if (pixelFormat == 0)
			return false;

		bool bResult = SetPixelFormat(DeviceContext, pixelFormat, &pfd) != 0; 
		if (!bResult) 
			return false;

		HGLRC tempOpenGLContext = wglCreateContext(DeviceContext);
		wglMakeCurrent(DeviceContext, tempOpenGLContext);

		GLenum error = glewInit();  
		if (error != GLEW_OK) 
			return false;

		int attributes[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4, 
			WGL_CONTEXT_MINOR_VERSION_ARB, 0, 
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB, 
			0
		};

		if (wglewIsSupported("WGL_ARB_create_context") == 1) { // If the OpenGL 3.x context creation extension is available  
			RenderContext = wglCreateContextAttribsARB(DeviceContext, NULL, attributes); // Create and OpenGL 3.x context based on the given attributes  
			wglMakeCurrent(NULL, NULL); // Remove the temporary context from being active  
			wglDeleteContext(tempOpenGLContext); // Delete the temporary OpenGL 2.1 context  
			wglMakeCurrent(DeviceContext, RenderContext); // Make our OpenGL 3.0 context current  
		}
		else {
			RenderContext = tempOpenGLContext; // If we didn't have support for OpenGL 3.x and up, use the OpenGL 2.1 context  
		}

		return true;
	}

	void OpenGLRenderer::PrepareScene()
	{
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	}

	void OpenGLRenderer::ResizeViewport(int width, int height)
	{
		WindowWidth = width;
		WindowHeight = height;
		AspectRatio = static_cast<float>(width) / height;

		glViewport(0, 0, WindowWidth, WindowHeight);
	}

	void OpenGLRenderer::Render()
	{
		//glViewport(0, 0, WindowWidth, WindowHeight);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		SwapBuffers(DeviceContext);
	}
}