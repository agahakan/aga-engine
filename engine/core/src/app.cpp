#include <aga/core/app.hpp>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace aga::core {
CoreModule::CoreModule(flecs::world& ecs) {
  ecs.module<CoreModule>("aga.core");
  ecs.component<Time>("time");
  ecs.component<AppExit>("app_exit");
}
} // namespace aga::core

namespace aga {
App::App(AppConfig config) : config_(std::move(config)), world_(), last_tick_(Clock::now()) {
  world_.import <core::CoreModule>();
  world_.set<core::Time>({});
  world_.set<core::AppExit>({});
}

App::~App() = default;

flecs::world& App::world() { return world_; }

const flecs::world& App::world() const { return world_; }

void App::run() {
  running_ = true;
  last_tick_ = Clock::now();

#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop_arg([](void* user_data) { static_cast<App*>(user_data)->tick(); }, this,
                               0, true);
#else
  while (running()) {
    tick();
  }
#endif
}

void App::tick() {
  const auto now = Clock::now();
  const std::chrono::duration<double> delta = now - last_tick_;
  last_tick_ = now;

  auto time = world_.get_mut<core::Time>();
  time->delta_seconds = delta.count();
  time->elapsed_seconds += time->delta_seconds;
  ++time->frame;
  world_.modified<core::Time>();

  if (!world_.progress(static_cast<float>(time->delta_seconds))) {
    quit();
  }

  if (const auto* exit = world_.get<core::AppExit>(); exit != nullptr && exit->requested) {
    quit();
  }
}

void App::quit() { running_ = false; }

bool App::running() const { return running_; }
} // namespace aga
