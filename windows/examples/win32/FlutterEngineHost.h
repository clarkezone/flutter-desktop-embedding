#pragma once
#include "OpenGLES.h"

using namespace winrt;
using namespace Windows::UI::Composition;

struct FlutterEngineHost {
  FlutterEngineHost(const SpriteVisual &hostVisual, double hostDpi, HWND hostHwnd);
  ~FlutterEngineHost();
  bool ConfigureFlutterEngine(const std::string &,
                              const std::string &,
                              const std::string &,
                              const std::vector<std::string> &);
  bool BuildConfigStartEngine(const std::string &,
                              const std::string &,
                              const std::string &,
                              const std::vector<std::string> &);
  void MakeCurrent();
  void SwapBuffers();
  void OnSize(int width, int height);
  void OnDpiChanged(int dpi);
  void OnPointerDown(double x, double y);
  void OnPointerUp(double x, double y);
  void OnPointerMove(double x, double y);

 private:
  void OnPointer(double x, double y, FlutterPointerPhase phase);
  void CreateRenderSurface();
  void DestroyRenderSurface();
  void UpdateSize(int width, int height, int dpi);
  
  std::unique_ptr<OpenGLES> mOpenGLES{nullptr};
  EGLSurface mRenderSurface{EGL_NO_SURFACE};
  SpriteVisual mHostVisual{nullptr};

  //TODO unique_ptr?
  FlutterEngine mEngine{nullptr};
  bool mEngineConfigured;
  int mHostWidth, mHostHeight;
  int mHostDpi;
  bool mPointerDown;
  HWND mWindow;
};