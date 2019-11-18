// Copyright 2018 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <flutter/flutter_view_controller.h>
#include <windows.h>

#include <chrono>
#include <codecvt>
#include <iostream>
#include <string>
#include <vector>

#include "flutter/generated_plugin_registrant.h"
#include "win32_window.h"
#include "window_configuration.h"

namespace {

// Returns the path of the directory containing this executable, or an empty
// string if the directory cannot be found.
std::string GetExecutableDirectory() {
  wchar_t buffer[MAX_PATH];
  if (GetModuleFileName(nullptr, buffer, MAX_PATH) == 0) {
    std::cerr << "Couldn't locate executable" << std::endl;
    return "";
  }
  std::wstring_convert<std::codecvt_utf8<wchar_t>> wide_to_utf8;
  std::string executable_path = wide_to_utf8.to_bytes(buffer);
  size_t last_separator_position = executable_path.find_last_of('\\');
  if (last_separator_position == std::string::npos) {
    std::cerr << "Unabled to find parent directory of " << executable_path
              << std::endl;
    return "";
  }
  return executable_path.substr(0, last_separator_position);
}

}  // namespace

ULONG_PTR GetParentProcessId(HANDLE h)  // By Napalm @ NetCore2K
{
  if (h == 0) {
    h = GetCurrentProcess();
  }

  ULONG_PTR pbi[6];
  ULONG ulSize = 0;
  LONG(WINAPI * NtQueryInformationProcess)
  (HANDLE ProcessHandle, ULONG ProcessInformationClass,
   PVOID ProcessInformation, ULONG ProcessInformationLength,
   PULONG ReturnLength);
  *(FARPROC *)&NtQueryInformationProcess =
      GetProcAddress(LoadLibraryA("NTDLL.DLL"), "NtQueryInformationProcess");
  if (NtQueryInformationProcess) {
    if (NtQueryInformationProcess(h, 0, &pbi, sizeof(pbi), &ulSize) >= 0 &&
        ulSize == sizeof(pbi))
      return pbi[5];
  }
  return (ULONG_PTR)-1;
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE prev, wchar_t *command_line,
                      int show_command) {
  // int nArgs;
  // auto szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
  // LocalFree(szArglist);

  auto args = std::string(GetCommandLineA());
  auto rest = args.find("Flutter");

  int receivedHwnd = -1;

  // Top-level window frame.
  Win32Window::Point origin(kFlutterWindowOriginX, kFlutterWindowOriginY);
  Win32Window::Size size(kFlutterWindowWidth, kFlutterWindowHeight);

  STARTUPINFO si{};
  PROCESS_INFORMATION pi{};
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  ZeroMemory(&pi, sizeof(pi));

  Win32Window window;
  std::unique_ptr<flutter::FlutterViewController> flutter_controller = {};
  if (rest == std::string::npos && !args.empty()) {
    //__debugbreak();
    receivedHwnd = atoi(args.c_str());

    auto result = ::AllocConsole();

    // Resources are located relative to the executable.
    std::string base_directory = GetExecutableDirectory();
    if (base_directory.empty()) {
      base_directory = ".";
    }
    std::string data_directory = base_directory + "\\data";
    std::string assets_path = data_directory + "\\flutter_assets";
    std::string icu_data_path = data_directory + "\\icudtl.dat";

    // Arguments for the Flutter Engine.
    std::vector<std::string> arguments;

    flutter_controller = std::make_unique<flutter::FlutterViewController>(
        icu_data_path, size.width, size.height, assets_path, arguments);
    RegisterPlugins(flutter_controller.get());

    // Set parent
    auto res =
        SetParent(flutter_controller->GetNativeWindow(), (HWND)receivedHwnd);
    if (res == nullptr) {
      auto error = GetLastError();
      OutputDebugString(L"Failed to set parent");
    }

    RECT frame;
    GetClientRect((HWND)receivedHwnd, &frame);

    MoveWindow(flutter_controller->GetNativeWindow(), frame.left, frame.top,
               frame.right - frame.left, frame.bottom - frame.top, true);

    SetFocus(flutter_controller->GetNativeWindow());
    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }

  } else {
    if (prev != nullptr) {
      return EXIT_FAILURE;
    }
    // Create a top-level win32 window to host the Flutter view.

    if (!window.CreateAndShow(kFlutterWindowTitle, origin, size)) {
      return EXIT_FAILURE;
    }

    auto wnd = window.GetHandle();
    wchar_t buffer[_MAX_U64TOSTR_BASE2_COUNT];
    auto handleAsString = _itow_s((long)wnd, buffer, _countof(buffer), 10);

    std::string base_directory = GetExecutableDirectory();
    std::wstring foo(base_directory.begin(), base_directory.end());
    auto result = CreateProcessW(
        foo.append(L"\\Flutter Desktop Example.exe").c_str(), buffer, nullptr,
        nullptr, false, 0, nullptr, nullptr, &si, &pi);

    if (result == 0) {
      auto error = GetLastError();
      OutputDebugString(L"s");
    }

    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
  }

  return EXIT_SUCCESS;
}
