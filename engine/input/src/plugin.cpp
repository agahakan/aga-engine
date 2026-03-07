#include <aga/input/module.hpp>

#include <aga/core/app.hpp>
#include <aga/window/module.hpp>
#include <aga/window/native_window.hpp>

#include <GLFW/glfw3.h>

namespace aga::input {
namespace {
struct KeyMap {
  Key key;
  int glfw_key;
};

constexpr KeyMap key_map[] = {
    {Key::w, GLFW_KEY_W},
    {Key::a, GLFW_KEY_A},
    {Key::s, GLFW_KEY_S},
    {Key::d, GLFW_KEY_D},
    {Key::q, GLFW_KEY_Q},
    {Key::e, GLFW_KEY_E},
    {Key::up, GLFW_KEY_UP},
    {Key::down, GLFW_KEY_DOWN},
    {Key::left, GLFW_KEY_LEFT},
    {Key::right, GLFW_KEY_RIGHT},
    {Key::space, GLFW_KEY_SPACE},
    {Key::escape, GLFW_KEY_ESCAPE},
    {Key::left_shift, GLFW_KEY_LEFT_SHIFT},
};
} // namespace

void Keyboard::begin_frame() {
  pressed_.fill(false);
  released_.fill(false);
}

void Keyboard::set(Key key, bool is_down) {
  const auto index = static_cast<std::size_t>(key);
  pressed_[index] = is_down && !down_[index];
  released_[index] = !is_down && down_[index];
  down_[index] = is_down;
}

bool Keyboard::down(Key key) const { return down_[static_cast<std::size_t>(key)]; }

bool Keyboard::pressed(Key key) const { return pressed_[static_cast<std::size_t>(key)]; }

bool Keyboard::released(Key key) const { return released_[static_cast<std::size_t>(key)]; }

InputModule::InputModule(flecs::world& ecs) {
  ecs.module<InputModule>("aga.input");
  ecs.component<InputState>("input_state");
}

std::string_view InputPlugin::name() const { return "input"; }

void InputPlugin::build(aga::App& app) {
  app.import_module<InputModule>();
  app.world().set<InputState>({});

  app.world().system("aga.input.poll").kind(flecs::PreUpdate).run([](flecs::iter& it) {
    auto ecs = it.world();
    auto* input = ecs.get_mut<InputState>();
    auto* window_resource = ecs.get_mut<window::WindowResource>();
    if (input == nullptr || window_resource == nullptr || !window_resource->main) {
      return;
    }

    input->keyboard.begin_frame();
    GLFWwindow* handle = window::detail::glfw_handle(*window_resource->main);
    for (const auto& binding : key_map) {
      const int state = glfwGetKey(handle, binding.glfw_key);
      input->keyboard.set(binding.key, state == GLFW_PRESS || state == GLFW_REPEAT);
    }

    if (input->keyboard.pressed(Key::escape)) {
      window_resource->main->request_close();
    }

    ecs.modified<InputState>();
  });
}
} // namespace aga::input
