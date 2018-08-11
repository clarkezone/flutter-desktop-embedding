//
// pch.h
// Header for platform projection include files
//

#pragma once

#define NOMINMAX

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
#include "../../lib/windows-x64-embedder/flutter_embedder.h"
#include "winrt/Windows.Foundation.h"

// Enable function definitions in the GL headers below
#define GL_GLEXT_PROTOTYPES

//// OpenGL ES includes
//#include <GLES2/gl2.h>
//#include <GLES2/gl2ext.h>
//
//// EGL includes
//#include <EGL/egl.h>
//#include <EGL/eglext.h>
//#include <EGL/eglplatform.h>
