//
// pch.h
// Precompiled header for commonly included header files
//

#pragma once

#define NOMINMAX

#include <DispatcherQueue.h>
#include <ShellScalingAPI.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.System.Threading.h"
#include "winrt/Windows.UI.Composition.h"
#include "winrt/Windows.UI.Core.h"
#include "winrt/Windows.UI.Input.h"

#include <algorithm>
#include <iostream>
#include "flutter_embedder.h"
#include "winrt/Windows.Foundation.h"

// Enable function definitions in the GL headers below
#define GL_GLEXT_PROTOTYPES

// OpenGL ES includes
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

// EGL includes
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>

#define EXIT_OK 0
#define EXIT_ERR -1