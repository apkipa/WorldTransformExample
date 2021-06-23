#include "public.h"

struct {
	enum {
		Data_TransformMatrix, Data_Dummy
	} type;
	wchar_t name[32];
	union {
		struct Data_TransformMatrix {
			float eM11, eM12, eM21, eM22, eDx, eDy;
		} tm;
		struct Data_Dummy {
			int i32;
		} dummy;
	};
} DataStack[1024];
int nDataInStack = 0;

XFORM xFormCur = { 1, 0, 0, 1, 0, 0 };

int DataStack_GetExistingDataIndexByName(const wchar_t *name) {
	for (int i = 0; i < nDataInStack; i++) {
		if (wcscmp(name, DataStack[i].name) == 0) {
			return i;
		}
	}
	return -1;	// Not found
}

int DataStack_GetDataIndexByName(const wchar_t *name) {
	for (int i = 0; i < nDataInStack; i++) {
		if (wcscmp(name, DataStack[i].name) == 0) {
			return i;
		}
	}
	wcscpy(DataStack[nDataInStack].name, name);
	return nDataInStack++;
}

void DataStack_SetDataAsTransformMatrix(
	const wchar_t *name, float eM11, float eM12, float eM21, float eM22, float eDx, float eDy
) {
	struct Data_TransformMatrix data = { eM11, eM12, eM21, eM22, eDx, eDy };
	int nIndex;

	nIndex = DataStack_GetDataIndexByName(name);

	DataStack[nIndex].type = Data_TransformMatrix;
	DataStack[nIndex].tm = data;
}

void DataStack_SetDataAsDummy(const wchar_t *name) {
	int nIndex;

	nIndex = DataStack_GetDataIndexByName(name);

	DataStack[nIndex].type = Data_Dummy;
	DataStack[nIndex].dummy = (struct Data_Dummy) { 42 };
}

bool DataStack_GetDataAsTransformMatrix(const wchar_t *name, struct Data_TransformMatrix *pData) {
	// Return: true -> success, false -> failure
	int nIndex = DataStack_GetExistingDataIndexByName(name);
	if (nIndex < 0) {
		return false;
	}
	if (DataStack[nIndex].type != Data_TransformMatrix) {
		return false;
	}
	*pData = DataStack[nIndex].tm;
	return true;
}

void ConsoleThreadSubProc_Debug(wchar_t **pArgs, int nArgCnt) {
	wprintf(
		L"Debug information:\n"
		"    %d / %d in data stack\n",
		nDataInStack, (int)_countof(DataStack)
	);
}

void ConsoleThreadSubProc_Set(wchar_t **pArgs, int nArgCnt) {
	if (nArgCnt < 1) {
		wprintf(
			L"Usage:\n"
			"    set tm <name> <eM11> <eM12> <eM21> <eM22> <eDx> <eDy>\n"
			"    set dummy <name> [dummy ...]\n"
		);
		return;
	}
	if (wcscmp(pArgs[0], L"tm") == 0) {
		float fArgs[6];
		if (nArgCnt != 7 + 1) {
			PrintErrorW(L"`set tm` excepted 7 arguments, got %d", nArgCnt - 1);
			return;
		}
		for (int i = 0; i < _countof(fArgs); i++) {
			if (!wcstof_strict(pArgs[i + 2], &fArgs[i])) {
				PrintErrorW(L"Cannot parse `%ls`: not a valid number", pArgs[i + 2]);
				return;
			}
		}
		DataStack_SetDataAsTransformMatrix(
			pArgs[1],
			fArgs[0], fArgs[1], fArgs[2], fArgs[3], fArgs[4], fArgs[5]
		);
	}
	else if (wcscmp(pArgs[0], L"dummy") == 0) {
		if (nArgCnt < 1 + 1) {
			PrintErrorW(L"`set dummy` excepted >= 1 arguments, got %d", nArgCnt - 1);
			return;
		}
		wprintf(L"Dummy set!\n");
		DataStack_SetDataAsDummy(pArgs[1]);
	}
	else {
		PrintErrorW(L"Unknown set target `%ls`", pArgs[0]);
	}
}

void ConsoleThreadSubProc_Animate(HWND hwnd, wchar_t **pArgs, int nArgCnt) {
	XFORM xOriginalForm, xFinalForm;
	int nIndex;

	if (nArgCnt < 1) {
		wprintf(
			L"Usage:\n"
			"    animate <name>\n"
		);
		return;
	}
	if (nArgCnt >= 2) {
		PrintErrorW(L"`animate` excepted 1 arguments, got %d", nArgCnt);
		return;
	}

	nIndex = DataStack_GetExistingDataIndexByName(pArgs[0]);
	if (nIndex < 0) {
		PrintErrorW(L"Data `%ls` not found", pArgs[0]);
		return;
	}

	if (DataStack[nIndex].type == Data_TransformMatrix) {
		const int TOTAL_TIME = (int)(1.5 * 1000);
		xOriginalForm = xFormCur;
		xFinalForm = (XFORM) { 
			DataStack[nIndex].tm.eM11, DataStack[nIndex].tm.eM12,
			DataStack[nIndex].tm.eM21, DataStack[nIndex].tm.eM22,
			DataStack[nIndex].tm.eDx, DataStack[nIndex].tm.eDy
		};
		for (
			int nBegin = GetTickCount(), nEnd = GetTickCount();
			nEnd - nBegin <= TOTAL_TIME;
			Sleep(10), nEnd = GetTickCount()
		) {
			// Prepare data
			double fOriginalT = (nEnd - nBegin) / (double)TOTAL_TIME;
			double t = (sin(fOriginalT * M_PI - M_PI / 2) + 1) / 2;
			/*
			wprintf(
				L"%.1f, %.1f; %.1f, %.1f; t = %f\n",
				xOriginalForm.eDx + (xFinalForm.eDx - xOriginalForm.eDx) * t,
				xOriginalForm.eDy + (xFinalForm.eDy - xOriginalForm.eDy) * t,
				(xFinalForm.eDx - xOriginalForm.eDx) * t,
				(xFinalForm.eDy - xOriginalForm.eDy) * t,
				t
			);
			*/
			// Update xform
			xFormCur = (XFORM) {
				xOriginalForm.eM11 + (xFinalForm.eM11 - xOriginalForm.eM11) * t,
				xOriginalForm.eM12 + (xFinalForm.eM12 - xOriginalForm.eM12) * t,
				xOriginalForm.eM21 + (xFinalForm.eM21 - xOriginalForm.eM21) * t,
				xOriginalForm.eM22 + (xFinalForm.eM22 - xOriginalForm.eM22) * t,
				xOriginalForm.eDx + (xFinalForm.eDx - xOriginalForm.eDx) * t,
				xOriginalForm.eDy + (xFinalForm.eDy - xOriginalForm.eDy) * t
			};
			// Request redraw
			InvalidateRect(hwnd, NULL, false);
		}
	}
	else if (DataStack[nIndex].type == Data_Dummy) {
		PrintErrorW(L"Cannot animate with dummy data");
	}
	else {
		PrintErrorW(L"Unexpected data type %d was found", DataStack[nIndex].type);
	}

	//wprintf(L"Will animate using `%ls`.\n", pArgs[0]);
}

DWORD WINAPI ConsoleThreadProc(void *pParam) {
	wchar_t strInput[1024], *pStrInput;
	wchar_t **pArgv;
	int nArgvCnt;
	HWND hwnd;

	hwnd = (HWND)pParam;

	wprintf(L"Initialization completed.\n\n");

	while (true) {
		wprintf(L"AppConsole $ ");
		if (fgetws(strInput, _countof(strInput), stdin) != NULL) {
			wchar_t *str;
			str = wcsrchr(strInput, L'\n');
			if (str != NULL) {
				*str = L'\0';
			}
		}
		pStrInput = strInput;
		while (iswspace(*pStrInput)) {
			pStrInput++;
		}
		if (*pStrInput == L'\0') {
			continue;
		}
		//wscanf(L" %[^\n]", strInput);
		//wprintf(L"You have typed `%ls`.\n", strInput);
		pArgv = CommandLineToArgvW(pStrInput, &nArgvCnt);
		if (wcscmp(pArgv[0], L"exit") == 0 || wcscmp(pArgv[0], L"quit") == 0) {
			PostMessageW(hwnd, WM_CLOSE, 0 ,0);
		}
		else if (wcscmp(pArgv[0], L"help") == 0) {
			wprintf(
				L"Help:\n"
				"    Available commands:\n"
				"        exit, quit, help, debug, set, animate\n"
			);
		}
		else if (wcscmp(pArgv[0], L"debug") == 0) {
			ConsoleThreadSubProc_Debug(&pArgv[1], nArgvCnt - 1);
		}
		else if (wcscmp(pArgv[0], L"set") == 0) {
			ConsoleThreadSubProc_Set(&pArgv[1], nArgvCnt - 1);
		}
		else if (wcscmp(pArgv[0], L"animate") == 0) {
			ConsoleThreadSubProc_Animate(hwnd, &pArgv[1], nArgvCnt - 1);
		}
		else {
			PrintErrorW(L"Unrecognized command `%ls`", pArgv[0]);
		}
		LocalFree(pArgv);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
		HANDLE hConsoleThread;
		hConsoleThread = CreateThread(NULL, 0, ConsoleThreadProc, (void*)hwnd, 0, NULL);
		CloseHandle(hConsoleThread);
        break;
	}
	case WM_PAINT: {
		const int INF = 1000000;
		const int GAP = 60;
		POINT ptCenter;
		RECT rtClient;
		HPEN hGrayPen;
		//RECT rtInf;
		HDC hdc;

		GetClientRect(hwnd, &rtClient);
		DwmFlush();
		hdc = GetDC(hwnd);
		SetGraphicsMode(hdc, GM_ADVANCED);
		hGrayPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
		ptCenter.x = rtClient.right / 2;
		ptCenter.y = rtClient.bottom / 2;
		//rtInf = (RECT) { -INF, -INF, INF, INF };
		//GdiSetBatchLimit(100);

		// Erase background
		//FillRect(hdc, &rtInf, GetStockObject(WHITE_BRUSH));
		FillRect(hdc, &rtClient, GetStockObject(WHITE_BRUSH));

		/*
		// Draw coordinates
		SelectObject(hdc, hGrayPen);
		for (int i = -100; i < 0; i++) {
			MoveToEx(hdc, i * GAP + ptCenter.x, -INF, NULL);
			LineTo(hdc, i * GAP + ptCenter.x, INF);
			MoveToEx(hdc, -INF, i * GAP + ptCenter.y, NULL);
			LineTo(hdc, INF, i * GAP + ptCenter.y);
		}
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		{
			MoveToEx(hdc, ptCenter.x, -INF, NULL);
			LineTo(hdc, ptCenter.x, INF);
			MoveToEx(hdc, -INF, ptCenter.y, NULL);
			LineTo(hdc, INF, ptCenter.y);
		}
		SelectObject(hdc, hGrayPen);
		for (int i = 1; i <= 100; i++) {
			MoveToEx(hdc, i * GAP + ptCenter.x, -INF, NULL);
			LineTo(hdc, i * GAP + ptCenter.x, INF);
			MoveToEx(hdc, -INF, i * GAP + ptCenter.y, NULL);
			LineTo(hdc, INF, i * GAP + ptCenter.y);
		}

		// Draw shapes
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		Rectangle(
			hdc,
			ptCenter.x - 80, ptCenter.y - 80,
			ptCenter.x + 80, ptCenter.y + 80
		);
		MoveToEx(hdc, ptCenter.x - 50, ptCenter.y + 30, NULL);
		LineTo(hdc, ptCenter.x - 20, ptCenter.y + 60);
		LineTo(hdc, ptCenter.x + 60, ptCenter.y - 60);
		*/

		SetWorldTransform(hdc, &(XFORM) { 1, 0, 0, -1, ptCenter.x, ptCenter.y });
		ModifyWorldTransform(hdc, &xFormCur, MWT_LEFTMULTIPLY);
		// Draw coordinates
		SelectObject(hdc, hGrayPen);
		for (int i = -100; i < 0; i++) {
			MoveToEx(hdc, i * GAP, -INF, NULL);
			LineTo(hdc, i * GAP, INF);
			MoveToEx(hdc, -INF, i * GAP, NULL);
			LineTo(hdc, INF, i * GAP);
		}
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		{
			MoveToEx(hdc, 0, -INF, NULL);
			LineTo(hdc, 0, INF);
			MoveToEx(hdc, -INF, 0, NULL);
			LineTo(hdc, INF, 0);
		}
		SelectObject(hdc, hGrayPen);
		for (int i = 1; i <= 100; i++) {
			MoveToEx(hdc, i * GAP, -INF, NULL);
			LineTo(hdc, i * GAP, INF);
			MoveToEx(hdc, -INF, i * GAP, NULL);
			LineTo(hdc, INF, i * GAP);
		}

		// Draw shapes
		SelectObject(hdc, GetStockObject(BLACK_PEN));
		SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
		Rectangle(
			hdc,
			-80, 80,
			80, -80
		);
		MoveToEx(hdc, -50, -30, NULL);
		LineTo(hdc, -20, -60);
		LineTo(hdc, 60, 60);

		//GdiFlush();
		DeleteObject(hGrayPen);
		ReleaseDC(hwnd, hdc);
		ValidateRect(hwnd, &rtClient);

		break;
	}
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

/*
#include <Windows.h>

//Constants
//	Control IDs
//		Timer
//			Id
#define TIMER_ID_Test 101
//			Delay
#define TIMER_DELAY_Test 500
//		Control
//			Button
//				Id
#define CONTROL_ID_BUTTON_Test 101
//	For WndProc_Close()
#define WNDPROC_CLOSE_RETURN_APPLY_CLOSE TRUE
#define WNDPROC_CLOSE_RETURN_DENY_CLOSE FALSE

#pragma region WndProc
int WndProc_Close(HWND hwnd) {
	//Todo: paste your code here

	return WNDPROC_CLOSE_RETURN_APPLY_CLOSE;
}

#pragma region WndProc_Command
void WndProc_Command_Button_Test(HWND hwnd, int nControlNotificationCode, HWND hwControl) {
	//Todo: paste your code here
}

void WndProc_Command(HWND hwnd, int nControlId, int nControlNotificationCode, HWND hwControl) {
	switch (nControlId) {
	case CONTROL_ID_BUTTON_Test:
		WndProc_Command_Button_Test(hwnd, nControlNotificationCode, hwControl);
		break;
		//Todo: paste your code here
	default:
		break;
	}
}
#pragma endregion

void WndProc_Create(HWND hwnd, LPCREATESTRUCT lpCreateStruct) {
	//Todo: paste your code here
}

void WndProc_Destroy(HWND hwnd) {
	//Todo: paste your code here

	PostQuitMessage(EXIT_SUCCESS);
}

void WndProc_LeftButtonDoubleClick(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_LeftButtonDown(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_LeftButtonUp(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_MiddleButtonDoubleClick(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_MiddleButtonDown(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_MiddleButtonUp(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_RightButtonDoubleClick(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_RightButtonDown(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_RightButtonUp(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_MouseMove(HWND hwnd, DWORD dwMouseKeyState, int xMouse, int yMouse) {
	//Todo: paste your code here
}

void WndProc_KeyDown(
	HWND hwnd,
	BYTE byteVirtualKeyCode,
	BYTE byteRepeatCount,
	BYTE byteScanCode,
	BOOL bIsExtended,
	BOOL bContextCode,
	BOOL bPreviousKeyState,
	BOOL bTransitionState
) {
	//Todo: paste your code here
}

void WndProc_KeyUp(
	HWND hwnd,
	BYTE byteVirtualKeyCode,
	BYTE byteRepeatCount,
	BYTE byteScanCode,
	BOOL bIsExtended,
	BOOL bContextCode,
	BOOL bPreviousKeyState,
	BOOL bTransitionState
) {
	//Todo: paste your code here
}

void WndProc_Paint(HWND hwnd) {
	PAINTSTRUCT PaintStruct;

	BeginPaint(hwnd, &PaintStruct);

	//Todo: paste your code here

	EndPaint(hwnd, &PaintStruct);
}

void WndProc_Size(HWND hwnd, int nResizingRequested, int nClientWidth, int nClientHeight) {
	//Todo: paste your code here
}

#pragma region WndProc_Timer
void WndProc_Timer_Test(HWND hwnd) {
	//Todo: paste your code here
}

void WndProc_Timer(HWND hwnd, UINT_PTR nTimerId) {
	switch (nTimerId) {
	case TIMER_ID_Test:
		WndProc_Timer_Test(hwnd);
		break;
		//Todo: paste your code here
	default:
		break;
	}
}
#pragma endregion

void WndProc_Notify(HWND hwnd, int nIdCommon, NMHDR *pNmhdr) {
	//Todo: paste your code here
}

void WndProc_SetFocus(HWND hwnd, HWND hwndLostFocus) {
	//Todo: paste your code here
}

void WndProc_KillFocus(HWND hwnd, HWND hwndGotFocus) {
	//Todo: paste your code here
}

int WndProc_CopyData(HWND hwnd, HWND hwSource, PCOPYDATASTRUCT pCopyDataStruct) {
	//Todo: paste your code here
	return 0;	//This message is not dealt with; use 1 to indicate it is processed
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_CLOSE:
		switch (WndProc_Close(hwnd)) {
		case WNDPROC_CLOSE_RETURN_APPLY_CLOSE:	break;
		case WNDPROC_CLOSE_RETURN_DENY_CLOSE:	return FALSE;
		default:								break;
		}
		break;
	case WM_COMMAND:
		WndProc_Command(hwnd, LOWORD(wParam), HIWORD(wParam), (HWND)lParam);
		break;
	case WM_CREATE:
		WndProc_Create(hwnd, (LPCREATESTRUCT)lParam);
		break;
	case WM_DESTROY:
		WndProc_Destroy(hwnd);
		break;
	case WM_LBUTTONDBLCLK:
		WndProc_LeftButtonDoubleClick(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_LBUTTONDOWN:
		WndProc_LeftButtonDown(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_LBUTTONUP:
		WndProc_LeftButtonUp(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MBUTTONDBLCLK:
		WndProc_MiddleButtonDoubleClick(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MBUTTONDOWN:
		WndProc_MiddleButtonDown(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MBUTTONUP:
		WndProc_MiddleButtonUp(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_RBUTTONDBLCLK:
		WndProc_RightButtonDoubleClick(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_RBUTTONDOWN:
		WndProc_RightButtonDown(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_RBUTTONUP:
		WndProc_RightButtonUp(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_MOUSEMOVE:
		WndProc_MouseMove(hwnd, (DWORD)wParam, MAKEPOINTS(lParam).x, MAKEPOINTS(lParam).y);
		break;
	case WM_KEYDOWN:
		WndProc_KeyDown(
			hwnd,
			(BYTE)wParam,
			(BYTE)(lParam & 65535),
			(BYTE)((lParam >> 16) & 255),
			(BOOL)((lParam >> 24) & 1),
			(BOOL)((lParam >> 29) & 1),
			(BOOL)((lParam >> 30) & 1),
			(BOOL)((lParam >> 31) & 1)
		);
		break;
	case WM_KEYUP:
		WndProc_KeyUp(
			hwnd,
			(BYTE)wParam,
			(BYTE)(lParam & 65535),
			(BYTE)((lParam >> 16) & 255),
			(BOOL)((lParam >> 24) & 1),
			(BOOL)((lParam >> 29) & 1),
			(BOOL)((lParam >> 30) & 1),
			(BOOL)((lParam >> 31) & 1)
		);
		break;
	case WM_PAINT:
		WndProc_Paint(hwnd);
		break;
	case WM_SIZE:
		WndProc_Size(hwnd, (int)wParam, LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_TIMER:
		WndProc_Timer(hwnd, (UINT_PTR)wParam);
		break;
	case WM_NOTIFY:
		WndProc_Notify(hwnd, (int)wParam, (NMHDR*)lParam);
		break;
	case WM_SETFOCUS:
		WndProc_SetFocus(hwnd, (HWND)wParam);
		break;
	case WM_KILLFOCUS:
		WndProc_KillFocus(hwnd, (HWND)wParam);
		break;
	case WM_COPYDATA:
		return WndProc_CopyData(hwnd, (HWND)wParam, (PCOPYDATASTRUCT)lParam);
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
#pragma endregion
*/
