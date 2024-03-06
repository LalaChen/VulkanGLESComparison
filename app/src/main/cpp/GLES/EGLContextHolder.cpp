//
// Created by LaChen on 2024/3/7.
//

#include "EGLContextHolder.h"

EGLContextHolder::EGLContextHolder()
: mContext(EGL_NO_CONTEXT)
, mDisplay(EGL_NO_DISPLAY)
{

}

EGLContextHolder::~EGLContextHolder() {

}
