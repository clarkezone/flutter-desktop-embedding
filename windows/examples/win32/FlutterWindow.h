#pragma once
#include "pch.h"
#include "DesktopWindow.h"
#include "flutterenginehost.h"
#include <windowsx.h>


using namespace winrt::Windows::System;

struct FlutterWindow : DesktopWindow<FlutterWindow> {
  FlutterWindow(const wchar_t *title) noexcept;
  LRESULT MessageHandler(UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept;
  bool BuildConfigStartEngine(const std::string &,
                              const std::string &,
                              const std::string &,
                              const std::vector<std::string> &);
  bool BuildConfigStartEngineFromSnapshot(const std::string &assets, const std::vector<std::string> &cmdline);
  void FlutterMessageLoop();

  void DoResize(UINT width, UINT height);

  void NewScale(UINT dpi);

 private:
  void ConfigWind(const LPCWSTR);
  void PrepareVisuals();
  void AddVisual(VisualCollection const &visuals, float x, float y);

  VisualCollection m_visuals{nullptr};
  DesktopWindowTarget m_target{nullptr};
  DispatcherQueueController m_controller{nullptr};

  std::unique_ptr<FlutterEngineHost>m_engine{nullptr};
};
#pragma once