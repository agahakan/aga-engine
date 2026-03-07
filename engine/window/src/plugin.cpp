#include <aga/window/module.hpp>

#include <aga/core/app.hpp>
#include <aga/window/window.hpp>

#include <utility>

namespace aga::window {
WindowModule::WindowModule(flecs::world& ecs) {
  ecs.module<WindowModule>("aga.window");
  ecs.component<WindowConfig>("window_config");
  ecs.component<WindowResource>("window_resource");
}

WindowPlugin::WindowPlugin(WindowConfig config) : config_(std::move(config)) {}

std::string_view WindowPlugin::name() const { return "window"; }

void WindowPlugin::build(aga::App& app) {
  app.import_module<WindowModule>();

  auto resource = WindowResource{std::make_shared<Window>(config_)};
  app.world().set<WindowConfig>(config_);
  app.world().set<WindowResource>(std::move(resource));

  app.world().system("aga.window.poll").kind(flecs::PreUpdate).run([](flecs::iter& it) {
    auto ecs = it.world();
    auto* window = ecs.get_mut<WindowResource>();
    if (window == nullptr || !window->main) {
      return;
    }

    window->main->poll_events();
    if (window->main->should_close()) {
      auto* exit = ecs.get_mut<core::AppExit>();
      if (exit != nullptr) {
        exit->requested = true;
        ecs.modified<core::AppExit>();
      }
    }
  });
}
} // namespace aga::window
