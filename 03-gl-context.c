#include <stdio.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <GL/gl.h>
#include <GL/glx.h>

int main(void)
{
    Display *display = XOpenDisplay(NULL);
    if (!display)
    {
        fprintf(stderr, "ERROR: Could not open display!\n");
        return 1;
    }

    int screen = DefaultScreen(display);

    GLint glx_attrs[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

    XVisualInfo *visual_info = glXChooseVisual(display, screen, glx_attrs);
    if (!visual_info)
    {
        fprintf(stderr, "ERROR: Could not choose a visual info!\n");
        return 1;
    }

    XSetWindowAttributes w_attr;
    w_attr.colormap = XCreateColormap(
        display, RootWindow(display, screen),
        visual_info->visual, AllocNone
    );
    w_attr.event_mask = ExposureMask | KeyPressMask | StructureNotifyMask;

    Window window = XCreateWindow(
        display, RootWindow(display, screen),
        0, 0, 800, 600, 1,
        visual_info->depth, InputOutput, visual_info->visual,
        CWColormap | CWEventMask, &w_attr
    );

    XStoreName(display, window, "I use x11 btw");

    XMapWindow(display, window);

    // Handle the "close window" button request by the window manager
    Atom wm_delete_msg = XInternAtom(display, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(display, window, &wm_delete_msg, 1);

    GLXContext glx_context = glXCreateContext(display, visual_info, NULL, GL_TRUE);
    glXMakeCurrent(display, window, glx_context);

    PFNGLCREATESHADERPROC glCreateShader = (PFNGLCREATESHADERPROC)glXGetProcAddress((const GLubyte*)"glCreateShader");
    PFNGLSHADERSOURCEPROC glShaderSource = (PFNGLSHADERSOURCEPROC)glXGetProcAddress((const GLubyte*)"glShaderSource");
    PFNGLCOMPILESHADERPROC glCompileShader = (PFNGLCOMPILESHADERPROC)glXGetProcAddress((const GLubyte*)"glCompileShader");
    PFNGLCREATEPROGRAMPROC glCreateProgram = (PFNGLCREATEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glCreateProgram");
    PFNGLATTACHSHADERPROC glAttachShader = (PFNGLATTACHSHADERPROC)glXGetProcAddress((const GLubyte*)"glAttachShader");
    PFNGLLINKPROGRAMPROC glLinkProgram = (PFNGLLINKPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glLinkProgram");
    PFNGLUSEPROGRAMPROC glUseProgram = (PFNGLUSEPROGRAMPROC)glXGetProcAddress((const GLubyte*)"glUseProgram");
    PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)glXGetProcAddress((const GLubyte*)"glGenVertexArrays");
    PFNGLBINDVERTEXARRAYPROC glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)glXGetProcAddress((const GLubyte*)"glBindVertexArray");
    PFNGLGENBUFFERSPROC glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte*)"glGenBuffers");
    PFNGLBINDBUFFERPROC glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte*)"glBindBuffer");
    PFNGLBUFFERDATAPROC glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte*)"glBufferData");
    PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)glXGetProcAddress((const GLubyte*)"glEnableVertexAttribArray");
    PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)glXGetProcAddress((const GLubyte*)"glVertexAttribPointer");

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

        glXSwapBuffers(display, window);

        // Caps to 60 FPS
        usleep(16666);
    }

    return 0;
}
