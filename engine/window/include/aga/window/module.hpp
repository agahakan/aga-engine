#pragma once

#include <aga/core/app.hpp>

#include <memory>
#include <string>
#include <string_view>

namespace aga::window {
class Window;

struct WindowConfig {
  std::string title = "aga engine";
  int width = 1280;
  int height = 720;
  bool resizable = true;
};

struct WindowResource {
  std::shared_ptr<Window> main;
};

struct WindowModule {
  explicit WindowModule(flecs::world& ecs);
};

class WindowPlugin final : public aga::Plugin {
public:
  explicit WindowPlugin(WindowConfig config = {});
  std::string_view name() const override;
  void build(aga::App& app) override;

private:
  WindowConfig config_;
};
} // namespace aga::window
