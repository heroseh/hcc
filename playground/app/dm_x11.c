
#include <X11/Xlib.h>

typedef struct DmX11 DmX11;
struct DmX11 {
	Display* display;
	Atom delete_message_atom;
};

DmX11 dm;

void dm_init(void) {
	dm.display = XOpenDisplay(NULL);
	APP_ASSERT(dm.display, "could not open X11 display connection");
	dm.delete_message_atom = XInternAtom(dm.display, "WM_DELETE_WINDOW", False);
}

void dm_screen_dims(int* width_out, int* height_out) {
	Screen* screen = XDefaultScreenOfDisplay(dm.display);
	*width_out = screen->width;
	*height_out = screen->height;
}

DmWindow dm_window_open(int width, int height) {
	DmWindow window;

	Display* d = dm.display;
	int s = DefaultScreen(d);

	Window w = XCreateSimpleWindow(d, RootWindow(d, s), width / 2, height / 2, width, height, 1, BlackPixel(d, s), WhitePixel(d, s));

	window.instance = d;
	window.handle = (void*)(uintptr_t)w;
	XSelectInput(d, w, ExposureMask | KeyPressMask);
	XMapWindow(d, w);
	XSetWMProtocols(dm.display, w, &dm.delete_message_atom, 1);

	return window;
}

bool dm_process_events(DmEvent* e) {
	XEvent xe;
	while (XPending(dm.display)) {
		XNextEvent(dm.display, &xe);
		if (xe.type == KeyPress) {
			e->type = DM_EVENT_TYPE_KEY_PRESSED;
			KeySym key_sym = XLookupKeysym(&xe.xkey, 0);
			if (key_sym < 128) {
				e->key = (char)key_sym;
				return true;
			}
		} else if (xe.type == KeyRelease) {
			e->type = DM_EVENT_TYPE_KEY_RELEASED;
			KeySym key_sym = XLookupKeysym(&xe.xkey, 0);
			if (key_sym < 128) {
				e->key = (char)key_sym;
				return true;
			}
		} else if (xe.type == ClientMessage) {
			if ((Atom)xe.xclient.data.l[0] == dm.delete_message_atom) {
				e->type = DM_EVENT_TYPE_WINDOW_CLOSED;
				return true;
			}
		}
	}

	return false;
}

