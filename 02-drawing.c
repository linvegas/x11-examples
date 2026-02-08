#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#define PI 3.14159265358979323846

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

    XSelectInput(display, window, ExposureMask | KeyPressMask | StructureNotifyMask);

    XMapWindow(display, window);

    // Handle the "close window" button request by the window manager
    Atom wm_delete_msg = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(display, window, &wm_delete_msg, 1);

    GC gc = XCreateGC(display, window, 0, NULL);

    bool running = true;

    int screen_w = 800;
    int screen_h = 600;

    int rect_x = 10;
    int rect_y = 10;
    int rect_w = 100;
    int rect_h = 100;
    float angle = 0.0;

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

            else if (event.type == ConfigureNotify)
            {
                screen_w = event.xconfigure.width;
                screen_h = event.xconfigure.height;
            }

            XFlush(display);
        }

        XSetWindowBackground(display, window, 0x181818);
        XClearWindow(display, window);

        angle += 0.02;

        rect_x = (screen_w/2 - rect_w/2) - + rect_w*2 * cosf(angle);
        rect_y = (screen_h/2 - rect_h/2) - + rect_w*2 * sinf(angle);

        XSetForeground(display, gc, 0xab4642);
        XFillRectangle(display, window, gc, rect_x, rect_y, rect_w, rect_h);

        int circle_x = screen_w/2 - 200/2;
        int circle_y = screen_h/2 - 200/2;
        XSetForeground(display, gc, 0xa1b56c);
        XFillArc(display, window, gc, circle_x, circle_y, 200, 200, 0, 360*64);

        // Caps to 60 FPS
        usleep(16666);
    }

    return 0;
}
