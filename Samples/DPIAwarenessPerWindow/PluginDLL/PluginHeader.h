#pragma once

#ifdef PLUGINDLL_EXPORTS
#define PLUGINDLL_API __declspec(dllexport)
#else
#define PLUGINDLL_API __declspec(dllimport)
#endif // !PLUGINDLL_EXPORTS

#define HWND_NAME_EXTERNAL         L"External Content"
#define PLUGINWINDOWCLASSNAME      L"Plugin Window Class"

namespace PlugInDll
{
	class PlugInDll
	{
	public:
		static PLUGINDLL_API HWND CreateContentHwnd(HINSTANCE hInstance, int nWidth, int nHeight);

	private:
		static PLUGINDLL_API void ClassRegistration(HINSTANCE hInstance);
		static PLUGINDLL_API LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static PLUGINDLL_API int ScaleToSystemDPI(int in, int mainMonitorDPI);
		static PLUGINDLL_API BOOL GetParentRelativeWindowRect(HWND hWnd, RECT* childBounds);
		static PLUGINDLL_API LRESULT CALLBACK SubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
	};
}
