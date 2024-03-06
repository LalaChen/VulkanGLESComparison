#include <thread>

#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <EGL/EGL.h>
#include <EGL/EGLext.h>

#include <GLES3/gl32.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3ext.h>

#include "android_log.h"

// Write C++ code here.
//
// Do not forget to dynamically load the C++ library into your application.
//
// For instance,
//
// In MainActivity.java:
//    static {
//       System.loadLibrary("vulkanglescomparison");
//    }
//
// Or, in MainActivity.kt:
//    companion object {
//      init {
//         System.loadLibrary("vulkanglescomparison")
//      }
//    }

//------------------ main Loop -------------------
const EGLint attribs[] = {
        EGL_SURFACE_TYPE   , EGL_WINDOW_BIT, //Window Type : EGL_WINDOW_BIT : On Screen Rendering. EGL_PIXMAP_BIT : Offscreen Rendering.
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT, //Context Type : EGL_OPENGL_BIT : OpenGL Context. EGL_OPENGL_ES_BIT : 1.0 or 1.1 EGL_OPENGL_ES2_BIT 2.0 EGL_OPENVG_BIT : VG
        EGL_BLUE_SIZE      , 8,
        EGL_GREEN_SIZE     , 8,
        EGL_RED_SIZE       , 8,
        EGL_ALPHA_SIZE     , 8,
        EGL_DEPTH_SIZE     , 24,
        EGL_STENCIL_SIZE   , 8,
        EGL_NONE
};

EGLint context_attribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3,
                             EGL_NONE };

AAssetManager *g_asset_mgr = nullptr;
ANativeWindow *g_window = nullptr;

std::thread g_render_thread;
EGLDisplay g_display; //front buffer
EGLSurface g_surface; //back buffer
EGLContext g_context; //gl state machine
EGLConfig g_config;
EGLint g_num_configs;
EGLint g_format;
EGLint g_sw;
EGLint g_sh;
ANativeWindow *g_bind_window = nullptr;
bool is_paused = false;

bool InitializeEGL();
void RefreshEGLSurface();
void DestroyEGL();

bool InitializeEGL()
{
    LOGI("Initializing context");

    if ((g_display = eglGetDisplay(EGL_DEFAULT_DISPLAY)) == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay() returned error %d", eglGetError());
        return false;
    }
    if (!eglInitialize(g_display, 0, 0)) {
        LOGE("eglInitialize() returned error %d", eglGetError());
        return false;
    }

    if (!eglChooseConfig(g_display, attribs, &g_config, 1, &g_num_configs)) {
        LOGE("eglChooseConfig() returned error %d", eglGetError());
        DestroyEGL();
        return false;
    }

    if (!eglGetConfigAttrib(g_display, g_config, EGL_NATIVE_VISUAL_ID, &g_format)) {
        LOGE("eglGetConfigAttrib() returned error %d", eglGetError());
        DestroyEGL();
        return false;
    }

    if (!(g_context = eglCreateContext(g_display, g_config, EGL_NO_CONTEXT, context_attribs))) {
        LOGE("eglCreateContext() returned error %d", eglGetError());
        DestroyEGL();
        return false;
    }

    return true;
}

void RefreshEGLSurface()
{
    if (g_bind_window != g_window) {
        LOGI("Refresh Surface");
        if (g_surface != EGL_NO_SURFACE) {
            eglDestroySurface(g_display, g_surface);
        }
        g_bind_window = g_window;
        ANativeWindow_setBuffersGeometry(g_bind_window, 0, 0, g_format);

        if (!(g_surface = eglCreateWindowSurface(g_display, g_config, g_bind_window, 0))) {
            LOGE("eglCreateWindowSurface() returned error %d", eglGetError());
        }

        if (!eglQuerySurface(g_display, g_surface, EGL_WIDTH, &g_sw) ||
            !eglQuerySurface(g_display, g_surface, EGL_HEIGHT, &g_sh)) {
            LOGE("eglQuerySurface() returned error %d", eglGetError());
        }
    }
}

void DestroyEGL()
{
    LOGI("Destroying context");

    eglMakeCurrent(g_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(g_display, g_context);
    eglDestroySurface(g_display, g_surface);
    eglTerminate(g_display);

    g_display = EGL_NO_DISPLAY;
    g_surface = EGL_NO_SURFACE;
    g_context = EGL_NO_CONTEXT;
}

void RunMainLoop(ANativeWindow *i_window, int i_w, int i_h)
{
    g_window = i_window;
    g_sw = i_w;
    g_sh = i_h;
    g_render_thread = std::thread(
        [](void) {
            InitializeEGL();
            RefreshEGLSurface();

            LOGI("Initialize Renderer : %d, %d, %d, %d", g_sw, g_sh, g_sw / 2, g_sh);
            if (!eglMakeCurrent(g_display, g_surface, g_surface, g_context)) {
                LOGE("eglMakeCurrent() returned error %d", eglGetError());
                return;
            }

            while(true) {
                if (is_paused == false) {
                    int error = glGetError();
                    glClearColor(0.31f, 0.31f, 0.31f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                    error = glGetError();
                    if (error != GL_NO_ERROR) {
                        LOGI("e : %d", error);
                    }

                    if (!eglSwapBuffers(g_display, g_surface)) {
                        LOGE("eglSwapBuffers() return error %d", eglGetError());
                    }

                }
            }
        }
    );
}


//--------------------------------------------------------------------------------------------
extern "C"
JNIEXPORT void JNICALL
Java_com_sdengine_vulkanglescomparison_MainActivity_InitializeApplication(JNIEnv *env, jobject thiz,
                                                                          jobject asset_mgr) {
    // TODO: implement InitializeApplication()
    g_asset_mgr = AAssetManager_fromJava(env, asset_mgr);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_sdengine_vulkanglescomparison_MainActivity_ChangeSurface(JNIEnv *env, jobject thiz,
                                                                  jobject surface, jint format,
                                                                  jint width, jint height) {
    // TODO: implement ChangeSurface()
    ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
    if (window == nullptr) {
        LOGE("We can't find the window from surface(%p).", surface);
        return;
    }
    LOGI("NativeWindow(%p) from surface(%p)with(%d,%d,(%d))", window, surface, width, height, format);
    RunMainLoop(window, width, height);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_sdengine_vulkanglescomparison_MainActivity_Pause(JNIEnv *env, jobject thiz) {
    // TODO: implement Pause()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_sdengine_vulkanglescomparison_MainActivity_TerminateApplication(JNIEnv *env,
                                                                         jobject thiz) {
    // TODO: implement TerminateApplication()
}