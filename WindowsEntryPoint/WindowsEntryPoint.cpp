// WindowsEntryPoint.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WindowsEntryPoint.h"
#include "../OpenGLRenderer/Technique.hpp"
#include "../OpenGLRenderer/OpenGLRenderer.hpp"
#include "../Physics/PhysicsManager.hpp"

#include <GL/glew.h>
#include <GL/wglew.h>

using namespace std;
using namespace Core;
using namespace Physics;

class SphereParticleTechnique : public Rendering::Technique
{
public:
	SphereParticleTechnique()
	{
		Setup();
	}

	virtual bool Setup()
	{
		AddShader(GL_VERTEX_SHADER, "SphereParticles.vs");
		AddShader(GL_GEOMETRY_SHADER, "SphereParticles.gs");
		AddShader(GL_FRAGMENT_SHADER, "SphereParticles.fs");

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glEnable(GL_PROGRAM_POINT_SIZE);

		Finalize();

		ViewProjectionIndex = glGetUniformLocation(ProgramIndex, "gVP");
		CameraPositionIndex = glGetUniformLocation(ProgramIndex, "gCameraPos");

		return true;
	}

	void SetViewProjection(const Core::Matrix4& viewProjection)
	{
		glUniformMatrix4fv(ViewProjectionIndex, 1, GL_FALSE, viewProjection.begin()->begin());
	}
	void SetCameraPosition(const Core::Vector4& cameraPosition)
	{
		glUniform4fv(CameraPositionIndex, 1, cameraPosition.begin());
	}

private:

	int ViewProjectionIndex;
	int CameraPositionIndex;
};

struct SphereSprite
{
	Core::Vector4 Position;
	float Radius;
};

class SimpleTechnique : public Rendering::Technique
{
public:
	SimpleTechnique()
	{
		Setup();
	}

	bool Setup()
	{
		AddShader(GL_VERTEX_SHADER, "Simple.vs");
		AddShader(GL_FRAGMENT_SHADER, "Simple.fs");

		Finalize();

		glEnableVertexAttribArray(0);

		ViewProjectionIndex = glGetUniformLocation(ProgramIndex, "ViewProjection");

		return true;
	}

	void SetViewProjection(const Core::Matrix4& viewProjection)
	{
		glUniformMatrix4fv(ViewProjectionIndex, 1, GL_FALSE, viewProjection.begin()->begin());
	}

private:

	int ViewProjectionIndex;
};

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

Rendering::OpenGLRenderer* Renderer;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE /*hPrevInstance*/,
                     _In_ LPTSTR    /*lpCmdLine*/,
                     _In_ int       nCmdShow)
{

	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_WINDOWSENTRYPOINT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSENTRYPOINT));

	SphereParticleTechnique spriteTechnique;
	SimpleTechnique simpleTechnique;
	PhysicsManager physicsManager(4);

	default_random_engine engine;
	uniform_real_distribution<float> positionDist(-500.0f, 500.0f);
	uniform_real_distribution<float> velocityDist(-10.0f, 10.0f);

	const int numSpheres = 200;
	for (int i = 0; i < numSpheres; ++i)
	{
		physicsManager.AddCollisionObject(	Vector4(positionDist(engine), positionDist(engine), positionDist(engine)), 
											Vector4(velocityDist(engine), velocityDist(engine), velocityDist(engine)),
											100);
	}

	unsigned int spriteVBO;
	glGenBuffers(1, &spriteVBO);

	unsigned int triangleVBO;
	glGenBuffers(1, &triangleVBO);

	Vector4 cameraPosition(0, 0, 0);

	Matrix4 triangleModel;

	bool bRunning = true;
	while (bRunning)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				bRunning = false;
			}
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else 
		{ 
			simd_vector<PhysicsState> physicsState;
			simd_vector<CollisionObject> collisionObjects;
			simd_vector<SphereSprite> sprites;

			physicsManager.PhysicsFrame(0.01f);

			physicsManager.CopyCurrentPhysicsState(physicsState);
			physicsManager.CopyCollisionObjects(collisionObjects);

			//transform(physicsState.begin(), physicsState.end(), collisionObjects.begin(), back_inserter(sprites),
			//	[](const PhysicsState& state, const CollisionObject& object)->SphereSprite
			//{
			//	const SphereSprite result = { state.Position, object.Radius };
			//	return result;
			//});

			for (int i = 0; i < numSpheres; ++i)
			{
				SphereSprite result;
				result.Position = physicsState[i].Position;
				result.Radius = collisionObjects[i].Radius;
				sprites.push_back(result);
			}

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//sprites
			glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
			glBufferData(GL_ARRAY_BUFFER, sprites.size() * sizeof(SphereSprite), &sprites.front(), GL_DYNAMIC_DRAW);

			spriteTechnique.Enable();
			spriteTechnique.SetViewProjection(Matrix4::OrthographicProjectionMatrics( -500, 500, 500, -500, -500, 500 ) );
			spriteTechnique.SetCameraPosition(cameraPosition * -1.0f);

			//cameraPosition += Vector4(0, 0, 0.01f);

			glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 5, 0);
			glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 5, (void*)4);

			glDrawArrays(GL_POINTS, 0, sprites.size());

			//triangle
			simd_vector<Vector4> triangleVertices;
			triangleVertices.push_back(Vector4(-100,	0,		-100));
			triangleVertices.push_back(Vector4(0,		100,	-100));
			triangleVertices.push_back(Vector4(100,		0,		-100));

			triangleModel = Matrix4(Vector4(0, 0, 1.0f), 0.1f) * triangleModel;

			simpleTechnique.Enable();
			simpleTechnique.SetViewProjection(triangleModel * Matrix4(cameraPosition * -1.0f) * Matrix4::PerspectiveProjectionMatrix(30.0f, 1.6f, 10.0f, 1000.0f));

			glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
			glBufferData(GL_ARRAY_BUFFER, triangleVertices.size() * sizeof(Vector4), &triangleVertices.front(), GL_DYNAMIC_DRAW);
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
			//glDrawArrays(GL_TRIANGLES, 0, triangleVertices.size());

			Renderer->Render();

			int error = glGetError();
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSENTRYPOINT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_WINDOWSENTRYPOINT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }
	
   Renderer = new Rendering::OpenGLRenderer(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_SIZE:
		Renderer->ResizeViewport(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
