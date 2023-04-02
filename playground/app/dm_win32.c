
typedef struct DmWin32 DmWin32;
struct DmWin32 {
	HINSTANCE hinstance;
	HWND hwnd;
};

DmWin32 dm;

void dm_init(void) {
}

void dm_screen_dims(int* width_out, int* height_out) {
	*width_out = GetSystemMetrics(SM_CXSCREEN);
	*height_out = GetSystemMetrics(SM_CYSCREEN);
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	return DefWindowProcA(hwnd, Msg, wParam, lParam);
}

DmWindow dm_window_open(int width, int height) {
	HINSTANCE hinstance = GetModuleHandle(NULL);

	// Register the window class.
	const char CLASS_NAME[] = "Sample Window Class";

	WNDCLASSEXA wc = { 0 };
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hinstance;
	wc.hCursor = LoadCursorA(NULL, IDC_ARROW); // load cursor otherwise it defaults to busy cursor icon
	wc.lpszClassName = CLASS_NAME;

	ATOM class_atom = RegisterClassExA(&wc);
	APP_ASSERT(class_atom != 0, "failed to register class: %lx", GetLastError());

	// Create the window.

	HWND hwnd = CreateWindowExA(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		"Hcc Playground",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window
		NULL,       // Menu
		hinstance,  // Instance handle
		NULL        // Additional application data
	);

	APP_ASSERT(hwnd, "failed to open windows: %lx", GetLastError());


	int screen_width = GetSystemMetrics(SM_CXSCREEN);
	int screen_height = GetSystemMetrics(SM_CYSCREEN);
	int center_x = screen_width / 2;
	int center_y = screen_height / 2;
	int half_width = width / 2;
	int half_height = height / 2;

	SetWindowPos(hwnd, HWND_TOP, center_x - half_width, center_y - half_height, center_x + half_width, center_y + half_height, 0);

	ShowWindow(hwnd, true);

	dm.hinstance = hinstance;
	dm.hwnd = hwnd;

	DmWindow window;
	window.instance = hinstance;
	window.handle = (void*)(uintptr_t)hwnd;
	return window;
}

bool dm_process_events(DmEvent* e) {
	MSG msg;
	if (!PeekMessage(&msg, dm.hwnd, 0, 0, PM_REMOVE)) {
		return false;
	}

	TranslateMessage(&msg);
	DispatchMessage(&msg);

	if (msg.message == WM_CHAR) {
		e->type = DM_EVENT_TYPE_KEY_PRESSED;
		e->key = msg.wParam;
	} else if (msg.message == WM_CLOSE) {
		e->type = DM_EVENT_TYPE_WINDOW_CLOSED;
	} else if (msg.message == WM_SIZE) {
		e->type = DM_EVENT_TYPE_WINDOW_RESIZE;
		e->window_width = LOWORD(msg.lParam);
		e->window_height = HIWORD(msg.lParam);
	} else {
		e->type = DM_EVENT_TYPE_UNKNOWN;
	}

	return true;
}

