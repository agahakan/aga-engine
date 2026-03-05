#pragma once

#include <aga/core/module.hpp>

#include <flecs.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace aga::core {
struct Time {
  double delta_seconds = 0.0;
  double elapsed_seconds = 0.0;
  std::uint64_t frame = 0;
};

struct AppExit {
  bool requested = false;
};
} // namespace aga::core

namespace aga {
class App;

class Plugin {
public:
  virtual ~Plugin() = default;
  virtual std::string_view name() const = 0;
  virtual void build(App& app) = 0;
};

struct AppConfig {
  std::string name = "aga app";
};

class App {
public:
  explicit App(AppConfig config = {});
  ~App();

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  flecs::world& world();
  const flecs::world& world() const;

  template <typename TPlugin, typename... Args> App& add_plugin(Args&&... args) {
    auto plugin = std::make_unique<TPlugin>(std::forward<Args>(args)...);
    plugin->build(*this);
    plugins_.push_back(std::move(plugin));
    return *this;
  }

  template <typename TModule> App& import_module() {
    world_.import <TModule>();
    return *this;
  }

  void run();
  void tick();
  void quit();
  bool running() const;

private:
  using Clock = std::chrono::steady_clock;

  AppConfig config_;
  flecs::world world_;
  std::vector<std::unique_ptr<Plugin>> plugins_;
  Clock::time_point last_tick_;
  bool running_ = false;
};
} // namespace aga
