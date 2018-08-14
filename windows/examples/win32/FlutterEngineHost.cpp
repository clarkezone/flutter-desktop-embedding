#include "pch.h"
#include "FlutterEngineHost.h"

FlutterEngineHost::FlutterEngineHost(const SpriteVisual &hostVisual,
                                     double hostDpi, HWND hostHwnd)
    : mEngineConfigured(false), mWindow(hostHwnd){
  mOpenGLES = std::make_unique<OpenGLES>();
  mHostVisual = hostVisual;

  Visual baseVisual{nullptr};
  hostVisual.as(baseVisual);
  OnDpiChanged(hostDpi);
  OnSize(baseVisual.Size().x, baseVisual.Size().y);
}

FlutterEngineHost::~FlutterEngineHost() {
  // TODO: de-init flutter
  DestroyRenderSurface();
}

bool FlutterEngineHost::BuildConfigStartEngine(
    const std::string &assets, const std::string &main,
    const std::string &packages, const std::vector<std::string> &cmdline) {
  CreateRenderSurface();
  mEngineConfigured = ConfigureFlutterEngine(assets, main, packages, cmdline);

  OnSize(mHostVisual.Size().x, mHostVisual.Size().y);

  return mEngineConfigured;
}

static void FlutterPlatformformMessage(const FlutterPlatformMessage *message,
                                       void *userData) {
  if (message->struct_size != sizeof(FlutterPlatformMessage)) {
    std::cerr << "Invalid message size received. Expected: "
              << sizeof(FlutterPlatformMessage) << " but received "
              << message->struct_size << std::endl;
    return;
  }

  auto host = (FlutterEngineHost *)userData;
}

bool FlutterEngineHost::ConfigureFlutterEngine(
    const std::string &assets, const std::string &main,
    const std::string &packages, const std::vector<std::string> &cmdline) {
  FlutterOpenGLRendererConfig cfg;

  cfg.fbo_callback = [](void *userData) -> uint32_t { 
	  return 0;
  };
  cfg.clear_current = [](void *userData) -> bool { return false; };
  cfg.make_current = [](void *userData) -> bool {
    auto host = (FlutterEngineHost *)userData;
    host->MakeCurrent();
    return true;
  };
  //cfg.make_resource_current = nullptr;
  cfg.make_resource_current = [](void *userData) -> bool { 
	  //return true;
    return false;
  };

  //cfg.get_entrypoint_by_name = nullptr;

  cfg.get_entrypoint_by_name =
	  [](void *userData, const char* what) -> void* {
    return eglGetProcAddress(what);
    //OutputDebugString(L"");
    //return nullptr;
  };

  cfg.present = [](void *userData) -> bool {
    auto host = (FlutterEngineHost *)userData;
    host->SwapBuffers();
    return true;
  };
  cfg.struct_size = sizeof(cfg);

  FlutterRendererConfig render{FlutterRendererType::kOpenGL};
  render.open_gl = cfg;

  // TODO: leak?
  const char **args_arr2 = new const char *[cmdline.size()];
  for (int i = 0; i < cmdline.size(); i++) {
    args_arr2[i] = cmdline.data()[i].c_str();
  }
  args_arr2[cmdline.size()] = nullptr;

  FlutterProjectArgs args{};
  args.struct_size = sizeof(FlutterProjectArgs);

  args.assets_path = assets.c_str();

  args.command_line_argc = cmdline.size();
  args.command_line_argv = args_arr2;  // const_cast<char **>(args_arr);  //
  // TODO: file bug why buildbot not including this [DEPENDENCY]
  // TODO: read this with relative path	
  args.icu_data_path = "C:\\Users\\james\\Source\\github.com\\clarkezone\\flutter-"
                       "desktop-embedding\\windows\\x64\\Debug\\icudtl.dat";
  args.main_path = main.c_str();

  args.packages_path = packages.c_str();
  args.platform_message_callback =
      FlutterPlatformformMessage;  // TODO redirect into class not static
  auto result =
      FlutterEngineRun(FLUTTER_ENGINE_VERSION, &render, &args, this, &mEngine);

  return result == FlutterResult::kSuccess;
}

void FlutterEngineHost::MakeCurrent() {
  mOpenGLES->MakeCurrent(mRenderSurface);
}

void FlutterEngineHost::SwapBuffers() {
  mOpenGLES->SwapBuffers(mRenderSurface);
}

void FlutterEngineHost::CreateRenderSurface() {
  if (mOpenGLES && mRenderSurface == EGL_NO_SURFACE) {
    //mRenderSurface = mOpenGLES->CreateSurface(mHostVisual);
    mRenderSurface = mOpenGLES->CreateSurface(mWindow);
	  
  }
}

void FlutterEngineHost::DestroyRenderSurface() {
  if (mOpenGLES) {
    mOpenGLES->DestroySurface(mRenderSurface);
  }
  mRenderSurface = EGL_NO_SURFACE;
}

void FlutterEngineHost::OnDpiChanged(int dpi) {
  mHostDpi = dpi;
  if (mEngineConfigured) {
    UpdateSize(mHostWidth, mHostHeight, mHostDpi);
  }
}

void FlutterEngineHost::OnSize(int width, int height) {
  mHostHeight = height;
  mHostWidth = width;
  mHostVisual.Size({(float)width, (float)height});
  if (mEngineConfigured) {
    UpdateSize(width, height, mHostDpi);
  }
}

void FlutterEngineHost::OnPointerMove(double x, double y) {
  if (mPointerDown) {
    OnPointer(x, y, FlutterPointerPhase::kMove);
  }
}

void FlutterEngineHost::OnPointerDown(double x, double y) {
  mPointerDown = true;
  OnPointer(x, y, FlutterPointerPhase::kDown);
}

void FlutterEngineHost::OnPointerUp(double x, double y) {
  mPointerDown = false;
  OnPointer(x, y, FlutterPointerPhase::kUp);
}

void FlutterEngineHost::OnPointer(double x, double y,
                                  FlutterPointerPhase phase) {
  FlutterPointerEvent event = {};

  event.struct_size = sizeof(event);

  event.phase = phase;

  event.x = x;

  event.y = y;

  event.timestamp =

      std::chrono::duration_cast<std::chrono::microseconds>(

          std::chrono::high_resolution_clock::now().time_since_epoch())

          .count();

  auto result = FlutterEngineSendPointerEvent(mEngine, &event, 1);
  // TODO: check result
}


    void FlutterEngineHost::UpdateSize(int width, int height, int dpi) {
  FlutterWindowMetricsEvent event = {};

  event.struct_size = sizeof(event);

  event.width = width;

  event.height = height;

  event.pixel_ratio = static_cast<double>(dpi) / 100;

  auto result = FlutterEngineSendWindowMetricsEvent(mEngine, &event);

  // TODO: check for failure here
  std::cout << "Flutter engine result:" << result << std::endl;
}