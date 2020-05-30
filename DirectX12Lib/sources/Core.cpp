#include "Core.h"
#include "Graphics.h"

namespace Core {

    void InitializeApplication(IApp& app)
    {
        app.Startup();
    }

    bool UpdateApplication(IApp& app)
    {
        return !app.IsDone();
    }

    void TerminateApplication(IApp& app)
    {
        app.Cleanup();

        //GameInput::Shutdown();
    }

    HWND g_hWnd = nullptr;

    LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	void RunApplication(IApp& app, const wchar_t* className)
	{
		Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_TYPE::RO_INIT_MULTITHREADED);
		ASSERT_SUCCEEDED(InitializeWinRT);

		HINSTANCE hInst = GetModuleHandle(0);

        // Register class
        WNDCLASSEX wcex;
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hInst;
        wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = className;
        wcex.hIconSm = LoadIcon(hInst, IDI_APPLICATION);
        ASSERT(0 != RegisterClassEx(&wcex), "Unable to register a window");

        // Create window
        RECT rc = { 0, 0, (LONG)Graphics::g_DisplayWidth, (LONG)Graphics::g_DisplayHeight };
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
        
        g_hWnd = CreateWindowW(
            className,
            className,
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            rc.right - rc.left,
            rc.bottom - rc.top,
            nullptr,
            nullptr,
            hInst,
            nullptr
        );

        ASSERT(g_hWnd != 0);

        InitializeApplication(app);

        ShowWindow(g_hWnd, SW_SHOWDEFAULT);

        do
        {
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT)
                break;
        } while (UpdateApplication(app));    // Returns false to quit loop

        Graphics::Terminate();
        TerminateApplication(app);
        Graphics::Shutdown();
	}

	bool IApp::IsDone(void)
	{
		return false;
	}

    //--------------------------------------------------------------------------------------
    // Called every time the application receives a message
    //--------------------------------------------------------------------------------------
    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {
        case WM_SIZE:
            //Graphics::Resize((UINT)(UINT64)lParam & 0xFFFF, (UINT)(UINT64)lParam >> 16);
            break;

        case WM_DESTROY:
        case WM_QUIT:
        case WM_CLOSE:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }
}