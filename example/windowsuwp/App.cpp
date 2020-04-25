#include <concrt.h>
#include <ppltasks.h>
#include <windows.ui.composition.h>

#include "pch.h"
#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.System.Threading.h"
#include "winrt/Windows.UI.Core.h"

#include <chrono>  //these should be at the bottom
#include <thread>

using namespace winrt;
using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::Foundation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView> {
  CompositionTarget m_target{nullptr};
  VisualCollection m_visuals{nullptr};

  float2 m_offset{};
  Compositor m_compositor{nullptr};
  IAsyncAction mRenderLoopWorker{nullptr};
  // Concurrency::critical_section mRenderSurfaceCriticalSection;

  std::unique_ptr<flutter::FlutterViewController> m_flutterController{nullptr};

  IFrameworkView CreateView() { return *this; }

  void Initialize(CoreApplicationView const &view) {
    auto result = Windows::UI::ViewManagement::ApplicationViewScaling::
        TrySetDisableLayoutScaling(true);
  }

  void Load(hstring const &) {}

  void Uninitialize() {}

  void Run() {
    CoreWindow window = CoreWindow::GetForCurrentThread();
    window.Activate();

    CoreDispatcher dispatcher = window.Dispatcher();
    StartMessagePump();
    dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
  }

  void StartMessagePump() {
    if (mRenderLoopWorker != nullptr &&
        mRenderLoopWorker.Status() ==
            Windows::Foundation::AsyncStatus::Started) {
      return;
    }

    CoreWindow window = CoreWindow::GetForCurrentThread();
    window.Activate();

    CoreDispatcher dispatcher = window.Dispatcher();

    auto workItemHandler = Windows::System::Threading::WorkItemHandler(
        [this, dispatcher](Windows::Foundation::IAsyncAction action) {
          // TODO do we need to lock?
          // critical_section::scoped_lock lock(mRenderSurfaceCriticalSection);

          while (action.Status() ==
                 winrt::Windows::Foundation::AsyncStatus::Started) {
            dispatcher.RunAsync(
                Windows::UI::Core::CoreDispatcherPriority::Normal,
                [this, dispatcher]() {
                  if (m_flutterController != nullptr) {
                    m_flutterController->ProcessMessages();
                  }
                });
            // TODO can we do better?
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          }
        });

    mRenderLoopWorker = Windows::System::Threading::ThreadPool::RunAsync(
        workItemHandler, Windows::System::Threading::WorkItemPriority::Low,
        Windows::System::Threading::WorkItemOptions::TimeSliced);
  }

  Windows::Foundation::IAsyncAction SetWindow(CoreWindow const &window) {
    auto appView =
        Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

    appView.SetDesiredBoundsMode(
        Windows::UI::ViewManagement::ApplicationViewBoundsMode::
            UseCoreWindow);

     appView.VisibleBoundsChanged({this, &App::OnBoundsChanged});

    Compositor compositor;
    m_compositor = compositor;
    ContainerVisual root = m_compositor.CreateContainerVisual();
    m_target = m_compositor.CreateTargetForCurrentView();
    m_target.Root(root);
    m_visuals = root.Children();

    window.PointerPressed({this, &App::OnPointerPressed});
    window.PointerReleased({this, &App::OnPointerReleased});
    window.PointerMoved({this, &App::OnPointerMoved});
    window.PointerWheelChanged({this, &App::OnPointerWheelChanged});

    // TODO mouse leave
    // TODO scroll
    // TODO fontchanged

    window.KeyUp({this, &App::OnKeyUp});
    window.KeyDown({this, &App::OnKeyDown});
    window.CharacterReceived({this, &App::OnCharacterReceived});

    window.PointerReleased([&](auto &&...) {});

    Windows::Storage::StorageFolder folder =
        Windows::ApplicationModel::Package::Current().InstalledLocation();

    Windows::Storage::StorageFolder assets =
        co_await folder.GetFolderAsync(L"Assets");  // TODO: handle error
    Windows::Storage::StorageFolder data =
        co_await assets.GetFolderAsync(L"data");  // TODO: handle error
    Windows::Storage::StorageFolder flutter_assets =
        co_await data.GetFolderAsync(L"flutter_assets");  // TODO: handle error
    Windows::Storage::StorageFile icu_data =
        co_await data.GetFileAsync(L"icudtl.dat");  // TODO: handle error

    std::string flutter_assets_path(to_string(flutter_assets.Path()));
    std::string icu_data_path(to_string(icu_data.Path()));

    std::vector<std::string> arguments;

    auto bounds = appView.VisibleBounds();

    // GetWindowBoundsPhysical(window);

    m_flutterController = std::make_unique<flutter::FlutterViewController>(
        icu_data_path, bounds.Width, bounds.Height, flutter_assets_path,
        arguments, winrt::get_abi(m_compositor));
    /*m_flutterController = std::make_unique<flutter::FlutterViewController>(
        icu_data_path, 1820, 1050, flutter_assets_path, arguments,
        winrt::get_abi(m_compositor));*/
    void *pointer = m_flutterController->view()->GetVisual();

    winrt::com_ptr<Windows::UI::Composition::ISpriteVisual> rawAbi = nullptr;

    Windows::UI::Composition::ISpriteVisual abiFlutterVisual = nullptr;

    winrt::copy_from_abi(abiFlutterVisual, pointer);
    // TODO ensure refcounting is correct, release abiPtr

    Windows::UI::Composition::Visual flutterVisual = nullptr;
    abiFlutterVisual.as(flutterVisual);
    if (flutterVisual != nullptr) {
      m_visuals.InsertAtTop(flutterVisual);
    }

    // m_flutterController->view()->SendWindowMetrics(640, 480);

    // window.Dispatcher().RunAsync(
    //    Windows::UI::Core::CoreDispatcherPriority::Normal, [this]() {
    //        //m_flutterController->ProcessMessages();
    //    }
    //);

    // m_flutterController->ProcessMessages();
  }

  //// Translates button codes from Win32 API to FlutterPointerMouseButtons.
  // static uint64_t ConvertWinButtonToFlutterButton(UINT button) {
  //  switch (button) {
  //    case WM_LBUTTONDOWN:
  //    case WM_LBUTTONUP:
  //      return kFlutterPointerButtonMousePrimary;
  //    case WM_RBUTTONDOWN:
  //    case WM_RBUTTONUP:
  //      return kFlutterPointerButtonMouseSecondary;
  //    case WM_MBUTTONDOWN:
  //    case WM_MBUTTONUP:
  //      return kFlutterPointerButtonMouseMiddle;
  //    case XBUTTON1:
  //      return kFlutterPointerButtonMouseBack;
  //    case XBUTTON2:
  //      return kFlutterPointerButtonMouseForward;
  //  }
  //  std::cerr << "Mouse button not recognized: " << button << std::endl;
  //  return 0;
  //}

  void OnPointerPressed(IInspectable const &, PointerEventArgs const &args) {
    auto x = static_cast<double>(args.CurrentPoint().Position().X);
    auto y = static_cast<double>(args.CurrentPoint().Position().Y);
    m_flutterController->view()->SendPointerDown(
        x * static_cast<double>(1.0), y * static_cast<double>(1.0),
        flutter::kFlutterPointerButtonMousePrimary);
  }

  void OnPointerReleased(IInspectable const &, PointerEventArgs const &args) {
    auto x = static_cast<double>(args.CurrentPoint().Position().X);
    auto y = static_cast<double>(args.CurrentPoint().Position().Y);
    m_flutterController->view()->SendPointerUp(
        x * static_cast<double>(1.0), y * static_cast<double>(1.0),
        flutter::kFlutterPointerButtonMousePrimary);
  }

  void OnPointerMoved(IInspectable const &, PointerEventArgs const &args) {
    auto x = static_cast<double>(args.CurrentPoint().Position().X);
    auto y = static_cast<double>(args.CurrentPoint().Position().Y);
    m_flutterController->view()->SendPointerMove(x * static_cast<double>(1.0),
                                                 y * static_cast<double>(1.0));
  }

  void OnPointerWheelChanged(IInspectable const&,
      PointerEventArgs const& args) {
    auto x = static_cast<double>(args.CurrentPoint().Position().X);
    auto y = static_cast<double>(args.CurrentPoint().Position().Y);
    //TODO this is broken
    m_flutterController->view()->SendScroll(0,-10);
  }

  void OnBoundsChanged(Windows::UI::ViewManagement::ApplicationView const &appView,
                       IInspectable const &) {
    if (m_flutterController) {
      auto bounds = appView.VisibleBounds();

       auto disp =
          Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
      float dpi = disp.LogicalDpi();

      m_flutterController->view()->SendWindowMetrics(
          static_cast<size_t>(bounds.Width),
          static_cast<size_t>(bounds.Height),
          dpi);
    }
  }

  void OnKeyUp(IInspectable const &, KeyEventArgs const &args) {
    // TODO: system key (back) and unicode handling
    auto status = args.KeyStatus();
    unsigned int scancode = status.ScanCode;
    int key = static_cast<int>(args.VirtualKey());
    char32_t chararacter = static_cast<char32_t>(key);
    int action = 0x0101;
    m_flutterController->view()->SendKey(key, scancode, action, chararacter);
  }

  void OnKeyDown(IInspectable const &, KeyEventArgs const &args) {
    // TODO: system key (back) and unicode handling
    auto status = args.KeyStatus();
    unsigned int scancode = status.ScanCode;
    int key = static_cast<int>(args.VirtualKey());
    char32_t chararacter = static_cast<char32_t>(key);
    int action = 0x0100;
    m_flutterController->view()->SendKey(key, scancode, action, chararacter);
  }

  void OnCharacterReceived(IInspectable const &,
                           CharacterReceivedEventArgs const &args) {
    auto key = args.KeyCode();
    wchar_t keycode = static_cast<wchar_t>(key);
    std::u16string text({keycode});
    m_flutterController->view()->SendText(text);
  }

  void GetWindowBoundsPhysical(CoreWindow const &window) {
    Rect bounds = window.Bounds();
    auto disp =
        Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
    float dpi = disp.LogicalDpi();
  }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  CoreApplication::Run(make<App>());
}
