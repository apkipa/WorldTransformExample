#include "public.h"

#define APP_WINDOW_CLASS_NAME L"WorldTransformExampleWnd"
#define APP_WINDOW_NAME L"World Transform Example"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

ATOM App_RegisterClass(HINSTANCE hInstance) {
	WNDCLASSW WndClass;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	//WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.hbrBackground = NULL;
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hInstance = hInstance;
	WndClass.lpfnWndProc = WndProc;
	WndClass.lpszClassName = APP_WINDOW_CLASS_NAME;
	WndClass.lpszMenuName = NULL;
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	return RegisterClassW(&WndClass);
}

HWND App_CreateWindow(HINSTANCE hInstance) {
	return CreateWindowW(
		APP_WINDOW_CLASS_NAME,
		APP_WINDOW_NAME,
		WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL
	);
}

int App_CycleMessage(void) {
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return (int)msg.wParam;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
	DWORD dwOriginalMode;
	HANDLE hStdOut;

	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleMode(hStdOut, &dwOriginalMode);
	SetConsoleMode(hStdOut, dwOriginalMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

	wprintf(L"World Transform Example v0.1.0 console\n\n");

	App_RegisterClass(hInstance);
	App_CreateWindow(hInstance);

	return App_CycleMessage();
}
