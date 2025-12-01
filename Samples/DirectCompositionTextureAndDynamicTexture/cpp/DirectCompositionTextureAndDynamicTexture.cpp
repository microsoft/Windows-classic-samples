#include "DirectCompositionTextureAndDynamicTexture.h"

// Window Procedure: Handles messages sent to the main window.
LRESULT CALLBACK
s_MainWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (message)
    {
    case WM_CLOSE:
        PostQuitMessage(0);
        break;

    default:
        result = DefWindowProc(hwnd, message, wParam, lParam);
    }

    return result;
}

// Entry point of a Windows application.
extern "C" int WINAPI wWinMain(HINSTANCE, HINSTANCE, _In_ LPTSTR, int nCmdShow)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HINSTANCE hInst = GetModuleHandle(nullptr);

    // Define and initialize the window class structure.
    WNDCLASSEX wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);                          // Set the size of the structure.
    wcex.style = CS_HREDRAW | CS_VREDRAW;                      // Redraw the window on horizontal or vertical size changes.
    wcex.lpfnWndProc = s_MainWindowProc;                       // Set the window procedure.
    wcex.hInstance = hInst;                                    // Associate with our instance.
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);             // Use the standard arrow cursor.
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);           // Set the background color.
    wcex.lpszClassName = L"MainWindowClass";                   // Set the class name.

    // Register the window class. If registration fails, exit.
    if (!RegisterClassEx(&wcex))
    {
        return false;
    }

    // Define window styles.
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
    DWORD exStyle = 0;

    // Set desired window size using TEXTURE_WIDTH and TEXTURE_HEIGHT constants.
    RECT rect = { 0, 0, TEXTURE_WIDTH, TEXTURE_HEIGHT };
    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    // Create the window with default position and calculated size.
    HWND hwnd = CreateWindowEx(
        exStyle,
        wcex.lpszClassName,                    // Window class name.
        L"Composition Textures Sample Window", // Window title.
        style,                                 // Window style.
        CW_USEDEFAULT,                         // Default horizontal position.
        CW_USEDEFAULT,                         // Default vertical position.
        rect.right - rect.left,                // Width.
        rect.bottom - rect.top,                // Height.
        nullptr,                               // No parent window.
        nullptr,                               // No menu.
        hInst,                                 // Instance handle.
        nullptr);                              // No additional application data.

    // If window creation fails, exit.
    if (!hwnd)
    {
        return 1;
    }

    // Display and update the window.
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    CApp app;
    // Start the application (starts main render thread).
    app.Start(hwnd);

    // Main message loop.
    MSG msg = { 0 };
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Stop the application (cleanup DirectComposition resources, etc.).
    app.Stop();

    return 0;
}
