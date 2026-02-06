#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "ERROR: Could not open display!\n");
        return 1;
    }

    int screen = DefaultScreen(display);

    Window window = XCreateSimpleWindow(
        display, RootWindow(display, screen),
        0, 0, 800, 600, 1,
        BlackPixel(display, screen),
        BlackPixel(display, screen)
    );

    XStoreName(display, window, "I use x11 btw");

    XSelectInput(display, window, ExposureMask | KeyPressMask);

    XMapWindow(display, window);

    // Handle the "close window" button request by the window manager
    Atom wm_delete_msg = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(display, window, &wm_delete_msg, 1);

    bool running = true;

    while (running)
    {
        XPending(display);

        while (QLength(display)) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                if (key == XK_Escape) running = false;
            }

            else if (event.type == ClientMessage)
            {
                if (event.xclient.data.l[0] == (long int)wm_delete_msg)
                    running = false;
            }

            XFlush(display);
        }

        // Drawing should go here
        // ...

        // Caps to 60 FPS
        usleep(16666);
    }

    return 0;
}
