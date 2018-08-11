#include "pch.h" 
#include "FlutterWindow.h"

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;
using namespace Windows::Foundation::Numerics;

FlutterWindow::FlutterWindow(const wchar_t* title) noexcept {
  m_controller = CreateDispatcherQueueController();

  ConfigWind(title);

  PrepareVisuals();

  // auto result = EnableMouseInPointer(true);
  // TODO throw if not good
}

void FlutterWindow::ConfigWind(const LPCWSTR WindowTitle) {
  WNDCLASS wc{};
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hInstance = reinterpret_cast<HINSTANCE>(&__ImageBase);
  wc.lpszClassName = WindowTitle;
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  RegisterClass(&wc);
  WINRT_ASSERT(!mWindow);

  WINRT_VERIFY(CreateWindow(wc.lpszClassName, WindowTitle,
                            WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                            nullptr, nullptr, wc.hInstance, this));

  WINRT_ASSERT(mWindow);
}

LRESULT FlutterWindow::MessageHandler(UINT const message, WPARAM const wparam,
                                      LPARAM const lparam) noexcept {
  int xPos = 0, yPos = 0;
  switch (message) {
    case WM_MOUSEMOVE:
      xPos = GET_X_LPARAM(lparam);
      yPos = GET_Y_LPARAM(lparam);
      m_engine->OnPointerMove(static_cast<double>(xPos),
                              static_cast<double>(yPos));
      break;
    case WM_LBUTTONDOWN:
      // case WM_POINTERDOWN:
      xPos = GET_X_LPARAM(lparam);
      yPos = GET_Y_LPARAM(lparam);
      m_engine->OnPointerDown(static_cast<double>(xPos),
                              static_cast<double>(yPos));
      break;
    case WM_LBUTTONUP:
      // case WM_POINTERUP:
      xPos = GET_X_LPARAM(lparam);
      yPos = GET_Y_LPARAM(lparam);
      m_engine->OnPointerUp(static_cast<double>(xPos),
                            static_cast<double>(yPos));
      break;
  }

  return base_type::MessageHandler(message, wparam, lparam);
}

void FlutterWindow::PrepareVisuals() {
  Compositor compositor;
  m_target = CreateDesktopWindowTarget(compositor, mWindow);
  auto root = compositor.CreateSpriteVisual();
  root.RelativeSizeAdjustment({1.0f, 1.0f});
  root.Brush(compositor.CreateColorBrush({0xFF, 0xEF, 0xE4, 0xB0}));
  m_target.Root(root);
  m_visuals = root.Children();

  AddVisual(m_visuals, mCurrentWidth, mCurrentHeight);
}

void FlutterWindow::AddVisual(VisualCollection const &visuals, float width,
                              float height) {
  Compositor compositor = m_visuals.Compositor();
  SpriteVisual visual = compositor.CreateSpriteVisual();

  visual.Size({width, height});

  visual.Brush(compositor.CreateColorBrush(Windows::UI::Colors::Red()));

  m_visuals.InsertAtTop(visual);

  m_engine = std::make_unique<FlutterEngineHost>(visual, mCurrentDpi);
}

void FlutterWindow::FlutterMessageLoop() {
  MSG message;

  while (GetMessage(&message, nullptr, 0, 0)) {
    DispatchMessage(&message);
  }
}

bool FlutterWindow::BuildConfigStartEngine(const std::string &assets,
                                           const std::string &main,
                                           const std::string &packages,
                                           const std::vector<std::string> &cmdline) {
  return m_engine->BuildConfigStartEngine(assets, main, packages, cmdline);
}

bool FlutterWindow::BuildConfigStartEngineFromSnapshot(
    const std::string &assets, const std::vector<std::string> &cmdline) {
  return m_engine->BuildConfigStartEngine(assets, "", "", cmdline);
}

void FlutterWindow::DoResize(UINT width, UINT height) {
  if (nullptr != m_engine) {
    m_engine->OnSize(width, height);
  }
}

void FlutterWindow::NewScale(UINT dpi) {
  if (nullptr != m_engine) {
    m_engine->OnDpiChanged(dpi);
  }
}