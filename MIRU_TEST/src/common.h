#pragma once

#if defined(_WIN64)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Windows.h"
#endif

#if defined(__ANDROID__)
#include "android_native_app_glue.h"
#include "android/log.h"
#endif
