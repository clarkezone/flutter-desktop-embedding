#pragma once
#include "pch.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Composition::Desktop;
using namespace Windows::Foundation::Numerics;

template <typename T>
struct DesktopWindow {
  static T *GetThisFromHandle(HWND const window) noexcept {
    return reinterpret_cast<T *>(GetWindowLongPtr(window, GWLP_USERDATA));
  }

  static LRESULT __stdcall WndProc(HWND const window, UINT const message,
                                   WPARAM const wparam,
                                   LPARAM const lparam) noexcept {
    WINRT_ASSERT(window);

    if (WM_NCCREATE == message) {
      auto cs = reinterpret_cast<CREATESTRUCT *>(lparam);
      T *that = static_cast<T *>(cs->lpCreateParams);
      WINRT_ASSERT(that);
      WINRT_ASSERT(!that->mWindow);
      that->mWindow = window;
      SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(that));

	  EnableNonClientDpiScaling(window);
      mCurrentDpi = GetDpiForWindow(window);
    } else if (T *that = GetThisFromHandle(window)) {
      return that->MessageHandler(message, wparam, lparam);
    }

    return DefWindowProc(window, message, wparam, lparam);
  }

  LRESULT MessageHandler(UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept {
    switch (message) {
      case WM_DPICHANGED: {
        return HandleDpiChange(mWindow, wparam, lparam);
      }

      case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
      }

      case WM_SIZE: {
        UINT width = LOWORD(lparam);
        UINT height = HIWORD(lparam);

        mCurrentWidth = width;
        mCurrentHeight = height;
        if (T *that = GetThisFromHandle(mWindow)) {
          that->DoResize(width, height);
        }
      }
    }

    return DefWindowProc(mWindow, message, wparam, lparam);
  }

  // DPI Change handler. on WM_DPICHANGE resize the window
  LRESULT HandleDpiChange(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    //HWND hWndStatic = GetWindow(hWnd, GW_CHILD);
    if (hWnd != nullptr) {
      UINT uDpi = HIWORD(wParam);

      // Resize the window
      auto lprcNewScale = reinterpret_cast<RECT *>(lParam);

      SetWindowPos(hWnd, nullptr, lprcNewScale->left, lprcNewScale->top,
                   lprcNewScale->right - lprcNewScale->left,
                   lprcNewScale->bottom - lprcNewScale->top,
                   SWP_NOZORDER | SWP_NOACTIVATE);

      if (T *that = GetThisFromHandle(hWnd)) {
        that->NewScale(uDpi);
      }
    }
    return 0;
  }

  void NewScale(UINT dpi) {}

  void DoResize(UINT width, UINT height) {}

  auto CreateDispatcherQueueController() {
    namespace abi = ABI::Windows::System;

    DispatcherQueueOptions options{sizeof(DispatcherQueueOptions),
                                   DQTYPE_THREAD_CURRENT, DQTAT_COM_STA};

    Windows::System::DispatcherQueueController controller{nullptr};
    check_hresult(::CreateDispatcherQueueController(
        options, reinterpret_cast<abi::IDispatcherQueueController **>(
                     put_abi(controller))));
    return controller;
  }

  DesktopWindowTarget CreateDesktopWindowTarget(Compositor const &compositor,
                                                HWND window) {
    namespace abi = ABI::Windows::UI::Composition::Desktop;

    auto interop = compositor.as<abi::ICompositorDesktopInterop>();
    DesktopWindowTarget target{nullptr};
    check_hresult(interop->CreateDesktopWindowTarget(
        window, true,
        reinterpret_cast<abi::IDesktopWindowTarget **>(put_abi(target))));
    return target;
  }

 protected:
  inline static UINT mCurrentDpi = 0;
  int mCurrentWidth = 0;
  int mCurrentHeight = 0;
  using base_type = DesktopWindow<T>;
  HWND mWindow = nullptr;
};
#pragma once
