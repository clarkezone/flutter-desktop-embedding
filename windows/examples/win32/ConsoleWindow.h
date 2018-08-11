#pragma once
#include "pch.h"

struct ConsoleWindow {
  ConsoleWindow() {
    AllocConsole();

    auto result = freopen_s(&stoutfp, "CONOUT$", "w", stdout);

    result = freopen_s(&stderrfp, "CONOUT$", "w", stderr);
  }

  ~ConsoleWindow() {
    fclose(stderrfp);
    fclose(stoutfp);
    FreeConsole();
  }

 private:
  FILE *stderrfp;
  FILE *stoutfp;
};