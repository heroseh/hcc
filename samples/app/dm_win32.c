
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

DmWindow dm_window_open(int width, int height) {
	HINSTANCE hinstance = GetModuleHandle(NULL);

	// Register the window class.
	const char CLASS_NAME[] = "Sample Window Class";

	WNDCLASS wc = {0};
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hinstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	// Create the window.

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		hinstance,  // Instance handle
		NULL        // Additional application data
	);

	APP_ASSERT(hwnd, "failed to open windows");


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
	if (!GetMessage(&msg, dm.hwnd, 0, 0)) {
		return false;
	}

	TranslateMessage(&msg);
	
	if (msg.message == WM_CHAR) {
		e->type = DM_EVENT_TYPE_KEY_PRESSED;
		e->key = msg.wParam;
	} else if (msg.message == WM_CLOSE) {
		e->type = DM_EVENT_TYPE_WINDOW_CLOSED;
	}

	return true;
}

