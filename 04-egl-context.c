#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <EGL/egl.h>

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

    // EGL

    EGLDisplay egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    eglInitialize(egl_display, NULL, NULL);
    eglBindAPI(EGL_OPENGL_API);

    EGLint attr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,        8,
        EGL_GREEN_SIZE,      8,
        EGL_BLUE_SIZE,       8,
        EGL_DEPTH_SIZE,      24,
        EGL_NONE
    };

    EGLConfig cfg;
    EGLint ncfg;
    eglChooseConfig(egl_display, attr, &cfg, 1, &ncfg);

    EGLContext ctx = eglCreateContext(egl_display, cfg, EGL_NO_CONTEXT, NULL);
    EGLSurface egl_surface = eglCreateWindowSurface(egl_display, cfg, (EGLNativeWindowType)window, NULL);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, ctx);

    printf("Renderer : %s\n", glGetString(GL_RENDERER));
    printf("Version  : %s\n", glGetString(GL_VERSION));

    // OPENGL

    PFNGLCREATESHADERPROC glCreateShader = (PFNGLCREATESHADERPROC)eglGetProcAddress("glCreateShader");
    PFNGLSHADERSOURCEPROC glShaderSource = (PFNGLSHADERSOURCEPROC)eglGetProcAddress("glShaderSource");
    PFNGLCOMPILESHADERPROC glCompileShader = (PFNGLCOMPILESHADERPROC)eglGetProcAddress("glCompileShader");
    PFNGLCREATEPROGRAMPROC glCreateProgram = (PFNGLCREATEPROGRAMPROC)eglGetProcAddress("glCreateProgram");
    PFNGLATTACHSHADERPROC glAttachShader = (PFNGLATTACHSHADERPROC)eglGetProcAddress("glAttachShader");
    PFNGLLINKPROGRAMPROC glLinkProgram = (PFNGLLINKPROGRAMPROC)eglGetProcAddress("glLinkProgram");
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC)eglGetProcAddress("glUseProgram");
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)eglGetProcAddress("glGenVertexArrays");
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)eglGetProcAddress("glBindVertexArray");
    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC)eglGetProcAddress("glGenBuffers");
    PFNGLBINDBUFFERPROC glBindBuffer = (PFNGLBINDBUFFERPROC)eglGetProcAddress("glBindBuffer");
    PFNGLBUFFERDATAPROC glBufferData = (PFNGLBUFFERDATAPROC)eglGetProcAddress("glBufferData");
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)eglGetProcAddress("glEnableVertexAttribArray");
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)eglGetProcAddress("glVertexAttribPointer");

    const char *vert_shader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "layout (location = 1) in vec3 color;\n"
        "out vec3 f_color;\n"
        "void main(){\n"
        "    gl_Position = vec4(pos,1.0);\n"
        "    f_color = color;\n"
        "}";

    const char *frag_shader =
        "#version 330 core\n"
        "in  vec3 f_color;\n"
        "out vec4 out_color;\n"
        "void main(){\n"
        "    out_color = vec4(f_color, 1.0);\n"
        "}";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vert_shader, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &frag_shader, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);

    float vertices[] = {
         0.0,  0.5,  0.0,  0.67, 0.27, 0.25,
        -0.5, -0.5,  0.0,  0.63, 0.70, 0.42,
         0.5, -0.5,  0.0,  0.48, 0.68, 0.76,
    };

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    bool running = true;
    int screen_w = 800;
    int screen_h = 600;

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

        glViewport(0, 0, screen_w, screen_h);

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        eglSwapBuffers(egl_display, egl_surface);

        // Caps to 60 FPS
        usleep(16666);
    }

    return 0;
}
