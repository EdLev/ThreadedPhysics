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

		glEnable(GL_PROGRAM_POINT_SIZE);

		Finalize();

		ViewProjectionIndex = glGetUniformLocation(ProgramIndex, "gVP");
		CameraPositionIndex = glGetUniformLocation(ProgramIndex, "gCameraPos");

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		return true;
	}

	virtual void Enable()
	{

		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);

		Technique::Enable();
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
	Core::Vector4 Color;
	float Radius;
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
	PhysicsManager physicsManager(1);

	default_random_engine engine( 0/*chrono::high_resolution_clock::now().time_since_epoch().count() */);
	uniform_real_distribution<float> positionDist(-500.0f, 500.0f);
	uniform_real_distribution<float> velocityDist(-10.0f, 10.0f);

	const int numSpheres = 100;
	for (int i = 0; i < numSpheres; ++i)
	{
		physicsManager.AddCollisionObject(	Vector4(positionDist(engine), positionDist(engine), positionDist(engine)),
											Vector4(velocityDist(engine), velocityDist(engine), velocityDist(engine)),
											50.0f);
	}

	unsigned int spriteVBO;
	glGenBuffers(1, &spriteVBO);

	unsigned int triangleVBO;
	glGenBuffers(1, &triangleVBO);

	Vector4 cameraPosition(0, 0, 0);

	Matrix4 triangleModel;

	chrono::time_point<chrono::high_resolution_clock> lastTime = chrono::high_resolution_clock::now();
	chrono::duration<float> second( 0 );
	unsigned int frameCounter = 0;

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

			chrono::duration<float> interval(chrono::high_resolution_clock::now() - lastTime);
			lastTime = chrono::high_resolution_clock::now();

			physicsManager.RunFrame( interval.count() );

			second += interval;

			char str[16];

			if (second.count() >= 1.0f)
			{
				sprintf_s(str, "%d\n", frameCounter);
				OutputDebugString(str);
				frameCounter = 0;
				second = second.zero();
			}

			physicsManager.CopyCurrentPhysicsState(physicsState);
			physicsManager.CopyCollisionObjects(collisionObjects);

			transform(physicsState.begin(), physicsState.end(), collisionObjects.begin(), back_inserter(sprites),
				[](const PhysicsState& state, const CollisionObject& object)->SphereSprite
			{
				const SphereSprite result = { state.Position, object.Color, object.Radius };
				return result;
			});

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

			//sprites
			glBindBuffer(GL_ARRAY_BUFFER, spriteVBO);

			//cameraPosition.X = 1000;

			spriteTechnique.Enable();
			spriteTechnique.SetViewProjection(Matrix4(cameraPosition) * Matrix4::PerspectiveProjectionMatrix(45, 1.6f, 10, 1000));
			//spriteTechnique.SetViewProjection(Matrix4::OrthographicProjectionMatrix(-500, 500, 500, -500, -500, 500));
			spriteTechnique.SetCameraPosition(cameraPosition);

			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), 0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), (void*)(sizeof(Vector4)) );
			glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(SphereSprite), (void*)(sizeof(Vector4) * 2));

			glBufferData(GL_ARRAY_BUFFER, sprites.size() * sizeof(SphereSprite), &sprites.front(), GL_DYNAMIC_DRAW);

			glDrawArrays(GL_POINTS, 0, sprites.size());

			Renderer->Render();

			++frameCounter;
		}
	}

	delete Renderer;
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
