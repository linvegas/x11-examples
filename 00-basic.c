#include <stdio.h>

#include <X11/Xlib.h>

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

    while (1)
    {
        XEvent event;
        XNextEvent(display, &event);
        if (event.type == KeyPress) {
            // Press <ESC> to exit
            if (event.xkey.keycode == 9) break;
        }
    }

    return 0;
}
