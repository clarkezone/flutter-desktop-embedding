#include "pch.h"

using namespace winrt;

using namespace Windows;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::UI::Composition;

struct App : implements<App, IFrameworkViewSource, IFrameworkView> {
  CompositionTarget m_target{nullptr};
  VisualCollection m_visuals{nullptr};
  Visual m_selected{nullptr};
  float2 m_offset{};

  IFrameworkView CreateView() { return *this; }

  void Initialize(CoreApplicationView const &) {}

  void Load(hstring const &) {}

  void Uninitialize() {}

  void Run() {
    CoreWindow window = CoreWindow::GetForCurrentThread();
    window.Activate();

    CoreDispatcher dispatcher = window.Dispatcher();
    dispatcher.ProcessEvents(CoreProcessEventsOption::ProcessUntilQuit);
  }

  Windows::Foundation::IAsyncAction SetWindow(CoreWindow const &window) {
    Compositor compositor;
    ContainerVisual root = compositor.CreateContainerVisual();
    m_target = compositor.CreateTargetForCurrentView();
    m_target.Root(root);
    m_visuals = root.Children();

    window.PointerPressed({this, &App::OnPointerPressed});
    window.PointerMoved({this, &App::OnPointerMoved});

    window.PointerReleased([&](auto &&...) { m_selected = nullptr; });

    Windows::Storage::StorageFolder folder =
        Windows::ApplicationModel::Package::Current().InstalledLocation();

    Windows::Storage::StorageFolder assets = co_await folder.GetFolderAsync(L"Assets"); //TODO: handle error
    Windows::Storage::StorageFolder data =
        co_await assets.GetFolderAsync(L"data");  // TODO: handle error
    Windows::Storage::StorageFolder flutter_assets =
        co_await data.GetFolderAsync(L"flutter_assets");  // TODO: handle error
    Windows::Storage::StorageFile icu_data =
        co_await data.GetFileAsync(L"icudtl.dat");  // TODO: handle error


    std::string flutter_assets_path(to_string(flutter_assets.Path()));
    std::string icu_data_path(to_string(icu_data.Path()));

    std::vector<std::string> arguments;

    flutter::FlutterViewController flutter_controller(icu_data_path, 320, 240, flutter_assets_path, arguments);
    void* pointer = flutter_controller.view()->GetVisual();
    //auto abi_visual_ = flutter_controller.GetNativeWindow();
    //TODO: update GetNativeWindow wrapper in engine to return abiptr to spritevisual
    //TODO: cppwinrt wrapper from abiptr
    //TODO: parent visual into tree
  }

  void OnPointerPressed(IInspectable const &, PointerEventArgs const &args) {
    float2 const point = args.CurrentPoint().Position();

    for (Visual visual : m_visuals) {
      float3 const offset = visual.Offset();
      float2 const size = visual.Size();

      if (point.x >= offset.x && point.x < offset.x + size.x &&
          point.y >= offset.y && point.y < offset.y + size.y) {
        m_selected = visual;
        m_offset.x = offset.x - point.x;
        m_offset.y = offset.y - point.y;
      }
    }

    if (m_selected) {
      m_visuals.Remove(m_selected);
      m_visuals.InsertAtTop(m_selected);
    } else {
      AddVisual(point);
    }
  }

  void OnPointerMoved(IInspectable const &, PointerEventArgs const &args) {
    if (m_selected) {
      float2 const point = args.CurrentPoint().Position();

      m_selected.Offset({point.x + m_offset.x, point.y + m_offset.y, 0.0f});
    }
  }

  void AddVisual(float2 const point) {
    Compositor compositor = m_visuals.Compositor();
    SpriteVisual visual = compositor.CreateSpriteVisual();

    static Color colors[] = {{0xDC, 0x5B, 0x9B, 0xD5},
                             {0xDC, 0xED, 0x7D, 0x31},
                             {0xDC, 0x70, 0xAD, 0x47},
                             {0xDC, 0xFF, 0xC0, 0x00}};

    static unsigned last = 0;
    unsigned const next = ++last % _countof(colors);
    visual.Brush(compositor.CreateColorBrush(colors[next]));

    float const BlockSize = 100.0f;

    visual.Size({BlockSize, BlockSize});

    visual.Offset({
        point.x - BlockSize / 2.0f,
        point.y - BlockSize / 2.0f,
        0.0f,
    });

    m_visuals.InsertAtTop(visual);

    m_selected = visual;
    m_offset.x = -BlockSize / 2.0f;
    m_offset.y = -BlockSize / 2.0f;
  }
};

int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) {
  CoreApplication::Run(make<App>());
}
