// main.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "ConsoleWindow.h"
#include "FlutterWindow.h"

using namespace std;
using namespace winrt;
using namespace Windows::Foundation;

std::vector<string> ParseCmd(LPWSTR args) {
  const wstring cmdlineWide{args};

  vector<string> argList;
  string cmdline(cmdlineWide.begin(), cmdlineWide.end());

  size_t pos = 0, prevPos = 0;

  while ((pos = cmdline.find(' ', pos)) != std::string::npos) {
    argList.push_back(cmdline.substr(prevPos, pos - prevPos));
    prevPos = ++pos;
  }
  argList.push_back(cmdline.substr(prevPos, pos - prevPos));

  return argList;
}

int __stdcall wWinMain(HINSTANCE, HINSTANCE, LPWSTR args, int) {
  const string flutter_example_assets =
      "build\\flutter_assets";

  init_apartment();
  auto cmdlineargs = ParseCmd(args);
  cmdlineargs.push_back("--dart-non-checked-mode");

  if (std::find(cmdlineargs.begin(), cmdlineargs.end(), "--showconsole") !=  cmdlineargs.end()) {
    static ConsoleWindow console;  
	std::cout << "Hello from stdout" << std::endl;
    cerr << "hello from error" << std::endl;
  }

  // TODO consistent varible naming
  FlutterWindow win{L"Hello Flutter on Win32"};

  auto validConfig = win.BuildConfigStartEngineFromSnapshot(
      flutter_example_assets, cmdlineargs);

  if (!validConfig) {
    return EXIT_ERR;
  }

  win.FlutterMessageLoop();

  return EXIT_OK;
}
