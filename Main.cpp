#include "Application.h"

long long Milliseconds_now();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	float DesiredFPS = 60.0f;

	Application * theApp = new Application();

	if (FAILED(theApp->Initialise(hInstance, nCmdShow)))
	{
		return -1;
	}

	// Main message loop
	MSG msg = { 0 };

	float deltaTime = 0.0f;

	while (WM_QUIT != msg.message)
	{
		// Update start time
		long long startTime = Milliseconds_now();

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			bool handled = false;

			if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
			{
				handled = theApp->HandleKeyboard(msg, deltaTime);
			}
			else if (WM_QUIT == msg.message)
				break;

			handled = theApp->HandleMouse(msg, deltaTime);

			if (!handled)
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			//if game loop running faster than frame rate
			if (deltaTime < (1.0f / DesiredFPS))
			{
				theApp->Update(1.0f / DesiredFPS);
			}
			else
			{
				theApp->Update(deltaTime);
			}

			theApp->Draw();

			//Calculate Delta Time in seconds
			long long endTime = Milliseconds_now();
			deltaTime = (endTime - startTime) / 1000.0f;

			if (deltaTime < (1.0f / DesiredFPS))
			{
				Sleep(((1.0f / DesiredFPS) - deltaTime) * 1000.0f);
			}
		}
	}

	delete theApp;
	theApp = nullptr;

	return (int)msg.wParam;
}

long long Milliseconds_now() {
	static LARGE_INTEGER s_frequency;
	//check to see if the application can read the frequency
	static BOOL s_use_qpc = QueryPerformanceFrequency(&s_frequency);
	if (s_use_qpc) {
		//most accurate method of getting system time
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		return((now.QuadPart * 1000) / s_frequency.QuadPart);
	}
	else
	{
		//same value but only updates 64 times a second
		return GetTickCount();
	}
}