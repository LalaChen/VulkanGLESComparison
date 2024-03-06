//
// Created by LaChen on 2024/3/7.
//

#pragma once

#include <EGL/EGL.h>
#include <EGL/EGLext.h>

class EGLContextHolder
{
public:
    EGLContextHolder();
    ~EGLContextHolder();
public:
    EGLContext mContext;
    EGLDisplay mDisplay;
};
