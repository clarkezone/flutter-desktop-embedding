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

#include "embedder.h"

#include <assert.h>
#include <algorithm>
#include <chrono>
#include <iostream>

#include <flutter_embedder.h>

#include "../linux/src/internal/keyboard_hook_handler.h"
#include "../linux/src/internal/plugin_handler.h"
#include "text_input_plugin_win.h"

static_assert(FLUTTER_ENGINE_VERSION == 1, "");

// Struct for storing state within an instance of the GLFW Window.
struct FlutterEmbedderState {
  FlutterEngine engine;
  std::unique_ptr<flutter_desktop_embedding::PluginHandler> plugin_handler;

  // plugin_handler owns these pointers. Destruction happens when this struct is
  // deleted from the heap.
  std::vector<flutter_desktop_embedding::KeyboardHookHandler *>
      keyboard_hook_handlers;
};

static constexpr char kDefaultWindowTitle[] = "Flutter";

bool FlutterInit() { return glfwInit(); }

void FlutterTerminate() { glfwTerminate(); }

static FlutterEmbedderState *GetSavedEmbedderState(GLFWwindow *window) {
  return reinterpret_cast<FlutterEmbedderState *>(
      glfwGetWindowUserPointer(window));
}

static void GLFWcursorPositionCallbackAtPhase(GLFWwindow *window,
                                              FlutterPointerPhase phase,
                                              double x, double y) {
  FlutterPointerEvent event = {};
  event.struct_size = sizeof(event);
  event.phase = phase;
  event.x = x;
  event.y = y;
  event.timestamp =
      std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::high_resolution_clock::now().time_since_epoch())
          .count();
  FlutterEngineSendPointerEvent(
      reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)), &event,
      1);
}

static void GLFWcursorPositionCallback(GLFWwindow *window, double x, double y) {
  GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kMove, x, y);
}

static void GLFWmouseButtonCallback(GLFWwindow *window, int key, int action,
                                    int mods) {
  double x, y;
  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {
    glfwGetCursorPos(window, &x, &y);
    GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kDown, x, y);
    glfwSetCursorPosCallback(window, GLFWcursorPositionCallback);
  }
  if (key == GLFW_MOUSE_BUTTON_1 && action == GLFW_RELEASE) {
    glfwGetCursorPos(window, &x, &y);
    GLFWcursorPositionCallbackAtPhase(window, FlutterPointerPhase::kUp, x, y);
    glfwSetCursorPosCallback(window, nullptr);
  }
}

static void GLFWKeyCallback(GLFWwindow *window, int key, int scancode,
                            int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

static void GLFWwindowSizeCallback(GLFWwindow *window, int width, int height) {
  FlutterWindowMetricsEvent event = {};
  event.struct_size = sizeof(event);
  event.width = width;
  event.height = height;
  // TODO: Handle pixel ratio for different DPI monitors.
  event.pixel_ratio = 1.0;
  FlutterEngineSendWindowMetricsEvent(
      reinterpret_cast<FlutterEngine>(glfwGetWindowUserPointer(window)),
      &event);
}

static bool GLFWMakeContextCurrent(void *user_data) {
  GLFWwindow *window = reinterpret_cast<GLFWwindow *>(user_data);
  glfwMakeContextCurrent(window);
  return true;
}

static bool GLFWClearContext(void *user_data) {
  glfwMakeContextCurrent(nullptr);
  return true;
}

static bool GLFWPresent(void *user_data) {
  GLFWwindow *window = reinterpret_cast<GLFWwindow *>(user_data);
  glfwSwapBuffers(window);
  return true;
}

static uint32_t GLFWGetActiveFbo(void *user_data) { return 0; }

// Clears the GLFW window to Material Blue-Grey.
//
// This function is primarily to fix an issue when the Flutter Engine is
// spinning up, wherein artifacts of existing windows are rendered onto the
// canvas for a few moments.
//
// This function isn't necessary, but makes starting the window much easier on
// the eyes.
static void GLFWClearCanvas(GLFWwindow *window) {
  glfwMakeContextCurrent(window);
  // This color is Material Blue Grey.
  glClearColor(236, 239, 241, 0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glFlush();
  glfwSwapBuffers(window);
  glfwMakeContextCurrent(nullptr);
}

// Spins up an instance of the Flutter Engine.
//
// This function launches the Flutter Engine in a background thread, supplying
// the necessary callbacks for rendering within a GLFWwindow.
//
// Returns a caller-owned pointer to the engine.
static FlutterEngine RunFlutterEngine(
    GLFWwindow *window, const std::string &main_path,
    const std::string &assets_path, const std::string &packages_path,
    const std::string &icu_data_path,
    const std::vector<std::string> &arguments) {
  std::vector<const char *> argv;
  std::transform(
      arguments.begin(), arguments.end(), std::back_inserter(argv),
      [](const std::string &arg) -> const char * { return arg.c_str(); });

  FlutterRendererConfig config = {};
  config.type = kOpenGL;
  config.open_gl.struct_size = sizeof(config.open_gl);
  config.open_gl.make_current = GLFWMakeContextCurrent;
  config.open_gl.clear_current = GLFWClearContext;
  config.open_gl.present = GLFWPresent;
  config.open_gl.fbo_callback = GLFWGetActiveFbo;
  FlutterProjectArgs args = {};
  args.struct_size = sizeof(FlutterProjectArgs);
  args.assets_path = assets_path.c_str();
  args.main_path = main_path.c_str();
  args.packages_path = packages_path.c_str();
  args.icu_data_path = icu_data_path.c_str();
  args.command_line_argc = argv.size();
  args.command_line_argv = &argv[0];
  FlutterEngine engine = nullptr;
  auto result =
      FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, window, &engine);
  if (result != kSuccess || engine == nullptr) {
    return nullptr;
  }
  return engine;
}

GLFWwindow *CreateFlutterWindow(size_t initial_width, size_t initial_height,
                                const std::string &main_path,
                                const std::string &assets_path,
                                const std::string &packages_path,
                                const std::string &icu_data_path,
                                const std::vector<std::string> &arguments) {
  auto window = glfwCreateWindow(initial_width, initial_height,
                                 kDefaultWindowTitle, NULL, NULL);
  if (window == nullptr) {
    return nullptr;
  }
  GLFWClearCanvas(window);
  auto engine = RunFlutterEngine(
      window, main_path, assets_path, packages_path, icu_data_path, arguments);
  if (engine == nullptr) {
    glfwDestroyWindow(window);
    return nullptr;
  }

  FlutterEmbedderState *state = new FlutterEmbedderState();
  //state->plugin_handler = std::make_unique<flutter_desktop_embedding::PluginHandler>(engine);
  state->engine = engine;
  /*auto input_plugin =
      std::make_unique<flutter_desktop_embedding::TextInputPluginWin>();
  state->keyboard_hook_handlers.push_back(input_plugin.get());*/

  glfwSetWindowUserPointer(window, state);
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  GLFWwindowSizeCallback(window, width, height);
  glfwSetKeyCallback(window, GLFWKeyCallback);
  glfwSetWindowSizeCallback(window, GLFWwindowSizeCallback);
  glfwSetMouseButtonCallback(window, GLFWmouseButtonCallback);
  return window;
}

bool AddPlugin(GLFWwindow *flutter_window, std::unique_ptr<flutter_desktop_embedding::Plugin> plugin) {
  auto state = GetSavedEmbedderState(flutter_window);
  plugin->set_flutter_engine(state->engine);
  return state->plugin_handler->AddPlugin(std::move(plugin));
}

GLFWwindow *CreateFlutterWindowInSnapshotMode(
    size_t initial_width, size_t initial_height, const std::string &assets_path,
    const std::string &icu_data_path,
    const std::vector<std::string> &arguments) {
  return CreateFlutterWindow(initial_width, initial_height, "", assets_path, "",
                             icu_data_path, arguments);
}

void FlutterWindowLoop(GLFWwindow *flutter_window) {
  while (!glfwWindowShouldClose(flutter_window)) {
    glfwWaitEvents();
  }
  auto state = GetSavedEmbedderState(flutter_window);
  FlutterEngineShutdown(state->engine);
  delete state;
  glfwDestroyWindow(flutter_window);
}
