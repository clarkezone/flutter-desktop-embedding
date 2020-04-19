#include <concrt.h>
#include <ppltasks.h>
#include <windows.ui.composition.h>



#include "pch.h"
#include "winrt/Windows.ApplicationModel.Core.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.System.Threading.h"
#include "winrt/Windows.UI.Core.h"
#include <chrono>
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
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
          }
        });

    mRenderLoopWorker = Windows::System::Threading::ThreadPool::RunAsync(
        workItemHandler, Windows::System::Threading::WorkItemPriority::Low,
        Windows::System::Threading::WorkItemOptions::TimeSliced);
  }

  Windows::Foundation::IAsyncAction SetWindow(CoreWindow const &window) {
    Compositor compositor;
    m_compositor = compositor;
    ContainerVisual root = m_compositor.CreateContainerVisual();
    m_target = m_compositor.CreateTargetForCurrentView();
    m_target.Root(root);
    m_visuals = root.Children();

    window.PointerPressed({this, &App::OnPointerPressed});
    window.PointerMoved({this, &App::OnPointerMoved});
    window.SizeChanged({this, &App::OnSizeChanged});

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

    //GetWindowBoundsPhysical(window);

    m_flutterController = std::make_unique<flutter::FlutterViewController>(
        icu_data_path, 1820, 1050, flutter_assets_path, arguments,
        winrt::get_abi(m_compositor));
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

    auto appView =
        Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();

     appView.SetDesiredBoundsMode(
        Windows::UI::ViewManagement::ApplicationViewBoundsMode::UseCoreWindow);

  }

  void OnPointerPressed(IInspectable const &, PointerEventArgs const &args) {
    // m_flutterController->view()->
  }

  void OnPointerMoved(IInspectable const &, PointerEventArgs const &args) {}

  void OnSizeChanged(IInspectable const&, WindowSizeChangedEventArgs const& args) {
    auto size = args.Size();
    m_flutterController->view()->SendWindowMetrics(
        static_cast<size_t>(size.Width), static_cast<size_t>(size.Height));
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
